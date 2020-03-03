/**
  ******************************************************************************
  * @file    CAN/Networking/platform_config.h 
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
#ifndef __CAN_H
#define __CAN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "message.h"
#include "table.h"

/* Exported macro ------------------------------------------------------------*/
#define CAN_BUFFER_SIZE 20
/* Exported types ------------------------------------------------------------*/
/*用作接收缓存的顺序存储循环队列 只在中断中插入can帧 只在程序中提取can帧*/
struct canQueueRx		
{
	CanRxMsg canBuffer[CAN_BUFFER_SIZE];
	uint8_t front;
	uint8_t rear;
};
/*用作发送缓存的顺序存储循环队列  只在程序中提取can帧*/
struct canQueueTx		
{
	CanTxMsg canBuffer[CAN_BUFFER_SIZE];
	uint8_t front;
	uint8_t rear;
};
struct linkStatus
{
		uint8_t linkEstablished;	/*连接建立标志*/
		uint8_t RTSReceived;
		uint8_t CTSSended;
		uint8_t emSended;
		struct
		{
			uint8_t available;
			uint8_t data[8];
		}buffer[80];							/*存储包*/
		uint16_t numOfByte;				/*字节个数*/
		uint8_t numOfPackage;			/*包个数*/
		uint8_t PGNCodeLow;
		uint8_t PGNCode;					/*PGN编号*/
		uint8_t PGNCodeHigh;
};
/* Exported constants --------------------------------------------------------*/

/* Define the STM32F10x hardware depending on the used evaluation board */
		//配置
#define RCC_APB2Periph_GPIO_CAN1    RCC_APB2Periph_GPIOA
#define GPIO_Remapping_CAN1         GPIO_Remap2_CAN1
#define GPIO_CAN1                   GPIOA  
#define GPIO_Pin_CAN1_RX            GPIO_Pin_11
#define GPIO_Pin_CAN1_TX            GPIO_Pin_12

/* Exported functions ------------------------------------------------------- */
int canInit(void);

void CanQueueInit(void);
uint8_t CanQueueRxDequeue(CanRxMsg * data_pointer);
uint8_t CanQueueTxEnqueue(CanTxMsg data);
uint8_t CanQueueTxDequeue(CanTxMsg * data_pointer);

void CanTask(void);											/*第二层任务*/
void canRxHandle(CanRxMsg rxMessage);		/*第二层接收处理*/
void canTxHandle(void);									/*第二层发送处理*/

void CanTxTask(void);		/*第一层发送任务*/
void LinkClose(void);
#endif /* __CAN_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
