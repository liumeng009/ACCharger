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
#ifndef __ADC_H
#define __ADC_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"	//统一在各自的.h文件中包含
#include "message.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void adcInit(void);
void  RCC_Configuration_Adc(void);
void  NVIC_Configuration_Adc(void);
void  GPIO_Configuration_Adc(void);

uint16_t adc1Value(void);	//读取ADC1的转换结果
uint16_t adc2Value(void);   //读取ADC2的转换结果
uint16_t adc3Value(void);   //读取ADC2的转换结果

void AdcFilter(void);
void AVGTask(void);
#endif /* __ADC_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
