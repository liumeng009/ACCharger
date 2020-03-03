/**
  ******************************************************************************
  * @file    table.h 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Evaluation board specific configuration file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TABLE_H
#define __TABLE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"	
#include "adc.h"
#include "usart.h"
#include "can.h"
#include "message.h"
/* Exported types ------------------------------------------------------------*/
typedef struct 
{
	uint8_t queue[20];     //条件存储
	uint8_t size;		  //当前个数
}conditionQueue;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void tableInit(void);
void stateMachineOpen(void);
void stateMachineClose(void);
void stateTimeUpdate(void);
void conditionUpdate(void);
void stateTransitionUpdate(void);
void ConditionInsert(conditionQueue * pC ,uint8_t conditionCode);
	
void messageRecvUpdate(uint8_t counter);
void messageTimeTableSet(uint8_t counter);

void ActionExecutor(struct message actionMessage);
uint8_t GetActiveState(void);
uint8_t AdcValueToCCstate();
uint8_t GetSuperState(void);
#endif /* __TABLE_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
