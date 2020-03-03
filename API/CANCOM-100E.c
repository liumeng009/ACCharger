/**
  ******************************************************************************
  * @file    CANCOM-100E.c 
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
#include "CANCOM-100E.h"
#include "string.h"
#include "usart.h"

/* Private typedef -----------------------------------------------------------*/
/*info ID1 ID2 ID3 ID4 data[0] data[1] data[2] data[3] data[4] data[5] data[6] data[7]*/
typedef struct						/*485的CAN结构*/
{
	uint8_t info;
	uint8_t ID1;
	uint8_t ID2;
	uint8_t ID3;
	uint8_t ID4;
	uint8_t data[8];				/*data[0]代表高字节 data[7]代表低字节*/
}CanTo485;

/* Private define ------------------------------------------------------------*/
#define EXT 0x01
#define STD	0x00
/* Private macro -------------------------------------------------------------*/
#define STATE_WAIT					0
#define STATE_SETTING_WAIT	1
#define STATE_SETTING_TX		2
#define STATE_SETTING_RX		3

/* Private variables ---------------------------------------------------------*/
CanTo485 canComTxFrame;
CanTo485 canComRxFrame;

static uint8_t mAppState = STATE_WAIT;    // State tracking variable

/* Private function prototypes -----------------------------------------------*/
void ModuleSettingMonitor(void);

/* Private functions ---------------------------------------------------------*/						 
/**
  * @brief  FrameTx()根据设置的帧信息，ID，数据，构成一帧 然后发送出去，但是并没
	*					有判断此时的485是否在working 这会导致一些问题
  * @param  
  * @retval 
  */
void FrameTx(	uint8_t FF,\
							uint8_t RTR,\
							uint8_t dataLength,\
							uint32_t ID,\
							uint8_t * pD)
{
		uint8_t i;			/*用来计算数据长度*/
		memset(&canComTxFrame,0,sizeof(canComTxFrame));	/*首先全部用0清除*/
	
		canComTxFrame.info = (FF << 7)|(RTR << 6)|(dataLength << 0);
		canComTxFrame.ID4 = ID;
		canComTxFrame.ID3 = ID>>8;
		canComTxFrame.ID2 = ID>>16;
		canComTxFrame.ID1 = ID>>24;
		for(i = 0; i< dataLength; i++)
		{
				canComTxFrame.data[i] = pD[i];
		}
		
		/*接下来就是发送了*/
		UsartWrite(3,(void *)&canComTxFrame,sizeof(canComTxFrame));
}

/**
  * @brief  MegmeetTask()是实时任务，主要读取Usart3的数据，然后立刻处理
  * @param  
  * @retval 
  */
void MegmeetTaskOn()
{
		static uint16_t moduleCounter;					/*模块计数器*/
		static uint8_t  CMDCounter;
		static uint8_t	cNum;										/*字节个数*/

		switch(mAppState)
		{
			case STATE_WAIT:
				moduleCounter = 0;												/*初始化计数器*/
				CMDCounter = 0;
				mAppState = STATE_SETTING_WAIT;
				vMegmeetTimerSetValue(0);
				break;
			case STATE_SETTING_WAIT:
				if(vMegmeetTimerGetValue() > 500)					/*延时了500ms*/
				{
					mAppState = STATE_SETTING_TX;						/*准备发送*/
				}
				break;
			case STATE_SETTING_TX:
			//	MegmeetTx(moduleCounter,2);
				mAppState = STATE_SETTING_RX;
				break;
			case STATE_SETTING_RX:
				cNum = UsartRead(3,(void *)&canComRxFrame);
				if(cNum > 0)
				{
						/*在这里已经接收到了 解析megmeet回复的帧 可以针对帧内容改变程序流向*/
					
						switch(canComRxFrame.data[0])
						{
							case 2:
								if(canComRxFrame.data[1] != 0x00)
								{
									mAppState = STATE_SETTING_WAIT;
								}
								else
								{
									usart1SendByte(0xee);
									mAppState = STATE_SETTING_WAIT;
								}
								break;
							default:break;
						}
						//....
						
						/*变更CMDCounter moduleCounter 为下一个功能做准备 但以上都是本功能*/
						moduleCounter++;
						if(moduleCounter >= 14)
							moduleCounter = 0;
				}
				else
				{
						moduleCounter++;
						if(moduleCounter >= 14)
							moduleCounter = 0;
						mAppState = STATE_SETTING_WAIT;
				}
				vMegmeetTimerSetValue(0);
				break;
			default:break;
		}
}

/**
  * @brief  MegmeetTask()是实时任务，主要读取Usart3的数据，然后立刻处理
	*					由于采用485方式 会出现发送的时候接收 接受的时候发送 就会错误
	*					因此，不仅要对CAN进行过滤 而且对发送的节奏控制好
	*					每200ms向一个模块轮流发送设置--读取--开关命令，然后下一个模块
	*					虽然CAN通信很可靠 但是也还是会有没有接收到帧的情况，而且协同
	*					多任务本身也必须有机制防止程序阻塞在某个位置，目前采用定时器方法
  * @param  
  * @retval 
  */
