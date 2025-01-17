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
#include "em_rtc.h"

#include "display.h"
#include "textdisplay.h"
#include "retargettextdisplay.h"
#include "em4config.h"



static volatile bool displayEnabled = false; /* Status of LCD display. */

static DISPLAY_Device_t displayDevice;    /* Display device handle.         */

static void GpioSetup(void);

/* Defines for Push Button 0 & 1 */
#define PB0_PORT                    gpioPortC
#define PB0_PIN                     9
#define PB1_PORT                    gpioPortC
#define PB1_PIN                     10

/* Defines for the LED */
#define LED_PORT                    gpioPortF
#define LED_PIN                     4

/* Defines for the RTC */
#define LFXO_FREQUENCY              32768
#define WAKEUP_INTERVAL_MS          10
#define RTC_COUNT_BETWEEN_WAKEUP    ((LFXO_FREQUENCY * WAKEUP_INTERVAL_MS) / 1000)

/* The time of the stopwatch*/
uint32_t time = 0;

/* Increment the stopwatch? */
bool enableCount = false;

/* Display the gecko on the LCD? */
bool enableGecko = false;





/***************************************************************************//**
 * @brief Setup GPIO interrupt for pushbuttons.
 ******************************************************************************/
static void GpioSetup(void)
{
  /* Enable GPIO clock. */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure Push Button 0 and Push Button 1 as an input,
   * so that we can read their values. */
  GPIO_PinModeSet(PB0_PORT, PB0_PIN, gpioModeInput, 1);
  GPIO_PinModeSet(PB1_PORT, PB1_PIN, gpioModeInput, 1);

  /* Configure PC0 as a push pull for LED drive */
  GPIO_PinModeSet(LED_PORT, LED_PIN, gpioModePushPull, 0);

  NVIC_EnableIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(PendSV_IRQn);

  /* Configure interrupts on falling edge for pins D8/B9 (Push Button 0),
   * B11/B10 (Push Button 1) and D3 */
  GPIO_IntConfig(PB0_PORT, PB0_PIN, false, true, true);
  GPIO_IntConfig(PB1_PORT, PB1_PIN, false, true, true);

  NVIC_SetPriority(GPIO_ODD_IRQn, 2);
  NVIC_SetPriority(GPIO_EVEN_IRQn, 1);
  NVIC_SetPriority(RTC_IRQn, 0);
  NVIC_SetPriority(PendSV_IRQn, 3);

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

/**************************************************************************//**
 * @brief GPIO Interrupt Handler
 *****************************************************************************/
void GPIO_IRQHandler_1(void)
{
  /* Clear flag for Push Button 1 (pin D8) interrupt */
  GPIO_IntClear(1 << PB0_PIN);

  /* Toggle enableCount to start/pause the stopwatch */
  if (enableCount)
  {
    enableCount = false;
    printf("\nPause");
  }
  else
  {
    /* The interrupt handler is called to compute the proper time
     * for the next interrupt, since enableCount is 0, the time will not
     * be increased. */
    RTC_IRQHandler();

    printf("\nStart");
    enableCount = true;
  }
}

/**************************************************************************//**
 * @brief GPIO Interrupt Handler
 *****************************************************************************/
void GPIO_IRQHandler_2(void)
{
  /* Get the interrupt source, either Push Button 2 (pin B11) or pin D3 */
  uint32_t interrupt_source = GPIO_IntGet();

  /* Push Button 2 (pin B11) */
  if (interrupt_source & (1 << PB1_PIN))
  {
    GPIO_IntClear(1 << PB1_PIN);
    printf("\n\n\n-----Clear-----\n");
    time        = 0;
    enableCount = false;
    printf("%lu",time);
  }

  /* Pin D3 - channel 3 => 2^3 */
  if (interrupt_source & (1 << 3))
  {
    /* Operations can be made atomic, i.e. it cannot be interrupted by
     * interrupts with higher priorities, by disabling iterrupts. Uncomment
     * __disable_irq(); and __enable_irq(); to see how the update of the time
     * is delayed by the dummy loop below.*/
    /* __disable_irq(); */

    /* Toggle enableGecko */
    if (enableGecko)
      enableGecko = false;
    else
      enableGecko = true;

    printf("Gecko");

    /* This dummy loop is intended to illustrate the different levels of
     * priority. The timer will continue to update the LCD display, but
     * interrupts produced by Push Button 1 and 2 will not be served until
     * after this function has finished. */
    for (uint32_t tmp = 0; tmp < 2000000; tmp++) ;

    GPIO_IntClear(1 << 3);

    /* Enable interrupts again */
    /* __enable_irq(); */
  }
}

void PendSV_Handler(void)
{
  printf("%lu \n",time);
}



/***************************************************************************//**
 * @brief GPIO Interrupt handler for even pins
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  GPIO_IRQHandler_2();
}

void GPIO_ODD_IRQHandler(void)
{
	GPIO_IRQHandler_1();
}

/**************************************************************************//**
 * @brief Real Time Counter Interrupt Handler
 *****************************************************************************/
void RTC_IRQHandler(void)
{
  /* Clear interrupt source */
  RTC_IntClear(RTC_IFC_COMP0);

  /* Increase the timer when appropriate. */
  if (enableCount)
    time++;

  /* Only update the number if necessary */
  if (enableCount)
    /* Set lower priority interrupt which will process data */
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/**************************************************************************//**
 * @brief Initialize Real Time Counter
 *****************************************************************************/
void initRTC()
{
  /* Starting LFXO and waiting until it is stable */
  CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

  /* Routing the LFXO clock to the RTC */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_RTC, true);

  /* Enabling clock to the interface of the low energy modules */
  CMU_ClockEnable(cmuClock_CORELE, true);

  const RTC_Init_TypeDef rtcInit =
  {
    .enable   = true,
    .debugRun = false,
    .comp0Top = true,
  };

  RTC_Init(&rtcInit);

  /* Set comapre value for compare register 0 */
  RTC_CompareSet(0, RTC_COUNT_BETWEEN_WAKEUP);

  /* Enable interrupt for compare register 0 */
  RTC_IntEnable(RTC_IFC_COMP0);

  /* Enabling Interrupt from RTC */
  NVIC_EnableIRQ(RTC_IRQn);
}

int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* Setup GPIO for pushbuttons. */
  GpioSetup();
  initRTC();
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


  //printf("TIJS\n");

  while (1)
    {
      /* Go to EM2 */
      EMU_EnterEM2(true);
      /* Wait for interrupts */
    }
}

