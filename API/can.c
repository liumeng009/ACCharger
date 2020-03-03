/**
  ******************************************************************************
  * @file    CAN/Networking/main.c 
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
#include "string.h"
#include "can.h"
#include "usart.h"
#include "pgn.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup CAN_Networking
  * @{
  */ 


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define __CAN1_USED__
/* #define __CAN2_USED__*/

#ifdef  __CAN1_USED__
  #define CANx                       CAN1
  #define GPIO_CAN                   GPIO_CAN1
  #define GPIO_Remapping_CAN         GPIO_Remapping_CAN1
  #define GPIO_CAN                   GPIO_CAN1
  #define GPIO_Pin_CAN_RX            GPIO_Pin_CAN1_RX
  #define GPIO_Pin_CAN_TX            GPIO_Pin_CAN1_TX
#else /*__CAN2_USED__*/ 
  #define CANx                       CAN2
  #define GPIO_CAN                   GPIO_CAN2
  #define GPIO_Remapping_CAN             GPIO_Remap_CAN2
  #define GPIO_CAN                   GPIO_CAN2
  #define GPIO_Pin_CAN_RX            GPIO_Pin_CAN2_RX
  #define GPIO_Pin_CAN_TX            GPIO_Pin_CAN2_TX
#endif  /* __CAN1_USED__ */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
CAN_InitTypeDef        CAN_InitStructure;
CAN_FilterInitTypeDef  CAN_FilterInitStructure;
CanTxMsg TxMessage;
pgnGroup pgnValue;			/*参数组表格 在gpn.c中已经定义extern型*/

struct canQueueTx canTxBuf;
struct canQueueRx canRxBuf;
struct message canMessageTx;	//can接收用的消息实例
struct message canMessageRx;	//can接收用的消息实例

/* Private function prototypes -----------------------------------------------*/
void NVIC_CAN_Config(void);
void CAN_Config(void);
void LED_Display(uint8_t Ledstatus);
void Init_RxMes(CanRxMsg *RxMessage);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  canInit program.
  * @param  None
  * @retval None
  */
int canInit(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* NVIC configuration */
  NVIC_CAN_Config();
   
  /* CAN configuration */
  CAN_Config();
  
  CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
	
	CanQueueInit();
 
  /* Transmit example
  TxMessage.StdId = 0x321;
  TxMessage.ExtId = 0x01;
  TxMessage.RTR = CAN_RTR_DATA;
  TxMessage.IDE = CAN_ID_STD;
  TxMessage.DLC = 1;
  TxMessage.Data[0] = 0xaa;				//写入数据
  CAN_Transmit(CANx, &TxMessage);		//发送过程！！！*/
}

/**
  * @brief  Configures the CAN.
  * @param  None
  * @retval None
  */
void CAN_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  /* GPIO clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
#ifdef  __CAN1_USED__
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CAN1, ENABLE);
#else /*__CAN2_USED__*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CAN1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CAN2, ENABLE);
#endif  /* __CAN1_USED__ */
  /* Configure CAN pin: RX */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CAN_RX;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	//上拉输入
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_CAN, &GPIO_InitStructure);
  
  /* Configure CAN pin: TX */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CAN_TX;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIO_CAN, &GPIO_InitStructure);
  
//  GPIO_PinRemapConfig(GPIO_Remapping_CAN , ENABLE);
  
  /* CANx Periph clock enable */
#ifdef  __CAN1_USED__
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
#else /*__CAN2_USED__*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
#endif  /* __CAN1_USED__ */
  
  
  /* CAN register init （全部初始化为0：复位初始化）*/
  CAN_DeInit(CANx);
  CAN_StructInit(&CAN_InitStructure);

  /* CAN cell init */
  CAN_InitStructure.CAN_TTCM = DISABLE;	//事件触发通信模式 关闭
  CAN_InitStructure.CAN_ABOM = ENABLE;	//自动离线管理     关闭
  CAN_InitStructure.CAN_AWUM = ENABLE; //自动唤醒模式	   关闭
  CAN_InitStructure.CAN_NART = DISABLE; //禁止报文自动重传 关闭
  CAN_InitStructure.CAN_RFLM = DISABLE; //接收FIFO锁定模式 关闭
  CAN_InitStructure.CAN_TXFP = DISABLE; //发送FIFO优先级   关闭
  CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;//正常工作模式
  
  /* CAN Baudrate = 1MBps*/
  CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;//重新同步跳跃宽度
  CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;//时间段1
  CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;//时间段2
  CAN_InitStructure.CAN_Prescaler = 4;	  //波特率分频器
  CAN_Init(CANx, &CAN_InitStructure);

  /* CAN filter init */
