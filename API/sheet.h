/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SHEET_H
#define __SHEET_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "message.h"
/* Exported macro ------------------------------------------------------------*/
#define MODULENUM   15				/*模块个数*/

/* Exported types ------------------------------------------------------------*/
typedef struct						/*megmeet每一个模块的结构体定义*/
{
	uint8_t on;								/*设定值*/
	uint16_t mAcurrent;
	uint32_t mVvoltage;
	
	uint16_t statusCurrent;		/*注意是X10 A*/
	uint16_t statusVoltage;		/*注意是X10 V*/
	uint16_t status;					/*状态值*/
	
	uint8_t textChanged;			/*标注值是否改变，在每一次有数据改变的时候置一*/
														/*在对数值处理之后清零*/
	uint8_t duration;					/*单位：s 每一次textChanged就清零，并且开始计时*/
}ModuleSetting;

void ModuleSettingTableUpdate(struct message data);
void ModuleWriteVA(uint8_t moduleNum,\
									 uint32_t voltage,\
									 uint16_t current);
void ModuleWriteONOFF(uint8_t moduleNum,\
											uint8_t onOff);
void ModuleWriteStatus(uint8_t moduleNum,\
											 uint8_t status);
void ModuleSettingTableInit(void);
ModuleSetting * GetModuleSetting(void);
uint8_t ModuleAddr(uint8_t moduleNum);
#endif
