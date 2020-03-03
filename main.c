/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/main.c 
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
#include "gpio.h"
#include "key.h"
#include "usart.h"
#include "systick.h"
#include "message.h"
#include "can.h"
#include "adc.h"
#include "table.h"
#include "AM2301.h"
#include "time.h"
#include "iDM100E.h"
#include "modbus.h"
#include "CANCOM-100E.h"
#include "sheet.h"
#include "rtc.h"
#include "pgn.h"
#include "spi.h"
#include "mcp2515.h"
#include "spibysoft.h"
#include "rc522.h"
#include "socket.h"
#include "lock.h"
#include "mvc.h"
//void Delay(u32 i);
extern uint8_t queueSwitch;		//使用消息系统切换标志位

/****************************************************
函数名：
形参：
返回值：
函数功能：
****************************************************/
int main(void)
{
	uint8_t returnValue;
	uint8_t addr;
	uint8_t i,j;
	char status;
	uint8_t a[] = {0x68,0x50,0x00,0x01,0x07,0x01,0x16,0x68,0x11,0x04,0x33,0x34,0x34,0x35,0x24,0x16};
	uint8_t b[20];
	uint8_t c[20];
	uint8_t Cmd_Read[9]=	{0x09,0xA1,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x38};//读数据指令---块：0x38
	struct message messageA;			//普通消息
	struct message messageB;			//普通消息
	GPIOInit();
	LockInit();
	sysTickInit();
	TimeInit();
	queueInit();
	canInit();
	adcInit();
	ModbusInit();
	ModuleSettingTableInit();
	usartInit();
	RtcInit();
	SpiInit();
	Mcp2515Init();
	SpiBySoftInit();
	RFID_Init();
	//W5500Init();
	MvcTableInit();				/*主逻辑状态机初始化*/
	MvcStateMachineOpen();
	usart1SendByte(0xee);	/*代表系统时钟和串口正常 可以调试*/
	usart1SendByte(0xff);
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
//	GPIO_SetBits(GPIOE, GPIO_Pin_3);	/*PWM*/
	while(1)
	{
		queueSwitch = 0;	 /*队列A正在处理中 禁止中断插入消息*/
		returnValue = queueDequeue(queueSwitch,&messageA);	//data_A在此区间内在赋值之前有用
		if(returnValue == 0)
		{	
			addr = messageA.destinationAddress;	
			switch(addr)
			{
				case 0:break;
				case 1:stateTimeUpdate();
							 MvcStateTimeUpdate();break;					/*时间*/
				case 2:conditionUpdate();										/*条件*/
							 stateTransitionUpdate();break;				/*迁移*/
				case 3:transactionTableUpdate();
							 MvcConditionUpdate();								/*条件*/
							 MvcStateTransitionUpdate();break;		/*迁移*/
				//case 3:WalletWrite(1234);break;
				case 4:ActionExecutor(messageA);break;			/*动作*/
				case 5:MvcActionExecutor(messageA);break;
				//case 5:SendCANmsg(0,0x12345678,a,0x18);break;
				case 6:iDM100ECommunication(messageA);break;
				//case 7:ModuleSettingTableUpdate(messageA);break;
				case 8:sysMonitor();break;
				//case 9:Mcp2515Receive();break;
				case 10:MFRC522Update();break;
				case 11:j =(j==0?1:0);LockSet(j);break;
				case 12:TestRtc();break;
				case 13:EmergencyStop();break;
				default:break;
			}
		}
		queueSwitch = 1;	/*队列B正在处理中 禁止中断插入消息*/
		returnValue = queueDequeue(queueSwitch,&messageB);
		if(returnValue == 0)
		{
			addr = messageB.destinationAddress;
			switch(addr)
			{
				case 0:break;
				case 1:stateTimeUpdate();
							 MvcStateTimeUpdate();break;					/*时间*/
				case 2:conditionUpdate();										/*条件*/
							 stateTransitionUpdate();break;				/*迁移*/
				case 3:transactionTableUpdate();
							 MvcConditionUpdate();								/*条件*/
							 MvcStateTransitionUpdate();break;		/*迁移*/
				//case 3:WalletWrite(1234);break;
				case 4:ActionExecutor(messageB);break;
				case 5:MvcActionExecutor(messageB);break;
				//case 5:SendCANmsg(0,1,a,0x18);break;
				case 6:iDM100ECommunication(messageB);break;
				//case 7:ModuleSettingTableUpdate(messageB);break;
				case 8:sysMonitor();break;
				//case 9:Mcp2515Receive();break;
				case 10:MFRC522Update();break;
				case 11:j =(j==0?1:0);LockSet(j);break;
				case 12:TestRtc();break;
				case 13:EmergencyStop();break;
				default:break;
			}
		}
		/*实时任务存放区 在这里可以主动放弃MCU的任务执行 其实是放在消息机制的间隙执行*/
		/*不可以有等外设的行为！*/
		AM2301Task();							/*温湿度传感器采集和处理任务*/
		ModbusTask();							/*显示器采集和处理任务*/
		//FormulaOnPgnGroup();			/*pgn表格中的逻辑策略 应该在发送之前检查*/
		//CanTask();								/*can通信第二层 处理协议传输和应用*/
		//CanTxTask();							/*can通信第一层 单纯发送*/
		//MCP2515Task();
		AVGTask();								/*对DMA方式的AD采样值进行平均化滑动处理*/
		//W5500Task();							/*网络接口通过SPI轮询进行处理*/
		LockTask();
		LedTask();
		//MegmeetTaskOn();
		//MegmeetTask();						/*485转can总线的接收任务*/	
	}
}




/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