void MegmeetTask()
{
		static uint16_t moduleCounter;					/*模块计数器*/
		static uint8_t  CMDCounter;
		static uint16_t retryCounter;						/*重传次数*/
		static uint8_t	cNum;										/*字节个数*/

		switch(mAppState)
		{
			case STATE_WAIT:
				moduleCounter = 0;												/*初始化计数器*/
				CMDCounter = 0;
				retryCounter = 0;													/*重传次数*/
				mAppState = STATE_SETTING_WAIT;
				vMegmeetTimerSetValue(0);
				break;
			case STATE_SETTING_WAIT:
				retryCounter = 0;													/*重传次数*/
				//ModuleSettingMonitor();
				if(vMegmeetTimerGetValue() > 100)					/*延时了100ms*/
				{
					mAppState = STATE_SETTING_TX;						/*准备发送*/
				}
				break;
			case STATE_SETTING_TX:
				//MegmeetTx(moduleCounter, CMDCounter);
				vMegmeetTimerSetValue(0);
				mAppState = STATE_SETTING_RX;
				break;
			case STATE_SETTING_RX:
				cNum = UsartRead(3,(void *)&canComRxFrame);
				if(cNum > 0)
				{
						/*在这里已经接收到了 解析megmeet回复的帧 可以针对帧内容改变程序流向*/
						switch(canComRxFrame.data[0])
						{
							case 0:
								if(canComRxFrame.data[1] != 0x00)
								{
									mAppState = STATE_SETTING_WAIT;
									retryCounter = 0;
								}
								else
								{
									usart1SendByte(moduleCounter);
									usart1SendByte(CMDCounter);
									usart1SendByte(0xee);
									mAppState = STATE_SETTING_TX;
									retryCounter++;
									if(retryCounter == 3)				/*重传3次 还不行就忽略了*/
										mAppState = STATE_SETTING_WAIT;
								}
								break;
							case 1:
					//			moduleSetting[moduleCounter].status = (canComRxFrame.data[6]<< 8)|canComRxFrame.data[7];
					//			moduleSetting[moduleCounter].status &= 0xF800;	/*无关位清零*/
					//			moduleSetting[moduleCounter].statusVoltage = (canComRxFrame.data[2]<< 8)|canComRxFrame.data[3];
					//			moduleSetting[moduleCounter].statusCurrent = (canComRxFrame.data[4]<< 8)|canComRxFrame.data[5];
								break;
							case 2:
								if(canComRxFrame.data[1] != 0x00)
								{
									mAppState = STATE_SETTING_WAIT;
									retryCounter = 0;
								}
								else
								{
									usart1SendByte(moduleCounter);
									usart1SendByte(CMDCounter);
									usart1SendByte(0xee);
									mAppState = STATE_SETTING_TX;
									retryCounter++;
									if(retryCounter == 3)				/*重传3次 还不行就忽略了*/
										mAppState = STATE_SETTING_WAIT;
								}
								break;
							default:break;
						}
						//....
						
						/*变更CMDCounter moduleCounter 为下一个功能做准备 但以上都是本功能*/
						if(retryCounter == 0)	/*在没有重传的状态下 CMDCounter才能+1*/
							CMDCounter++;
						if(CMDCounter == 3)
						{
								CMDCounter = 0; /*归零*/
								moduleCounter++;
								if(moduleCounter == MODULENUM)
									moduleCounter = 0;
						}
				}
				else
				{
					if(vMegmeetTimerGetValue() > 50)
					{
						mAppState = STATE_SETTING_WAIT;
					}
				}
				break;
			default:break;
		}
}

/**
  * @brief  MegmeetTx按照megmeet的规定发送报文
  * @param  None
  * @retval 0：正确 1：错误
  */
void MegmeetTx(uint8_t moduleNum, uint8_t CMD, ModuleSetting setting)
{
		uint8_t data[8];
		switch(CMD)
		{
			case 0:
				data[0] = CMD;
				data[1] = 0;
				data[2] = setting.mAcurrent>>8;
				data[3] = setting.mAcurrent;
			
				data[4] = setting.mVvoltage>>24;
				data[5] = setting.mVvoltage>>16;
				data[6] = setting.mVvoltage>>8;
				data[7] = setting.mAcurrent;
				FrameTx(EXT,0,8, 0x1307c080|(ModuleAddr(moduleNum)&0x0f) ,data);
				break;
			case 1:
				data[0] = CMD;
				data[1] = 0;
				data[2] = 0;
				data[3] = 0;
				data[4] = 0;
				data[5] = 0;
				data[6] = 0;
				data[7] = 0;
				FrameTx(EXT,0,8, 0x1307c080|(ModuleAddr(moduleNum)&0x0f) ,data);
				break;
			case 2:
				data[0] = CMD;
				data[1] = 0;
				data[2] = 0;
				data[3] = 0;
				data[4] = 0;
				data[5] = 0;
				data[6] = 0;
				data[7] = setting.on;
				FrameTx(EXT,0,8, 0x1307c080|(ModuleAddr(moduleNum)&0x0f),data);
				break;
			default:break;
		}
}
/**
  * @brief  MegmeetRx根据不同CMD 识别回复
  * @param  None
  * @retval 0：正确 1：错误
  */		
void MegmeetRx(uint8_t moduleNum, uint8_t CMD)
{
	
}

