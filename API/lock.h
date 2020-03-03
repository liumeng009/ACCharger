/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOCK_H
#define __LOCK_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "message.h"
/* Exported macro ------------------------------------------------------------*/
#define UNLOCK 0
#define LOCK	 1
/* Exported types ------------------------------------------------------------*/
void LockInit(void);
uint8_t LockSet(uint8_t cmd);
uint8_t GetLockStatus(void);
void LockTask(void);
#endif
