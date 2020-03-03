/**
  ******************************************************************************
  * @file    SysTick/TimeBase/main.h 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Header for main.c module
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
#ifndef __SYSTICK_H
#define __SYSTICK_H
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "message.h"
#include "usart.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int sysTickInit(void);
uint32_t GetTickCount(void);
/*闹钟定时器*/
void vUsart2TimerSetAlarm(uint8_t alarmValue);
void vUsart3TimerSetAlarm(uint8_t alarmValue);
void vUsart4TimerSetAlarm(uint8_t alarmValue);

void vCanTimerSetAlarm(uint8_t alarmValue);
void vCanTimerClose(void);
/*普通定时器*/
void vMegmeetTimerSetValue(uint8_t initValue);
uint16_t vMegmeetTimerGetValue(void);

void vAdcTimerSetValue(uint8_t initValue);
uint16_t vAdcTimerGetValue();

void vRC522TimerSetValue(uint8_t initValue);
uint16_t vRC522TimerGetValue();

void vW5500TimerSetValue(uint8_t initValue);
uint16_t vW5500TimerGetValue();

void vLockTimerSetValue(uint16_t initValue);
uint16_t vLockTimerGetValue();

void vLedTimerSetValue(uint16_t initValue);
uint16_t vLedTimerGetValue();
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
