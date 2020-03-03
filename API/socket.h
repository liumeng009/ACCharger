/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SOCKET_H
#define __SOCKET_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "message.h"
/* Exported macro ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
void Timer2_Init_Config(void);		//Timer2初始化配置
void System_Initialization(void);	//STM32系统初始化函数(初始化STM32时钟及外设)
void W5500Delay(unsigned int d);			//延时函数(ms)
int W5500Init(void);
void W5500Task(void);
#endif
