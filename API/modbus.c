/**
  ******************************************************************************
  * @file    modbus.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "modbus.h"
#include "usart.h"
#include "crc8_16.h"
#include "string.h"
#include "CANCOM-100E.h"
#include "iDM100E.h"
#include "AM2301.h"
#include "adc.h"
#include "table.h"
#include "rc522.h"
#include "mvc.h"
#include "rtc.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define SLAVE_ADDRESS 0x01

/* Private macro -------------------------------------------------------------*/
#define STATE_WAIT					0
#define STATE_FRAME_CHECK		1
#define STATE_FRAME_ADDR		2
#define STATE_FRAME_FUNC		3

/* Private variables ---------------------------------------------------------*/
uint8_t rxFrame[40];
uint8_t txFrame[120];

ModuleSetting * moduleTable;							/*麦格米特的功率模块表格*/
float * piDM100EVoltage;
float * piDM100ECurrent;
float * piDM100EW;
float * pTemperature;
float * pHumidity;

uint8_t keyValue;							/*键值变量 之所以不用队列 是因为没有必要 
																与之相应的API是GetKeyValue()*/

static uint8_t mAppState = STATE_WAIT;    // State tracking variable

/* Private function prototypes -----------------------------------------------*/
uint8_t crcCheck(uint8_t * pD, uint8_t num);
void modbusFunction(uint8_t * pD, uint8_t num);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  ModbusInit()初始化 modbus通信时依据的表格 moduleTable等等....
  * @param  None
  * @retval None
  */
void ModbusInit()
{
		moduleTable = GetModuleSetting();				/*获取表格*/
		piDM100EVoltage = GetVoltage();					/*电表读出的电压电流电量指针*/
		piDM100ECurrent = GetCurrent();
		piDM100EW = GetW();
		pTemperature = GetTemperature();
		pHumidity = GetHumidity();
}
/**
  * @brief  Main program 采用协同多任务 基本的流程是 0.等待接收 1.检查错误 2.判断地址 3.
	*	判断功能 4.根据功能扫描数据 5.返回数据 6.等待接收
	*					注意:变量要是static的！
  * @param  None
  * @retval None
  */
void ModbusTask()
{
//		static uint16_t timeCounter;
//		static uint16_t counter;
//		static uint16_t crcValue;
		static uint8_t	cNum;
		

		switch(mAppState)
		{
			case STATE_WAIT:											/*等待modbus数据请求*/
				cNum = UsartRead(2,rxFrame);
				if( cNum > 0)
				{
						mAppState = STATE_FRAME_CHECK;
				}
				break;
			case STATE_FRAME_CHECK:								/*检查modbus请求合法性*/
				if(crcCheck(rxFrame,cNum))
				{
						mAppState = STATE_FRAME_ADDR;
				}
				else
						mAppState = STATE_WAIT;
				break;
			case STATE_FRAME_ADDR:								/*看modbus请求是不是针对自己*/
				if(rxFrame[0] == SLAVE_ADDRESS)
						mAppState = STATE_FRAME_FUNC;
				else
						mAppState = STATE_WAIT;
				break;
			case STATE_FRAME_FUNC:								/*处理mobbus请求*/
				modbusFunction(rxFrame,cNum);
				mAppState = STATE_WAIT;
				break;
			default:break;
		}
}

/**
  * @brief  crcCheck检查接到的frame是不是正确的 注意num不能小于2！
  * @param  None
  * @retval 0：正确 1：错误
  */
uint8_t crcCheck(uint8_t * pD, uint8_t num)
{
		uint16_t temp;
		uint16_t crc;

		if(num < 2)	/*num值小于2 会导致数组反越界 然后hartware fault中断*/
				return 1;
		
		crc=Get_Crc16(pD,(num-2));
		
		temp = pD[num-2]<<8;
		temp = pD[num-1];
		if(crc == temp)
			return 0;
		else
			return 1;
}
/**
  * @brief  modbusFunction根据不同的功能码执行相应的操作
	* 				第一：要对周期更新的值进行处理
						第二：要对input进行处理 主要获得键值 获得键值以后，要么对控件内容进行加减操作
									要么把键值放入键值队列为状态机提供条件
						键值表：key       key-value
										空				0x00
										确定键		0x01
										取消键		0x02
										电量键    0x03
										金额键    0x04
										时间键		0x05
										自动键		0x06
										user键		0x07
										root键		0x08
										deve键		0x09
  * @param  None
  * @retval None
  */		