#ifdef  __CAN1_USED__
  CAN_FilterInitStructure.CAN_FilterNumber = 0;
#else /*__CAN2_USED__*/
  CAN_FilterInitStructure.CAN_FilterNumber = 14;
#endif  /* __CAN1_USED__ */
  CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
  CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
  CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
  CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
  CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
  CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
  CAN_FilterInit(&CAN_FilterInitStructure);
  
}

/**
  * @brief  Configures the NVIC for CAN.
  * @param  None
  * @retval None
  */
void NVIC_CAN_Config(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
#ifndef STM32F10X_CL
#ifdef  __CAN1_USED__
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
#else  /*__CAN2_USED__*/
  /* CAN2 is not implemented in the device */
   #error "CAN2 is implemented only in Connectivity line devices"
#endif /*__CAN1_USED__*/
#else
#ifdef  __CAN1_USED__ 
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
#else  /*__CAN2_USED__*/
  NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
#endif /*__CAN1_USED__*/

#endif /* STM32F10X_CL*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Initializes a Rx Message.  默认 CAN_ID_EXT
  * @param  CanRxMsg *RxMessage  
  * @retval None
  */
void Init_RxMes(CanRxMsg *RxMessage)
{
  uint8_t i = 0;

  RxMessage->StdId = 0x00;
  RxMessage->ExtId = 0x00;
  RxMessage->IDE = CAN_ID_EXT;
  RxMessage->DLC = 0;
  RxMessage->FMI = 0;
  for (i = 0;i < 8;i++)
  {
    RxMessage->Data[i] = 0x00;
  }
}

/**
  * @brief  CanTxTask只负责发送任务 接收在中断中进行 所以不用负责
						他是第一层 TX：此函数
											 RX：中断
  * @param  None
  * @retval None
  */
void CanTxTask()
{
		CanTxMsg txMessage;				/*can发送消息*/
		if(CanQueueTxDequeue(&txMessage) == 0)
		{
				/*在这里其实算是第一层*/
				CAN_Transmit(CANx, &txMessage);
				/*根据返回值可以判断是否可以发送出去*/
				//..
		}
}

#define STATE_WAIT					0
#define STATE_RECV					1
#define STATE_SEND					2
static uint8_t cAppState = STATE_WAIT;    // State tracking variable
/**
  * @brief  CanTask状态机实时运行 就是接收--发送--接收 不停采集FIFO数据（第二层）
	接收有两种情况：1，普通8字节数据 那就是普通接收了
									2，传输协议 此时接收变为一个过程，要管理连接，处理数据，时间管理
	在接受过程中改变两个主要的量：
									1，PGN参数表 这是最根本的目的
									2，linkstatus表 这个是为了管理连接用的
	发送只有一种情况：一般需要发送的时候都是为了传输协议的需要，即基于linkstatus表
										确定发送什么。注意，此处的发送只是为了传输协议
	那么就是说，本来这个任务就是第二层数据链路层的功能，接收和发送都是基于linkstatus
	表格进行的，在这一层，不论是长帧还是短帧，按理说都只是处理帧格式，而不设计具体内
	容。所以在接收中处理PGN参数表是不合适的。应该再加一层，在这一层主要处理参数内容。
	但目前先临时采用此不成熟结构。
	另外，在数据链路层中，你要对你接收到的帧做一个记录，这个记录主要是你接收到了什么
	类型的报文。以供外部使用。可以用一个队列，将所有接收到的报文记录一下，供状态机进
	行状态切换。
  * @param  None
  * @retval None
  */
void CanTask()
{
	static CanRxMsg rxMessage;				/*can接受消息*/
	static CanTxMsg txMessage;				/*can发送消息*/
	
	switch(cAppState)
	{
		case STATE_WAIT:								/*初始化*/
			Init_RxMes(&rxMessage);
			cAppState = STATE_RECV;
			break;
		case STATE_RECV:		/*这里的接收是抽象意义的接收 是根据canlink参数内容决定的*/
			if(CanQueueRxDequeue(&rxMessage) == 0)
			{
					/*在这里是第二层  解析数据 时间控制*/
					canRxHandle(rxMessage);	
			}
			cAppState = STATE_SEND;
			break;
		case STATE_SEND:		/*这里的发送是抽象意义的发送 是根据canlink参数内容决定的*/
			canTxHandle();
			cAppState = STATE_WAIT;
			break;
		defalult:break;
	}
}

/*协议传输表 此表记录了所有与协议传输相关的参数 所有操作都是基于此表进行的*/
struct linkStatus canLink;

/**
  * @brief  canRxHandle (第二层)处理普通数据和协议传输数据的内容，普通数据就是更新
		参数表，协议传输数据就是处理连接，然后更新参数表。同时还要通知状态机的
		条件检测系统，为状态迁移提供条件。
		有两种方法： 1.发消息通知
								 2.调用conditon的API
  * @param  CanRxMsg
  * @retval None
  */
void canRxHandle(CanRxMsg rxMessage)
{
		uint32_t pgn;
	
		/*处理普通情况 (本应是第三层内容)*/
		NormalMessageParse(rxMessage);
	
		/*处理协议传输情况*/
		pgn = (rxMessage.ExtId >> 16) & 0x000000ff;     /*从ID中提取pgn*/
		switch(pgn)
		{
			case 0xEC:											/*传输协议*/
				if(rxMessage.Data[0] == 0x10)	/*RTS*/
				{
						canLink.RTSReceived = 1;
						canLink.numOfByte = (rxMessage.Data[2]<<8) | rxMessage.Data[1];
						canLink.numOfPackage = rxMessage.Data[3];
						canLink.PGNCodeLow = rxMessage.Data[5];
						canLink.PGNCode = rxMessage.Data[6];
						canLink.PGNCodeHigh = rxMessage.Data[7];
						/*打开定时器*/
						vCanTimerSetAlarm(200);
				}
				break;
			case 0xE8:
				/*首先判断连接建立了没有 linkestablish 根据data[0]填入到数组 并且打开定时器*/
				if(canLink.linkEstablished == 1)
				{
						canLink.buffer[rxMessage.Data[0]].available = 1;
						memcpy(canLink.buffer[rxMessage.Data[0]].data,rxMessage.Data,8);
						/*打开定时器*/
						vCanTimerSetAlarm(200);
				}
				break;
			default:break;
		}
}
/**
  * @brief  canTxHandle处理linkstatus表格 实时运行 扫描几种状态 以if else形式判断
	显得有点乱 以后可以该进成表格方式表达
  * @param  None
  * @retval None
  */
void canTxHandle()
{
		uint8_t counter;		/*普通计数器*/
		CanTxMsg txMessage;
	
		/*在连接并未建立的情况下*/
		if(canLink.linkEstablished == 0)
		{
				if(canLink.RTSReceived == 1)
				{
						txMessage.ExtId = 0x1CECF455;
						txMessage.RTR = CAN_RTR_DATA;
						txMessage.IDE = CAN_ID_EXT;
						txMessage.DLC = 8;
						txMessage.Data[0] = 0x11;			/*控制字节*/
						txMessage.Data[1] = canLink.numOfPackage;			/*可发送数据包数*/
						txMessage.Data[2] = 0x01;			/*即将发送的编号*/
						txMessage.Data[3] = 0xff;
						txMessage.Data[4] = 0xff;
						txMessage.Data[5] = canLink.PGNCodeLow;
						txMessage.Data[6] = canLink.PGNCode;
						txMessage.Data[7] = canLink.PGNCodeHigh;
						
						/*发送CTS*/
						CanQueueTxEnqueue(txMessage);	/*投入队列准备发送*/
						canLink.CTSSended = 1;
					
						/*连接建立*/
						canLink.linkEstablished = 1;
						vCanTimerSetAlarm(200);				/*开始计时200ms 如果超时 连接中断*/
				}
		}
		else			/*链接已经建立的情况下*/
		{
				if(canLink.emSended == 1)
				{
						LinkClose();					/*关闭连接*/
				}
				else
				{
						for(counter = 0; counter < canLink.numOfPackage; counter++)
						{
								if(canLink.buffer[counter].available == 0)
										break;
						}
						/*扫描所有的数据是否收到*/
						/*没收到就退出 收到就发送em*/
						if(counter == canLink.numOfPackage)			/*全部收到*/
						{
								txMessage.ExtId = 0x1CECF455;
								txMessage.RTR = CAN_RTR_DATA;
								txMessage.IDE = CAN_ID_EXT;
								txMessage.DLC = 8;
								txMessage.Data[0] = 0x13;			/*控制字节*/
								txMessage.Data[1] = canLink.numOfByte;			/*可发送数据包数*/
								txMessage.Data[2] = canLink.numOfByte >> 8;			/*即将发送的编号*/
								txMessage.Data[3] = canLink.numOfPackage;
								txMessage.Data[4] = 0xff;
								txMessage.Data[5] = canLink.PGNCodeLow;
								txMessage.Data[6] = canLink.PGNCode;
								txMessage.Data[7] = canLink.PGNCodeHigh;
								CanQueueTxEnqueue(txMessage);	/*投入队列准备发送*/
								
								vCanTimerClose();						/*计时关闭*/
							
								canLink.emSended = 1;
							
								/*将数据采集到PGN表格中 (本应是第三层内容)*/
								//...
								LongMessageParse(&canLink);
						}
				}
		}
}
/**
  * @brief  LinkClose关闭连接 清空所有数据 
  * @param  None
  * @retval None
  */
void LinkClose()
{
		memset(&canLink,0,sizeof(canLink));		/*清零所有连接参数*/
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @brief  CanQueueInit初始化发送和接收队列 接收在中断中 发送全部在主程序 故这种
	队列没有互斥问题 可以随意访问
  * @param  None
  * @retval None
  */
void CanQueueInit()
{
		canTxBuf.front = 0;			/*发送缓冲 填入就会自动发送*/
		canTxBuf.rear = 0;
												
		canRxBuf.front = 0;			/*接收缓冲 存放已经接受的*/
		canRxBuf.rear = 0;
}
/**
  * @brief  只在中断中调用 只在中断中更改rear值
  * @param  None
  * @retval None
  */
uint8_t CanQueueRxEnqueue(CanRxMsg data)		
{
	uint8_t temp;
	if((((canRxBuf.rear) + 1)%CAN_BUFFER_SIZE) == canRxBuf.front)		//满了
	{
			return 1;
	}
	else
	{
			temp = canRxBuf.rear;
			canRxBuf.canBuffer[temp] = data;
			canRxBuf.rear = (temp + 1) % CAN_BUFFER_SIZE;
			return 0;
	}
}
/**
  * @brief  只在程序中调用 只在程序中中更改front值
  * @param  None
  * @retval None
  */
uint8_t CanQueueRxDequeue(CanRxMsg * data_pointer)
{
		uint8_t temp;
		if(canRxBuf.front == canRxBuf.rear)							//空了
		{
  			return 1;
		}
		else
		{
  			temp = canRxBuf.front;
				*data_pointer = canRxBuf.canBuffer[temp];
				canRxBuf.front = (temp + 1)%CAN_BUFFER_SIZE;
				return 0;
		}
}
/**
  * @brief  只在中断中调用 只在函数中更改rear值
  * @param  None
  * @retval None
  */
uint8_t CanQueueTxEnqueue(CanTxMsg data)		
{
	uint8_t temp;
	if((((canTxBuf.rear) + 1)%CAN_BUFFER_SIZE) == canTxBuf.front)		//满了
	{
			return 1;
	}
	else
	{
			temp = canTxBuf.rear;
			canTxBuf.canBuffer[temp] = data;
			canTxBuf.rear = (temp + 1) % CAN_BUFFER_SIZE;
			return 0;
	}
}
/**
  * @brief  只在程序中调用 只在程序中中更改front值
  * @param  None
  * @retval None
  */
uint8_t CanQueueTxDequeue(CanTxMsg * data_pointer)
{
		uint8_t temp;
		if(canTxBuf.front == canTxBuf.rear)							//空了
		{
  			return 1;
		}
		else
		{
  			temp = canTxBuf.front;
				*data_pointer = canTxBuf.canBuffer[temp];
				canTxBuf.front = (temp + 1)%CAN_BUFFER_SIZE;
				return 0;
		}
}
/**
  * @}
  */

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

CanRxMsg RxMessage;

/**
  * @brief  This function handles CAN1 Handler.
  * @param  None
  * @retval None
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
		//只要接收FIFO中有数据 就会不停进入此中断 没有给CPU留下了足够时间处理信息
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage); /*必须在中断中读取数据*/
		CanQueueRxEnqueue(RxMessage);				/*读取完之后压入队列 等待实时任务读取*/
}

/**
  * @brief  This function handles CAN2 Handler.
  * @param  None
  * @retval None
  */
#ifdef STM32F10X_CL
void CAN2_RX0_IRQHandler(void)
{
  CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
  if ((RxMessage.StdId == 0x321)&&(RxMessage.IDE == CAN_ID_STD) && (RxMessage.DLC == 1))
  {
  }
}
#endif


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
