/***************************************************************************//**
 * @file
 * @brief Clock example for SLSTK3400A-EFM32HG
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_pcnt.h"
#include "em_prs.h"
#include "display.h"
#include "glib.h"
#include "bspconfig.h"

/* Defines for the LED 0 */
#define LED0_PORT    gpioPortF
#define LED0_PIN     4

/* Frequency of RTC (COMP0) pulses on PRS channel 2
 * = frequency of LCD polarity inversion. */
#define RTC_PULSE_FREQUENCY    (64)


/* The current time reference. Number of seconds since midnight
 * January 1, 1970.  */
static volatile time_t curTime = 0;
static volatile time_t curCount =0;
static volatile time_t ledTime =0;
static volatile time_t timeArray[10];

/* PCNT interrupt counter */
static volatile int pcntIrqCount = 0;

/* Flag to check when we should redraw a frame */
static volatile bool updateDisplay = true;
static volatile bool timeIsFastForwarding = false;
static volatile bool scheduleTimeRequest = false;
static volatile bool buttonPressed = false;


/* Global glib context */
GLIB_Context_t gc;



/***************************************************************************//**
 * @brief Setup GPIO interrupt for pushbuttons.
 ******************************************************************************/
static void gpioSetup(void)
{
  /* Enable GPIO clock */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure PB0 as input and enable interrupt  */
  GPIO_PinModeSet(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, gpioModeInputPull, 1);
  GPIO_IntConfig(BSP_GPIO_PB0_PORT, BSP_GPIO_PB0_PIN, false, true, true);

  /* Configure PB1 as input and enable interrupt */
  GPIO_PinModeSet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, gpioModeInputPull, 1);
  GPIO_IntConfig(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN, false, true, true);

  GPIO_PinModeSet(LED0_PORT, LED0_PIN, gpioModePushPull, 0);

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

/***************************************************************************//**
* @brief Unified GPIO Interrupt handler (pushbuttons)
*        PB0 Switches between analog and digital clock modes
*        PB1 Increments the time by one minute
*******************************************************************************/
void GPIO_Unified_IRQ(void)
{
  /* Get and clear all pending GPIO interrupts */
  uint32_t interruptMask = GPIO_IntGet();
  GPIO_IntClear(interruptMask);

  /* Act on interrupts */
  if (interruptMask & (1 << BSP_GPIO_PB0_PIN)) {
	buttonPressed = true;
    scheduleTimeRequest = true;
    updateDisplay = true;
  }

  if (interruptMask & (1 << BSP_GPIO_PB1_PIN)) {

	if(!(scheduleTimeRequest))
	  /* Increase time by 1 second. */
		curTime++;
	else
	{
		timeArray[curCount-1]++;
	}

    timeIsFastForwarding = true;
    updateDisplay = true;

  }
}

/***************************************************************************//**
 * @brief GPIO Interrupt handler for even pins
 ******************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  GPIO_Unified_IRQ();
}

/***************************************************************************//**
 * @brief GPIO Interrupt handler for odd pins
 ******************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  GPIO_Unified_IRQ();
}

/***************************************************************************//**
 * @brief   Set up PCNT to generate an interrupt every second.
 *
 ******************************************************************************/
void pcntInit(void)
{
  PCNT_Init_TypeDef pcntInit = PCNT_INIT_DEFAULT;

  /* Enable PCNT clock */
  CMU_ClockEnable(cmuClock_PCNT0, true);
  /* Set up the PCNT to count RTC_PULSE_FREQUENCY pulses -> one second */
  pcntInit.mode = pcntModeOvsSingle;
  pcntInit.top = RTC_PULSE_FREQUENCY;
  pcntInit.s1CntDir = false;
  /* The PRS channel used depends on the configuration and which pin the
     LCD inversion toggle is connected to. So use the generic define here. */
  pcntInit.s0PRS = (PCNT_PRSSel_TypeDef)LCD_AUTO_TOGGLE_PRS_CH;

  PCNT_Init(PCNT0, &pcntInit);

  /* Select PRS as the input for the PCNT */
  PCNT_PRSInputEnable(PCNT0, pcntPRSInputS0, true);

  /* Enable PCNT interrupt every second */
  NVIC_EnableIRQ(PCNT0_IRQn);
  PCNT_IntEnable(PCNT0, PCNT_IF_OF);
}

/***************************************************************************//**
 * @brief   This interrupt is triggered at every second by the PCNT
 *
 ******************************************************************************/
void PCNT0_IRQHandler(void)
{
  PCNT_IntClear(PCNT0, PCNT_IF_OF);

  pcntIrqCount++;

  /* Increase time with 1s */
  if (!(timeIsFastForwarding))
  {
    curTime++;

    /* Check if current time is been set a a scheduled time*/
    for(int i =0; i<10; i++)
    {
    	if(timeArray[i] == curTime) //turn on led
    	{
    		GPIO_PinOutSet(LED0_PORT, LED0_PIN);
    		ledTime = curTime;
    	}

    	if(curTime >= ledTime+30) //after 30sec turn led back off
    	{
    		GPIO_PinOutClear(LED0_PORT, LED0_PIN);

    	}
    }
  }

  /* Notify main loop to redraw clock on display. */
  if(!(scheduleTimeRequest))
	  updateDisplay = true;
}

