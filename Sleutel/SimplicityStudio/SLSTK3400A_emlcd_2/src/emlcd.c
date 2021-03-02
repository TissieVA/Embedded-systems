/***************************************************************************//**
 * @file
 * @brief Energy Mode demo for SLSTK3400A_EFM32HG
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

#include <stdio.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_pcnt.h"

#include "display.h"
#include "textdisplay.h"
#include "retargettextdisplay.h"
#include "em4config.h"

/* Frequency of RTC (COMP0) pulses on PRS channel 2. */
#define RTC_PULSE_FREQUENCY    (LS013B7DH03_POLARITY_INVERSION_FREQUENCY)
#define SLEEP_TIME             (1)

static volatile bool      displayEnabled = false; /* Status of LCD display. */
static volatile bool      enterEM4 = false;
static volatile uint32_t  seconds = 0;     /* Seconds elapsed since reset.  */

static DISPLAY_Device_t displayDevice;    /* Display device handle.         */


static void GpioSetup(void);
static void PcntInit(void);

/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* Setup GPIO for pushbuttons. */
  GpioSetup();

  /* Initialize the display module. */
  displayEnabled = true;
  DISPLAY_Init();

  /* Retrieve the properties of the display. */
  if ( DISPLAY_DeviceGet(0, &displayDevice) != DISPLAY_EMSTATUS_OK ) {
    /* Unable to get display handle. */
    while ( 1 ) ;
  }

  /* Retarget stdio to the display. */
  if ( TEXTDISPLAY_EMSTATUS_OK != RETARGET_TextDisplayInit() ) {
    /* Text display initialization failed. */
    while ( 1 ) ;
  }

  /* Set PCNT to generate an interrupt every second. */
  PcntInit();

  printf("\n TIJS");

  /* Turn on LFXO to be able to see the difference between EM2 and EM3. */
  CMU_OscillatorEnable(cmuOsc_LFXO, true, false);


}


/***************************************************************************//**
 * @brief Setup GPIO interrupt for pushbuttons.
 ******************************************************************************/
static void GpioSetup(void)
{
  /* Enable GPIO clock. */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure PC8 as input and enable interrupt. */
  GPIO_PinModeSet(EM4_NON_WU_PB_PORT, EM4_NON_WU_PB_PIN, gpioModeInputPull, 1);
  GPIO_IntConfig(EM4_NON_WU_PB_PORT, EM4_NON_WU_PB_PIN, false, true, true);

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

  /* Configure PC9 as input. */
  GPIO_PinModeSet(EM4_WU_PB_PORT, EM4_WU_PB_PIN, gpioModeInputPull, 1);
}

/***************************************************************************//**
 * @brief Unified GPIO Interrupt handler (pushbuttons)
 *        PB0 Starts selected test
 *        PB1 Cycles through the available tests
 *****************************************************************************/
void GPIO_Unified_IRQ(void)
{
  /* Get and clear all pending GPIO interrupts */
  uint32_t interruptMask = GPIO_IntGet();
  GPIO_IntClear(interruptMask);

  /* Act on interrupts */
  if (interruptMask & (1 << EM4_NON_WU_PB_PIN)) {
    /* ALT_PB */
    enterEM4 = true;
  }
}

/***************************************************************************//**
 * @brief GPIO Interrupt handler for even pins
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  GPIO_Unified_IRQ();
}

/***************************************************************************//**
 * @brief GPIO Interrupt handler for odd pins
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  GPIO_Unified_IRQ();
}

/***************************************************************************//**
 * @brief   Set up PCNT to generate an interrupt every second.
 *
 ******************************************************************************/
void PcntInit(void)
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
