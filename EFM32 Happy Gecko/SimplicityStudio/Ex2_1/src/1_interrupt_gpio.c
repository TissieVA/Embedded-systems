/**************************************************************************//**
 * @file
 * @brief GPIO Interrupt Example
 * @author Energy Micro AS
 * @version 1.0.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs Software License Agreement. See 
 * "http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt"  
 * for details. Before using this software for any purpose, you must agree to the 
 * terms of that agreement.
 *
 ******************************************************************************/
#include "em_device.h"
#include <stdint.h>
#include "em_chip.h"
#include "em_cmu.h"
#include "bsp.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_gpio.h"

/* Since the code can be run on both Giant Gecko and Tiny Gecko we need the following
   if statement since the pins for Push Button 0 and Push Button 1 are different for 
   the two chips */

/* Defines for Push Button 0 */
#define PB0_PORT    gpioPortC
#define PB0_PIN     9

/* Defines for Push Button 1 */
#define PB1_PORT    gpioPortC
#define PB1_PIN     10

/* Defines for the LED 0 */
#define LED0_PORT    gpioPortF
#define LED0_PIN     4

/* Defines for the LED 1 */
#define LED1_PORT    gpioPortF
#define LED1_PIN     5

bool enableLed0 = 0;
bool enableLed1 = 0;


void GPIO_IRQHandler_EVEN(void)
{
	GPIO_IntClear(1 << PB1_PIN);

	/* Toggle value */
	enableLed1 = !enableLed1;

	if (enableLed1)
	{
	/* Turn on LED */
	GPIO_PinOutSet(LED1_PORT, LED1_PIN);
	}
	else
	{
	/* Turn off LED )*/
	GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	}
}

void GPIO_IRQHandeler_ODD(void)
{
	GPIO_IntClear(1 << PB0_PIN);

	/* Toggle value */
	enableLed0 = !enableLed0;

	if (enableLed0)
	{
		/* Turn on LED */
		GPIO_PinOutSet(LED0_PORT, LED0_PIN);
	}
	else
	{
		/* Turn off LED )*/
		GPIO_PinOutClear(LED0_PORT, LED0_PIN);
	}
}

/**************************************************************************//**
 * @brief GPIO_EVEN Handler
 * Interrupt Service Routine for even GPIO pins
 *****************************************************************************/


void GPIO_EVEN_IRQHandler(void)
{
	GPIO_IRQHandler_EVEN();
}

/**************************************************************************//**
 * @brief GPIO_ODD Handler
 * Interrupt Service Routine for odd GPIO pins
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
	GPIO_IRQHandeler_ODD();
}



/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Initialize chip */
  CHIP_Init();

  /* Enable clock for GPIO module, we need this because
   *  the button and the LED are connected to GPIO pins. */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure push button 0 as an input,
   * so that we can read its value. */
  GPIO_PinModeSet(PB0_PORT, PB0_PIN, gpioModeInput, 1);

  /* Configure LED 0 as a push pull, so that we can
   * set its value - 1 to turn on, 0 to turn off */
  GPIO_PinModeSet(LED0_PORT, LED0_PIN, gpioModePushPull, 0);

  /* Configure push button 1 as an input,
     * so that we can read its value. */
    GPIO_PinModeSet(PB1_PORT, PB1_PIN, gpioModeInput, 1);

    /* Configure LED 1 as a push pull, so that we can
     * set its value - 1 to turn on, 0 to turn off */
    GPIO_PinModeSet(LED1_PORT, LED1_PIN, gpioModePushPull, 0);

  /* Enable GPIO_ODD interrupt vector in NVIC. We want Push Button 0 to
   * send an interrupt when pressed. GPIO interrupts are handled by either
   * GPIO_ODD or GPIO_EVEN, depending on whether the pin number is odd or even,
   * PB1 is therefore handled by GPIO_EVEN for Tiny Gecko (D8) and by GPIO_ODD for
   * Giant Gecko (B9). */
  

  NVIC_EnableIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);



  /* Configure PD8 (Push Button 0) interrupt on falling edge, i.e. when it is
   * pressed, and rising edge, i.e. when it is released. */
  GPIO_IntConfig(PB0_PORT, PB0_PIN, true, true, true);
  GPIO_IntConfig(PB1_PORT, PB1_PIN, true, true, true);

  while (1)
  {
    /* Go to EM3 */
    EMU_EnterEM3(true);
  }
}