/***************************************************************************//**
 * @brief  Increments the clock quickly while PB1 is pressed.
 *         A callback is used to update either the analog or the digital clock.
 *
 ******************************************************************************/
void fastForwardTime(void (*drawClock)(struct tm*, bool redraw))
{
  unsigned int i = 0;
  struct tm    *time;

  /* Wait 2 seconds before starting to adjust quickly */
  int waitForPcntIrqCount = pcntIrqCount + 2;

  while (pcntIrqCount != waitForPcntIrqCount) {
    /* Return if the button is released */
    if (GPIO_PinInGet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN) == 1) {
    	time = gmtime((time_t const *) &timeArray[curCount-1]);
    	timeIsFastForwarding = false;
    	drawClock(time, true);
    	return;
    }

    /* Keep updating the second counter while waiting */
    if (updateDisplay) {
    	time = gmtime((time_t const *) &curTime);
    	drawClock(time, true);
    }

    EMU_EnterEM2(false);
  }

  /* Keep incrementing the time while the button is held */
  while (GPIO_PinInGet(BSP_GPIO_PB1_PORT, BSP_GPIO_PB1_PIN) == 0) {
    if (i % 1000 == 0) {
    	if(!(scheduleTimeRequest))
    	{
    		/* Increase time by 1 minute (60 seconds). */
    		curTime += 60;
    		time = gmtime((time_t const *) &curTime);
    	}

    	else
    	{
    		timeArray[curCount-1]+=60;
    		time = gmtime((time_t const *) &timeArray[curCount-1]);
    	}


      drawClock(time, true);
    }
    i++;
  }
  timeIsFastForwarding = false;
}

void scheduleTime(void (*drawClock)(struct tm*, bool redraw))
{
  struct tm    *counttime;

  if(buttonPressed)
  {
	/* Show a counter from 1 to 10 on screen, add 1 when pressing right button */
  curCount += 1;
  buttonPressed = false;
  counttime = gmtime((time_t const *) &curCount);
  drawClock(counttime, true);
  }

if(curCount > 10)
  {
	  curCount = 0;
	  scheduleTimeRequest = false;
  }

}

/***************************************************************************//**
 * @brief  Updates the digital clock.
 *
 ******************************************************************************/
void digitalClockUpdate(struct tm *time, bool redraw)
{
  char clockString[16];

  if (redraw) {
    GLIB_setFont(&gc, (GLIB_Font_t *)&GLIB_FontNumber16x20);
    gc.backgroundColor = White;
    gc.foregroundColor = Black;
    GLIB_clear(&gc);
  }

  sprintf(clockString, "%02d:%02d:%02d", time->tm_hour, time->tm_min, time->tm_sec);
  GLIB_drawString(&gc, clockString, strlen(clockString), 1, 52, true);

  /* Update display */
  DMD_updateDisplay();
}


/***************************************************************************//**
 * @brief  Shows an digital clock on the display.
 *
 ******************************************************************************/
void digitalClockShow(bool redraw)
{
  /* Convert time format */
  struct tm *time = gmtime((time_t const *) &curTime);

  if (updateDisplay) {
	  if(!(scheduleTimeRequest))
		  digitalClockUpdate(time, redraw);
    updateDisplay = false;
    if (timeIsFastForwarding) {
      fastForwardTime(digitalClockUpdate);
    }
    if(scheduleTimeRequest)
    {
    	scheduleTime(digitalClockUpdate);
    }
  }
}

/***************************************************************************//**
 * @brief  Main function of clock example.
 *
 ******************************************************************************/
int main(void)
{
  EMSTATUS status;



  /* Chip errata */
  CHIP_Init();

  /* Use the 21 MHz band in order to decrease time spent awake.
     Note that 21 MHz is the highest HFRCO band on ZG. */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  CMU_HFRCOBandSet(cmuHFRCOBand_21MHz);

  /* Setup GPIO for pushbuttons. */
  gpioSetup();

  /* Initialize display module */
  status = DISPLAY_Init();
  if (DISPLAY_EMSTATUS_OK != status) {
    while (true)
      ;
  }

  /* Initialize the DMD module for the DISPLAY device driver. */
  status = DMD_init(0);
  if (DMD_OK != status) {
    while (true)
      ;
  }

  status = GLIB_contextInit(&gc);
  if (GLIB_OK != status) {
    while (true)
      ;
  }

  /* Set PCNT to generate interrupt every second */
  pcntInit();



  /* Enter infinite loop that switches between analog and digital clock
   * modes, toggled by pressing the button PB0. */
  while (true) {
	digitalClockShow(true);
    /*Sleep between each frame update */
    EMU_EnterEM2(false);
  }
}
