/**
  ******************************************************************************
  * @file    Project/STM32F4xx_StdPeriph_Templates/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.6.0
  * @date    04-September-2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"

/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile int msTicks = 0;				// Variable für Timer
volatile int flag_10ms = 0;			// Variable für Flag CAN TxMessage
CanRxMsg RxMessage;							// Variable für RxMessage
extern volatile int p;					// Externe Variable von main.c
extern volatile int loop;				// Externe Variable von main.c
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	msTicks++;																					// Variable zählt hoch im Millisekundentakt
	
	if(!(msTicks%10)) {																	// if-Bedingung gibt "true" jedesmal wenn "msTicks" ohne Rest durch 10 teilbar ist.
		flag_10ms = 1;																		// Ist die if-Bedingung erfüllt, wird die Variable "flag_10ms" auf eins gesetzt.
	}
}

void CAN1_RX0_IRQHandler(void)												// Dieser Interrupt wird jedesmal aufgerufen, wenn eine CAN Nachricht empfangen wird.
{
  CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);						// Hier wird die CAN Nachricht empfangen	
	if (RxMessage.ExtId == 0x18FF3120)									// Überprüfen ob CAN Nachricht, die ID 18FF3120 hat
	{
		if (RxMessage.Data[0] == 0x7F){										// Hier werden die Databytes "0" und "1" auf die gewünschten Werte geprüft
			if (RxMessage.Data[1] == 0x7D){
				if (RxMessage.ExtId == 0x18FF3121){						// Überprüfen ob CAN Nachricht, die ID 18FF3121 hat
					if(RxMessage.Data[0] == 0x7F){
						if(RxMessage.Data[1] == 0x7D){						// Hier werden die Databytes "0" und "1" auf die gewünschten Werte geprüft
							p++;																		// "p" wird um einen Schritt erhöht wenn alle Bedingungen erfüllt sind (beide Motoren im Stillstand)
							loop = 0;																// "loop" wird wieder auf Null gesetzt. So wird die neue Position im Main wieder als NUllposition eingelesen.
						}
				}
				
			}
	}
}
}
	}
/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
