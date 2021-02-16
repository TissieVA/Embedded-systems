/******************************************************************************
 * @file 1_registers.c
 * @brief Register Operation example
 * @author Silicon Labs
 * @version 1.18
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/

#include "em_device.h"
#include <stdint.h>
#include "em_chip.h"
#include "em_cmu.h"
#include "bsp.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"

/******************************************************************************
 * @brief  Main function
 * Main is called from _program_start, see assembly startup file
 *****************************************************************************/
int main(void)
{

  CHIP_Init();  

  
  // Enable timer
  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER0;

  // Start timer
  TIMER0->CMD = 0x1;

  // Wait for counter to be 1000
  while( TIMER0->CNT <1000)
  {

  }

  // Creating variables


  uint8_t EightBit = 129;
  int32_t *EightBit_ptr = &EightBit;
  uint16_t SixteenBit = 32769;
  int32_t *SixteenBit_ptr = &SixteenBit;
  uint32_t ThritytwoBit = 80000001;
  uint32_t *ThritytwoBit_ptr = &ThritytwoBit;
  uint32_t A [3];
  A[0] = 0;
  A[1] = 1;
  A[2] = 2;
  uint32_t *A_ptr = &A;

CMU_ClockEnable(cmuClock_GPIO, true);
GPIO_PinModeSet(gpioPortF, 4, gpioModePushPull, 1);

  while (1)
  {
	  if(TIMER0->CNT % 2)
	  {

	  }
	  else
	  {
		  BSP_LedsSet(0);
	  }
  }
}

void LedController(int led, bool onOff)
{
	switch(led)
	{
		case 0:
			if(onOff)
				GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 1);
			else
				GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 0);
	}
}