void modbusFunction(uint8_t * pD, uint8_t num)
{
		uint16_t startAddress;
		uint16_t numOfChannel;
		uint8_t i,j,u8Tmp;
		uint16_t u16Tmp;
		uint32_t u32Tmp;
		uint16_t crc;
		uint8_t moduleCounter;
	
		switch(pD[1])
		{
			/*功能码为04的情况 也就是传递表格的情况*/
			case 0x04:
				startAddress = (pD[2]<<8)|pD[3];	/*02 03 代表起始地址*/
				numOfChannel = (pD[4]<<8)|pD[5];	/*04 05 代表通道个数 就是有多少个数据*/
				txFrame[0] = pD[0];								/*00 地址码*/
				txFrame[1] = pD[1];								/*01 功能码*/
				txFrame[2] = numOfChannel*2;			/*字节数*/
				
				/*以下是数据*/

				txFrame[3] = MvcGetTransation().cardStatus.serialNum[0];
				txFrame[4] = MvcGetTransation().cardStatus.serialNum[1];
				txFrame[5] = MvcGetTransation().cardStatus.serialNum[2];
				txFrame[6] = MvcGetTransation().cardStatus.serialNum[3];
				txFrame[7] = (MvcGetTransation().cardStatus.rxWalletValue) >> 24;
				txFrame[8] = (MvcGetTransation().cardStatus.rxWalletValue) >> 16;
				txFrame[9] = (MvcGetTransation().cardStatus.rxWalletValue) >> 8;
				txFrame[10] = (MvcGetTransation().cardStatus.rxWalletValue);
				txFrame[11] = 0;
				txFrame[12] = MvcGetTransation().chargeMode;
				txFrame[13] = (MvcGetTransation().chargeTime)>>24;
				txFrame[14] = (MvcGetTransation().chargeTime)>>16;
				txFrame[15] = (MvcGetTransation().chargeTime)>>8;
				txFrame[16] = (MvcGetTransation().chargeTime);
				txFrame[17] = (MvcGetTransation().chargeMoney)>>24;
				txFrame[18] = (MvcGetTransation().chargeMoney)>>16;
				txFrame[19] = (MvcGetTransation().chargeMoney)>>8;
				txFrame[20] = (MvcGetTransation().chargeMoney);
				txFrame[21] = (MvcGetTransation().chargeElectricity)>>24;
				txFrame[22] = (MvcGetTransation().chargeElectricity)>>16;
				txFrame[23] = (MvcGetTransation().chargeElectricity)>>8;
				txFrame[24] = (MvcGetTransation().chargeElectricity);
				txFrame[25] = (MvcGetTransation().accumulateTime)>>24;
				txFrame[26] = (MvcGetTransation().accumulateTime)>>16;
				txFrame[27] = (MvcGetTransation().accumulateTime)>>8;
				txFrame[28] = (MvcGetTransation().accumulateTime);
				txFrame[29] = (MvcGetTransation().accumulateMoney)>>24;
				txFrame[30] = (MvcGetTransation().accumulateMoney)>>16;
				txFrame[31] = (MvcGetTransation().accumulateMoney)>>8;
				txFrame[32] = (MvcGetTransation().accumulateMoney);
				txFrame[33] = (MvcGetTransation().accumulateElectricity)>>24;
				txFrame[34] = (MvcGetTransation().accumulateElectricity)>>16;
				txFrame[35] = (MvcGetTransation().accumulateElectricity)>>8;
				txFrame[36] = (MvcGetTransation().accumulateElectricity);
				txFrame[37] = 0;
				txFrame[38] = MvcGetTransation().transactionStatus;
				txFrame[39] = MvcGetTransation().transactionSponsor[0];
				txFrame[40] = MvcGetTransation().transactionSponsor[1];
				txFrame[41] = MvcGetTransation().transactionSponsor[2];
				txFrame[42] = MvcGetTransation().transactionSponsor[3];
				txFrame[43] = MvcGetTransation().transactionPayer[0];
				txFrame[44] = MvcGetTransation().transactionPayer[1];
				txFrame[45] = MvcGetTransation().transactionPayer[2];
				txFrame[46] = MvcGetTransation().transactionPayer[3];
			  //txFrame[47] = adc3Value()>>8;
				//txFrame[48] = adc3Value();
				txFrame[47] = 0;				//抢连接状态
				txFrame[48] = MvcGetTransation().CCstate;
				txFrame[49] = 0;
				txFrame[50] = MvcGetTransation().rate * 100;
				txFrame[51] = 0;
				txFrame[52] = MvcGetTransation().moneyOK;
				txFrame[53] = 0;
				txFrame[54] = MvcGetTransation().matchStatus;
				txFrame[55] = 0;
				txFrame[56] = MvcGetTransation().lockStatus;
				//txFrame[57] = adc1Value()>>8;//0;
				//txFrame[58] = adc1Value();//*pTemperature;
				txFrame[57] = 0;
				txFrame[58] = *pTemperature;
				txFrame[59] = 0;
				txFrame[60] = *pHumidity;
				txFrame[61] = 0;
				txFrame[62] = MvcGetTransation().chargeStatus;
				txFrame[63] = 0;								/*界面指针*/
				txFrame[64] = MvcGetActiveState();
				
				txFrame[65] = 0;
				txFrame[66] = *piDM100ECurrent;
				txFrame[67] = 0;
				txFrame[68] = *piDM100EVoltage;
				txFrame[69] = (MvcGetTransation().w)>>24;
				txFrame[70] = (MvcGetTransation().w)>>16;
				txFrame[71] = (MvcGetTransation().w)>>8;
				txFrame[72] = (MvcGetTransation().w);
				txFrame[73] = 0;
				txFrame[74] = MvcGetTransation().cardStatus.status;

				txFrame[75] = (MvcGetTransation().rtcTime.year)>>8;
				txFrame[76] = (MvcGetTransation().rtcTime.year);
				txFrame[77] = 0;
				txFrame[78] = (MvcGetTransation().rtcTime.month);
				txFrame[79] = 0;
				txFrame[80] = (MvcGetTransation().rtcTime.day);
				txFrame[81] = 0;
				txFrame[82] = (MvcGetTransation().rtcTime.hour);
				txFrame[83] = 0;
				txFrame[84] = (MvcGetTransation().rtcTime.minute);
				txFrame[85] = 0;
				txFrame[86] = (MvcGetTransation().rtcTime.second);
				
				txFrame[87] = (MvcGetTransation().startTime.year)>>8;
				txFrame[88] = (MvcGetTransation().startTime.year);
				txFrame[89] = 0;
				txFrame[90] = (MvcGetTransation().startTime.month);
				txFrame[91] = 0;
				txFrame[92] = (MvcGetTransation().startTime.day);
				txFrame[93] = 0;
				txFrame[94] = (MvcGetTransation().startTime.hour);
				txFrame[95] = 0;
				txFrame[96] = (MvcGetTransation().startTime.minute);
				txFrame[97] = 0;
				txFrame[98] = (MvcGetTransation().startTime.second);
				
				txFrame[99] = (MvcGetTransation().endTime.year)>>8;
				txFrame[100] = (MvcGetTransation().endTime.year);
				txFrame[101] = 0;
				txFrame[102] = (MvcGetTransation().endTime.month);
				txFrame[103] = 0;
				txFrame[104] = (MvcGetTransation().endTime.day);
				txFrame[105] = 0;
				txFrame[106] = (MvcGetTransation().endTime.hour);
				txFrame[107] = 0;
				txFrame[108] = (MvcGetTransation().endTime.minute);
				txFrame[109] = 0;
				txFrame[110] = (MvcGetTransation().endTime.second);
				txFrame[111] = 0;
				txFrame[112] = 0;
				txFrame[113] = 0;
				txFrame[114] = 0;
				
				/*
				txFrame[17] = adc3Value()>>8;
				txFrame[18] = adc3Value();
				txFrame[19] = adc1Value()>>8;
				txFrame[20] = adc1Value();
				*/
				
				crc=Get_Crc16(txFrame,115);			/*填入要校验的个数*/
				txFrame[115] = (crc&0xff00)>>8;	/*把生成的校验结果 填入相应序号*/
				txFrame[116] = crc&0xff;
				
				UsartWrite(2,txFrame,117);
			
				break;
			case 0x05:
				/*功能码为05的情况 也就是处理键值的情况*/
				UsartWrite(2,pD,num);
				if(pD[4] == 0xff)		/*按下*/
					keyValue =  pD[3];
				else								/*抬起*/
					/**************/;
				break;
			case 0x10:
				/*功能码为16的情况 也就只写的情况*/
				txFrame[0] = pD[0];								/*00 地址码*/
				txFrame[1] = pD[1];								/*01 功能码*/
				txFrame[2] = pD[2];								/*字节数*/
				txFrame[3] = pD[3];								/*字节数*/
				txFrame[4] = pD[4];								/*通道数*/
				txFrame[5] = pD[5];								/*通道数*/
				crc=Get_Crc16(txFrame,6);			/*填入要校验的个数*/
				txFrame[6] = (crc&0xff00)>>8;	/*把生成的校验结果 填入相应序号*/
				txFrame[7] = crc&0xff;
				UsartWrite(2,txFrame,8);
			
				/*数据处理*/
				u32Tmp = (pD[7]<<24)|(pD[8]<<16)|(pD[9]<<8)|(pD[10]);
				MvcSetChargeValue(u32Tmp);
				break;
			default:break;
		}
}

/**
  * @brief  获得键值 获得完之后就将键值清零 也就是没有按键按下的状态
						其实就是按键分为有按键按下和没有按键按下两种状态
						有按键按下就读取键值
						按键的时间长短又分为不按和长按和短按三种状态 这不过这里
						不考虑时间罢了
						键值表：key       key-value
										空				0x00
										确定键		0x01
										取消键		0x02
										电量键    0x05
										金额键    0x0
										时间键		0x0
										自动键		0x0
										user键		0x04
										root键		0x03
										deve键		0x09
  * @param  None
  * @retval None
  */
uint8_t GetKeyValue(void)
{
		uint8_t temp;
	
		temp = keyValue;
		keyValue = 0;			/*清零*/
		return temp;
}