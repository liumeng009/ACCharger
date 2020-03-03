/**
  ******************************************************************************
  * @file    sheet.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * 此文件描述周立功CANCOM_100E的格式转换模式的用法，也就是发送规定形式的485数据
  *	帧，和接收规定形式的485数据帧，以及发送之前如何通过结构体构成帧，和接收之后
	*	如何解析帧
	* 整个过程无时间控制 主要是 帧结构的定义和一个实时接受任务
	* 本文件采用的硬件接口是Usart3 别的不要再使用了
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "sheet.h"
#include "string.h"
#include "usart.h"
#include "CANCOM-100E.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
#define MODULE1ADDR 0x01		/*模块地址*/
#define MODULE2ADDR 0x02
#define MODULE3ADDR 0x03
#define MODULE4ADDR 0x04
#define MODULE5ADDR 0x05
#define MODULE6ADDR 0x06
#define MODULE7ADDR 0x07
#define MODULE8ADDR 0x08
#define MODULE9ADDR 0x09
#define MODULE10ADDR 0x0a
#define MODULE11ADDR 0x0b
#define MODULE12ADDR 0x0c
#define MODULE13ADDR 0x0d
#define MODULE14ADDR 0x0e
#define MODULE15ADDR 0x0f

#define MODULEON			0x55
#define	MODULEOFF			0xaa
/*缺省值代表着有效范围内的典型值 是容错的有效手段*/
#define VOLTAGE_DEFAULT_VALUE 400000	/*缺省值 单位：mV*/
#define CURRENT_DEFAULT_VALUE 6000		/*缺省值 单位：mA*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint8_t	onOffUpdatePointer = 0;			/*这三个量指示当前要更新的量*/
uint8_t	VAUpdatePointer = 0;
uint8_t	statusUpdatePointer = 0;

ModuleSetting moduleSettingTable[MODULENUM];	/*15个模块的表格 提供操作API*/


/*15个模块的地址 注意设置*/
const uint8_t moduleAddr[MODULENUM] = {MODULE1ADDR,MODULE2ADDR,MODULE3ADDR,MODULE4ADDR,
																 MODULE5ADDR,MODULE6ADDR,MODULE7ADDR,MODULE8ADDR,
																 MODULE9ADDR,MODULE10ADDR,MODULE11ADDR,MODULE12ADDR,
																 MODULE13ADDR,MODULE14ADDR,MODULE15ADDR};

