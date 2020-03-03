/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIME_H
#define __TIME_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
void TimeInit(void);
uint16_t Tim6GetCounterValue(void);
void Tim6SetCounterValue(uint16_t value);

#endif
