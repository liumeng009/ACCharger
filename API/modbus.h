/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODBUS_H
#define __MODBUS_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
void ModbusInit(void);
void ModbusTask(void);
uint8_t GetKeyValue(void);
#endif