/* Private function prototypes -----------------------------------------------*/
void ModuleSettingMonitor(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  ModuleSettingTableUpdate()同时处理开关量、电压、电流更新，更新操作是
						在每个500ms时间节点上，更新的对象由表格自己维护（顺序），自动执行
						这样表格就一直在更新了
  * @param  
  * @retval 
  */
void ModuleSettingTableUpdate(struct message data)
{
		/*判断pointer合法性*/
		
		/*执行CANCOM的发送操作 数据由表格提供*/
		switch(data.cmd)
		{
			case 0:/*更新模拟量*/
				MegmeetTx(VAUpdatePointer,0,moduleSettingTable[VAUpdatePointer]);
				//发送消息moduleSettingTable[onOffUpdatePointer].mAcurrent;
				//发送消息moduleSettingTable[onOffUpdatePointer].mVvoltage;
				VAUpdatePointer++;
				if(VAUpdatePointer == MODULENUM)	/*==15 超出范围了*/
						VAUpdatePointer = 0;
				break;
			case 1:/*更新状态*/
				MegmeetTx(statusUpdatePointer,1,moduleSettingTable[statusUpdatePointer]);
				statusUpdatePointer++;
				if(VAUpdatePointer == MODULENUM)	/*==15 超出范围了*/
						VAUpdatePointer = 0;
				break;
			case 2:/*更新开关量*/
				//发送消息moduleSettingTable[onOffUpdatePointer].on
				MegmeetTx(onOffUpdatePointer,2,moduleSettingTable[onOffUpdatePointer]);
				onOffUpdatePointer++;
				if(onOffUpdatePointer == MODULENUM)	/*==15 超出范围了*/
						onOffUpdatePointer = 0;
				break;
			default:break;
		}
}	

/**
  * @brief  ModuleWriteVA()写入电压电流
  * @param  
  * @retval 
  */
void ModuleWriteVA(uint8_t moduleNum,\
									 uint32_t voltage,\
									 uint16_t current)
{
		if(moduleNum > MODULENUM)
			return;
		moduleSettingTable[moduleNum].mAcurrent = current;
		moduleSettingTable[moduleNum].mVvoltage = voltage;
}	
/**
  * @brief  ModuleWriteONOFF()写入开关量
  * @param  
  * @retval 
  */
void ModuleWriteONOFF(uint8_t moduleNum,\
											uint8_t onOff)
{
		if(moduleNum > MODULENUM)
			return;
		moduleSettingTable[moduleNum].on = onOff;
}	
/**
  * @brief  ModuleWriteStatus()写入状态量
  * @param  
  * @retval 
  */
void ModuleWriteStatus(uint8_t moduleNum,\
											 uint8_t status)
{
		if(moduleNum > MODULENUM)
			return;
		moduleSettingTable[moduleNum].status = status;
}	
/**
  * @brief  void MegmeetInit()
  * @param  
  * @retval 
  */
void ModuleSettingTableInit()
{
		uint8_t i;
		for(i = 0; i< MODULENUM; i++)
		{
				moduleSettingTable[i].on = MODULEON;					/*0x55 ：开  0xaa： 关*/
				moduleSettingTable[i].mVvoltage = VOLTAGE_DEFAULT_VALUE;
				moduleSettingTable[i].mAcurrent = CURRENT_DEFAULT_VALUE;
		}
}	

/**
  * @brief  GetModuleSetting用于返回表格 做外部访问 是比较规范的方式
  * @param  None
  * @retval None
  */	
ModuleSetting * GetModuleSetting()
{
	return moduleSettingTable;
}
uint8_t ModuleAddr(uint8_t moduleNum)
{
	return moduleAddr[moduleNum];
}

/**
  * @brief  ModuleSettingMonitor数据正确性检测器 首先，填入表格的数据一定要正确
		之后，表格中的数据有两种属性，一种是期望值，就是你期望数据会是什么，另一种是
		实际值。就是这个数据真实的状态。从期望值到真实值有一个周期。那么期望值在变化
·		后，在固定t周期内，如果没有达到期望值或期望值的相邻范围内，那么可以认为模块
		已经失去正常反馈的能力（坏了），如果达到了期望值，那么说明正确。但是如果期望
		值在不停地变化，那么真实值自然也会在不停变化，那么真实值的判断就没有意义了，
		那么就要缩小真实值的判断频率，使其具有真正的参考价值。
  * @param  None
  * @retval None
  */	
void ModuleSettingMonitor()
{
		uint8_t counter;
		for(counter = 0; counter < MODULENUM; counter++)
		{
				if(moduleSettingTable[counter].on == MODULEOFF)
				{
						moduleSettingTable[counter].mVvoltage = VOLTAGE_DEFAULT_VALUE;
						moduleSettingTable[counter].mAcurrent = CURRENT_DEFAULT_VALUE;
				}
				else
				{
						if(moduleSettingTable[counter].mVvoltage < 400000)
								moduleSettingTable[counter].mVvoltage = VOLTAGE_DEFAULT_VALUE;
						if(moduleSettingTable[counter].mVvoltage > 600000)
								moduleSettingTable[counter].mVvoltage = CURRENT_DEFAULT_VALUE;
						if(moduleSettingTable[counter].mAcurrent > 6000)
								moduleSettingTable[counter].mVvoltage = 6000;
				}
		}
}
