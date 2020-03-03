/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CANCOM_100E_H
#define __CANCOM_100E_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "sheet.h"
/* Exported macro ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

void FrameTx(	uint8_t FF,\
							uint8_t RTR,\
							uint8_t dataLength,\
							uint32_t ID,\
							uint8_t * pD);

void MegmeetTx(uint8_t moduleNum, uint8_t CMD, ModuleSetting setting);

#endif
