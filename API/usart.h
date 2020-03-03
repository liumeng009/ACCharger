/**
  ******************************************************************************
  * @file    USART/Interrupt/platform_config.h 
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
#ifndef __USART_H
#define __USART_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "systick.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

//采用的宏定义！！！
  #define USARTy                   USART1
  #define USARTy_GPIO              GPIOA
  #define USARTy_CLK               RCC_APB2Periph_USART1
  #define USARTy_GPIO_CLK          RCC_APB2Periph_GPIOA
  #define USARTy_RxPin             GPIO_Pin_10
  #define USARTy_TxPin             GPIO_Pin_9
  #define USARTy_IRQn              USART1_IRQn
  #define USARTy_IRQHandler        USART1_IRQHandler
  
  #define USARTz                   USART2
  #define USARTz_GPIO              GPIOA
  #define USARTz_CLK               RCC_APB1Periph_USART2
  #define USARTz_GPIO_CLK          RCC_APB2Periph_GPIOA
  #define USARTz_RxPin             GPIO_Pin_3
  #define USARTz_TxPin             GPIO_Pin_2
  #define USARTz_IRQn              USART2_IRQn
  #define USARTz_IRQHandler        USART2_IRQHandler
  
	#define USARTa                   USART3
  #define USARTa_GPIO              GPIOB
  #define USARTa_CLK               RCC_APB1Periph_USART3
  #define USARTa_GPIO_CLK          RCC_APB2Periph_GPIOB
  #define USARTa_RxPin             GPIO_Pin_11
  #define USARTa_TxPin             GPIO_Pin_10
  #define USARTa_IRQn              USART3_IRQn
  #define USARTa_IRQHandler        USART3_IRQHandler
	
	#define USARTb                   UART4
  #define USARTb_GPIO              GPIOC
  #define USARTb_CLK               RCC_APB1Periph_UART4
  #define USARTb_GPIO_CLK          RCC_APB2Periph_GPIOC
  #define USARTb_RxPin             GPIO_Pin_11
  #define USARTb_TxPin             GPIO_Pin_10
  #define USARTb_IRQn              UART4_IRQn
  #define USARTb_IRQHandler        UART4_IRQHandler
	
	#define USARTc                   UART5
  #define USARTc_GPIO              GPIOD
  #define USARTc_CLK               RCC_APB1Periph_UART5
  #define USARTc_GPIO_RX_CLK       RCC_APB2Periph_GPIOD
	#define USARTc_GPIO_TX_CLK       RCC_APB2Periph_GPIOC
  #define USARTc_RxPin             GPIO_Pin_2
  #define USARTc_TxPin             GPIO_Pin_12
  #define USARTc_IRQn              UART5_IRQn
  #define USARTc_IRQHandler        UART5_IRQHandler

/* Exported macro ------------------------------------------------------------*/
#define USART1_485 0		/*485开启宏定义*/
#define USART2_485 1		/*485开启宏定义*/
#define USART3_485 1		/*485开启宏定义*/
#define USART4_485 1		/*485开启宏定义*/

/* Exported functions ------------------------------------------------------- */
int usartInit(void);
void usart1SendByte(uint8_t byte);
void Usart2SetReady(void);
void Usart3SetReady(void);
void Usart4SetReady(void);
void USART2Write(uint8_t *Data, uint8_t len);
void MbTransceiver(struct message data);
uint8_t UsartWrite(uint8_t UsartNum, uint8_t * pD , uint8_t size);
uint8_t UsartRead(uint8_t UsartNum, uint8_t * pD);
void Delay(u32 i);
#endif /* __PLATFORM_CONFIG_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
