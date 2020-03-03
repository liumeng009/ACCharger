/**
  ******************************************************************************
  * @file    USART/Interrupt/main.c 
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
#include "usart.h"
#include "string.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup USART_Interrupt
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define MAX_TX_BUFFER_SIZE 150	//发送缓冲区的实际值 如果超出会导致错误
#define MAX_RX_BUFFER_SIZE 150   //接受缓冲区的实际值 如果超出会导致错误
/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;

uint8_t usart1TxCounter = 0;
uint8_t usart2TxCounter = 0;
uint8_t usart3TxCounter = 0;
uint8_t usart4TxCounter = 0;

uint8_t usart1RxCounter = 0;
uint8_t usart2RxCounter = 0;
uint8_t usart3RxCounter = 0;
uint8_t usart4RxCounter = 0;


struct txBuffer
{
	uint8_t queue[MAX_TX_BUFFER_SIZE];	//申请一个区域存放数据
	uint8_t size;		//实际字节个数
	uint8_t ready;		//0：代表没有准备好 1：代表准备好了
							//中断根据此标志位才能决定是否发送
}usart1TxBuffer, usart2TxBuffer, usart3TxBuffer, usart4TxBuffer;

struct rxBuffer
{
	uint8_t queue[MAX_RX_BUFFER_SIZE];	//申请一个区域存放数据
	uint8_t size;		//实际字节个数
	uint8_t ready;		//0：代表没有准备好 1：代表准备好了
							//中断根据此标志位才能决定是否发送
}usart1RxBuffer, usart2RxBuffer, usart3RxBuffer, usart4RxBuffer;

/* Private function prototypes -----------------------------------------------*/
void RCC_Usart_Configuration(void);
void GPIO_Usart_Configuration(void);
void NVIC_Usart_Configuration(void);

void Delay(u32 i)
{
	for(;i>0;i--);
} 
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   usartInit program
  * @param  None
  * @retval None
  */
int usartInit()
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* System Clocks Configuration */
  RCC_Usart_Configuration();
       
  /* NVIC configuration */
  NVIC_Usart_Configuration();

  /* Configure the GPIO ports */
  GPIO_Usart_Configuration();

/* USARTy and USARTz configuration ------------------------------------------------------*/
  /* USARTy and USARTz configured as follow:
        - BaudRate = 9600 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
	
	 /* Configure USARTy (USART 1) */
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTy, &USART_InitStructure);
	
  /* Configure USARTz (USART 2)*/
	USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTz, &USART_InitStructure);
	/* Configure USARTa (USART 3)*/
	USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTa, &USART_InitStructure);
	/* Configure USARTb (USART 4)*/
	USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	//USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_Even;
	//USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USARTb, &USART_InitStructure);
  
  /* Enable USARTy Receive and Transmit interrupts */
  USART_ITConfig(USARTy, USART_IT_RXNE, ENABLE);
//  USART_ITConfig(USARTy, USART_IT_TXE, ENABLE);

  /* Enable USARTz Receive and Transmit interrupts */
  USART_ITConfig(USARTz, USART_IT_RXNE, ENABLE); //只要rx buffer不为空就中断
 // USART_ITConfig(USARTz, USART_IT_TXE, ENABLE);	//只要tx buffer为空就中断
 
   /* Enable USARTa Receive and Transmit interrupts */
  USART_ITConfig(USARTa, USART_IT_RXNE, ENABLE); //只要rx buffer不为空就中断
 // USART_ITConfig(USARTa, USART_IT_TXE, ENABLE);	//只要tx buffer为空就中断
 
   /* Enable USARTb Receive and Transmit interrupts */
  USART_ITConfig(USARTb, USART_IT_RXNE, ENABLE); //只要rx buffer不为空就中断
 // USART_ITConfig(USARTb, USART_IT_TXE, ENABLE);	//只要tx buffer为空就中断

  /* Enable the USARTy */
  USART_Cmd(USARTy, ENABLE);
  /* Enable the USARTz */
  USART_Cmd(USARTz, ENABLE);
	/* Enable the USARTa */
  USART_Cmd(USARTa, ENABLE);
	/* Enable the USARTb */
  USART_Cmd(USARTb, ENABLE);
	
	usart1TxBuffer.ready = 1;		/*代表刚开始的时候可以发送*/
	usart2TxBuffer.ready = 1;
	usart3TxBuffer.ready = 1;
	usart4TxBuffer.ready = 1;
	
	usart1RxBuffer.ready = 0;		/*代表刚开始的时候不能接收*/
	usart2RxBuffer.ready = 0;
	usart3RxBuffer.ready = 0;
	usart4RxBuffer.ready = 0;
  return 0;

}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_Usart_Configuration(void)
{   
  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(USARTy_GPIO_CLK | USARTz_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(USARTa_GPIO_CLK | USARTb_GPIO_CLK | USARTc_GPIO_TX_CLK | USARTc_GPIO_RX_CLK |RCC_APB2Periph_AFIO, ENABLE);

  /* Enable USARTy Clock */
  RCC_APB2PeriphClockCmd(USARTy_CLK, ENABLE); 
  /* Enable USARTz Clock */
  RCC_APB1PeriphClockCmd(USARTz_CLK, ENABLE);  
	/* Enable  usart3 4 5 Clock*/
	RCC_APB1PeriphClockCmd(USARTa_CLK, ENABLE); 
	RCC_APB1PeriphClockCmd(USARTb_CLK, ENABLE); 
	RCC_APB1PeriphClockCmd(USARTc_CLK, ENABLE); 
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void GPIO_Usart_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure USARTy Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = USARTy_RxPin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTy_GPIO, &GPIO_InitStructure);
  
  /* Configure USARTz Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = USARTz_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure);  
	
	/* Configure USARTa Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = USARTa_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTa_GPIO, &GPIO_InitStructure);  
	
	/* Configure USARTb Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = USARTb_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USARTb_GPIO, &GPIO_InitStructure);
  
  /* Configure USARTy Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTy_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USARTy_GPIO, &GPIO_InitStructure);

  /* Configure USARTz Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTz_TxPin;
  GPIO_Init(USARTz_GPIO, &GPIO_InitStructure);  
	
	/* Configure USARTa Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTa_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USARTa_GPIO, &GPIO_InitStructure);
	
	/* Configure USARTb Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = USARTb_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USARTb_GPIO, &GPIO_InitStructure);
		
	/*485 con configure */
#ifdef USART2_485
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOG,GPIO_Pin_9);	/*485 常态为接收*/
#endif

#ifdef USART3_485
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOG,GPIO_Pin_7);		/*485 常态为接收*/
#endif

#ifdef USART4_485
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOG,GPIO_Pin_7);		/*485 常态为接收*/
#endif

}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Usart_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTy_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTz_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTa_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the USARTz Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USARTb_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

}

/**
  * @brief  中断发送函数 为了不占用CPU资源 采用了中断发送 但调用周期必须要间隔！！
						在此函数中用了while等待，虽然时间不会长，但是也是可能的系统崩溃因素
						要么把此函数做成实时进程，要么运用消息机制不停检查
						如果做成实时进程，那么不太好实现针对性调用。
						如果做成消息，则会容易一些
  * @param  None
  * @retval None
  */
uint8_t UsartWrite(uint8_t UsartNum, uint8_t * pD , uint8_t size)
{
		switch(UsartNum)
		{
			case 1:	while(usart1TxBuffer.ready == 0){};
							memcpy(usart1TxBuffer.queue,pD,size);
							usart1TxBuffer.size = size;
							USART_ITConfig(USARTy, USART_IT_TXE, ENABLE);
							break;
			case 2:	while(usart2TxBuffer.ready == 0){};
							memcpy(usart2TxBuffer.queue,pD,size);
							usart2TxBuffer.size = size;
							USART_ITConfig(USARTz, USART_IT_TXE, ENABLE);
							break;
			case 3:	while(usart3TxBuffer.ready == 0){};
							memcpy(usart3TxBuffer.queue,pD,size);
							usart3TxBuffer.size = size;
							USART_ITConfig(USARTa, USART_IT_TXE, ENABLE);
							break;
			case 4:	while(usart4TxBuffer.ready == 0){};
							memcpy(usart4TxBuffer.queue,pD,size);
							usart4TxBuffer.size = size;
							USART_ITConfig(USARTb, USART_IT_TXE, ENABLE);
							break;
			default:break;
		}
		
		return 0;
}

/**
  * @brief  周期接收函数 定时进行扫描
  * @param  None
  * @retval None
  */
uint8_t UsartRead(uint8_t UsartNum, uint8_t * pD)
{
		uint8_t numOfByte;
		switch(UsartNum)
		{
			case 1:
						if(usart1RxBuffer.ready == 1)
						{
									memcpy(pD,usart1RxBuffer.queue,usart1RxBuffer.size);
									numOfByte = usart1RxBuffer.size;
									usart1RxBuffer.ready = 0;
									usart1RxBuffer.size = 0;
									return numOfByte;
						}
						else
									return 0;
			case 2:
						if(usart2RxBuffer.ready == 1)
						{
									memcpy(pD,usart2RxBuffer.queue,usart2RxBuffer.size);
									numOfByte = usart2RxBuffer.size;
									usart2RxBuffer.ready = 0;
									usart2RxBuffer.size = 0;
									return numOfByte;
						}
						else
									return 0;
			case 3:
						if(usart3RxBuffer.ready == 1)
						{
									memcpy(pD,usart3RxBuffer.queue,usart3RxBuffer.size);
									numOfByte = usart3RxBuffer.size;
									usart3RxBuffer.ready = 0;
									usart3RxBuffer.size = 0;
									return numOfByte;
						}
						else
									return 0;
			case 4:
						if(usart4RxBuffer.ready == 1)
						{
									memcpy(pD,usart4RxBuffer.queue,usart4RxBuffer.size);
									numOfByte = usart4RxBuffer.size;
									usart4RxBuffer.ready = 0;
									usart4RxBuffer.size = 0;
									return numOfByte;
						}
						else
									return 0;
			default:break;
		}
}
			
				
/**
  * @brief  函数处理发送请求 接受自处理 超时检测 
  * @param  None
  * @retval None
  */
void usartStateMachine()
{
	uint8_t linkEstablished = 0;
	//接收到时间消息 timeBase++
	
	//接收到发送消息
	USART_ITConfig(USARTy, USART_IT_TXE, ENABLE);
	linkEstablished = 1;
	
	/*接收到外部消息
	if(linkEstablished == 1)
		//计算时间 看看是不是超时了
	else
		//直接丢掉 无意义
		
	//时间处理
	if(linkEstablished == 1)
		//看看是不是超时了
	else
		//清零            */
}

void usart1SendByte(uint8_t byte)
{
	USART_ClearFlag(USARTy, USART_FLAG_TC);	//复位是1 因此要清零
	USART_SendData(USARTy, byte);
	while(USART_GetFlagStatus(USARTy,USART_FLAG_TC) != SET);
}

/**
  * @brief  modbus 收发处理函数
  * @param  none
  * @retval None
	* @detail 此函数每10ms执行一次 主要检测有没有收到数据 然后解析数据 返回数据
	* 操作的数据来自一张表格 之后在进行设计 其实这个函数应该放到高层来处理，但是
	* 在这里就不进行分层了
  */
void MbTransceiver(struct message data)
{
		/*首先检查接受缓存的ready是否为1 如果是则进行报文解析*/
		/*如果不是 则表示没有数据到来 则什么也不做*/
//			unsigned char* pv=(unsigned char*)&Display_VALUE_V;
//			unsigned char* pa=(unsigned char*)&Display_VALUE_A;
//			unsigned char* pw=(unsigned char*)&Display_VALUE_KW;
//			signed char* ptem=(signed char*)&temperature;
//			unsigned char* phum=(unsigned char*)&humidity;
	
			uint8_t rsp_info_crc_frame[29]={0};
			uint8_t rsp_info_frame[27]={0};
			uint16_t Crc;
			
		if(usart2RxBuffer.ready == 0)
			return;
		
		/*usart2RxBuffer*/
		if((usart2RxBuffer.queue[1]==0x02)&&(usart2RxBuffer.queue[6]==0xf9)&&(usart2RxBuffer.queue[7]==0xcb))
		{
//				USART2_Write(rsp_status_frame,6);
			 
				/*USART2 打开接收中断*/
				USART_ITConfig(USART2,USART_IT_RXNE ,ENABLE);
		}
		else if((usart2RxBuffer.queue[1]==0x04)&&(usart2RxBuffer.queue[6]==0x00)&&(usart2RxBuffer.queue[7]==0x0f))
		{
				/*---------------------给屏幕发送的帧-----------------------*/
				/*float转16进制*/	
		 
//				pv=(unsigned char*)&Display_VALUE_V;
//				rsp_info_frame[7]=pv[3];
//				rsp_info_frame[8]=pv[2];
//				rsp_info_frame[9]=pv[1];
//				rsp_info_frame[10]=pv[0];
				/*电压*/
				
//				pa=(unsigned char*)&Display_VALUE_A;
//				rsp_info_frame[11]=pa[3];
//				rsp_info_frame[12]=pa[2];
//				rsp_info_frame[13]=pa[1];
//				rsp_info_frame[14]=pa[0];
				/*电流*/
		
//				pw=(unsigned char*)&Display_VALUE_KW;
//				rsp_info_frame[15]=pw[3];
//				rsp_info_frame[16]=pw[2];
//				rsp_info_frame[17]=pw[1];
//				rsp_info_frame[18]=pw[0];
				/*电能*/
		
		
				/*温度*/
//				ptem=(signed char*)&temperature;
//				rsp_info_frame[19]=ptem[3];
//				rsp_info_frame[20]=ptem[2];
//				rsp_info_frame[21]=ptem[1];
//				rsp_info_frame[22]=ptem[0];
	
				/*湿度*/
//				phum=(unsigned char*)&humidity;
//				rsp_info_frame[23]=phum[3];
//				rsp_info_frame[24]=phum[2];
//				rsp_info_frame[25]=phum[1];
//				rsp_info_frame[26]=phum[0];
		
//				for(i=0;i<27;i++)
				{
//					rsp_info_crc_frame[i]=rsp_info_frame[i];
				}
//				Crc=Get_Crc16(rsp_info_frame,27);
		
				/*将16位的crc写入*/
				rsp_info_crc_frame[27]=(Crc&0xff00)>>8;
				rsp_info_crc_frame[28]=Crc&0xff;
				 
		
//				USART2_Write(rsp_info_crc_frame,29);
		}
		/*如果功能码是0x03，则为读取卡的金额*/
		else if((usart2RxBuffer.queue[1]==0x03)&&(usart2RxBuffer.queue[6]==0xf4)&&(usart2RxBuffer.queue[7]==0x08))
		{
		
//				for(i=0;i<7;i++)
				{
//						rsp_read_money_crc_frame[i]=rsp_read_money_frame[i];
				}
//				Crc=Get_Crc16(rsp_read_money_frame,7);
//				rsp_read_money_crc_frame[7]=(Crc&0xff00)>>8;
//				rsp_read_money_crc_frame[8]=Crc&0xff;
				
//				USART2_Write(rsp_read_money_crc_frame,9);
		}
		
		/*按下开始充电，打开继电器*/
		//依然是根据屏幕动作接收帧 回复相同帧 否则会一直发 通讯机制阻塞
		else if((usart2RxBuffer.queue[1]==0x05)&&(usart2RxBuffer.queue[4]==0xFF))
	  {
				/*原帧返回*/
	//			USART2_Write(on_rsp,8);

    }

		/*按下终止充电，关闭继电器*/
		else if((usart2RxBuffer.queue[1]==0x05)&&(usart2RxBuffer.queue[4]==0x00))
		{
				GPIO_SetBits(GPIOE,GPIO_Pin_4);	
		//		USART2_Write(off_rsp,8);
		}
		/*按下返回键或者按下返回主窗口*/
		else if((usart2RxBuffer.queue[1]==0x10)&&(usart2RxBuffer.queue[11]==0xB3)&&(usart2RxBuffer.queue[12]==0xef))
		{
		//		USART2_Write(mainwindow,8);
		}
}

void USART2Write(uint8_t *Data, uint8_t len)
{
		uint8_t counter;
		/*等待ready为1 只要发送中断打开 ready一定可以变1*/
		while(usart2TxBuffer.ready == 0);
		
		for(counter = 0; counter < len; counter++)
			//	usart2TxBuffer.queue[counter] = data[counter];
		
		usart2TxBuffer.ready = 1;		/*代表数据已经填好*/
		usart2TxBuffer.size = len;
	
		USART_ITConfig(USARTz, USART_IT_TXE, ENABLE);	/*打开中断 自动发送*/
}

void Usart2SetReady()
{
		usart2RxBuffer.ready = 1;
}
void Usart3SetReady()
{
		usart3RxBuffer.ready = 1;
}
void Usart4SetReady()
{
		usart4RxBuffer.ready = 1;
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

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles USARTy global interrupt request.
  * @param  None
  * @retval None
  */
void USARTy_IRQHandler(void)
{
		if(USART_GetITStatus(USARTy, USART_IT_RXNE) != RESET)
		{
				USART_ClearITPendingBit(USARTy, USART_IT_RXNE);
				
				if(usart1RxBuffer.size == 0)	/*如果缓冲中没有数据*/
				{
						usart1RxBuffer.ready = 0;	/*现在的缓冲处于操作状态*/
						//vUsart2TimerSetAlarm(2);	/*定时器开始计时*/
				}
				
				/* Read one byte from the receive data register */
				/*如果担心中断中清掉以前没来得及的数据 可以设计一个FIFO*/
				usart1RxBuffer.queue[usart1RxCounter++] = USART_ReceiveData(USARTy);
				
				usart1RxBuffer.size = usart1RxCounter;	/*更新size size代表个数*/
				
				//vUsart2TimerSetAlarm(2);	/*定时器开始计时*/
				
					/* Disable the USARTz Receive interrupt */
				//USART_ITConfig(USARTz, USART_IT_RXNE, DISABLE);

		}
		
		if(USART_GetITStatus(USARTy, USART_IT_TXE) != RESET)
		{   
#ifdef USART1_485
			//GPIO_SetBits(GPIOG,GPIO_Pin_9);
#endif
			usart1TxBuffer.ready = 0;
			if(usart1TxCounter == usart1TxBuffer.size) /*此次并没有数据要发 只是为了关中断*/
			{	
					/* Disable the USARTy Transmit interrupt */
					USART_ClearITPendingBit(USARTy, USART_IT_TXE);
					USART_ITConfig(USARTy, USART_IT_TXE, DISABLE);
				
#ifdef USART1_485
				//	GPIO_ResetBits(GPIOG,GPIO_Pin_9);
#endif		
					usart1TxBuffer.ready = 1;
					usart1TxCounter = 0;		//清零便于下一次计数
			}
			else
				/* Write one byte to the transmit data register 写入不等待 */
				USART_SendData(USARTy, usart1TxBuffer.queue[usart1TxCounter++]);
		}
}

/**
  * @brief  This function handles USARTz global interrupt request.
  * @param  None
  * @retval None
  */
void USARTz_IRQHandler(void)
{
		if(USART_GetITStatus(USARTz, USART_IT_RXNE) != RESET)
		{
				USART_ClearITPendingBit(USARTz, USART_IT_RXNE);
				
				if(usart2RxBuffer.size == 0)	/*如果缓冲中没有数据*/
				{
						usart2RxCounter = 0;			/*清零计数器*/
						usart2RxBuffer.ready = 0;	/*现在的缓冲处于操作状态*/
						vUsart2TimerSetAlarm(3);	/*定时器开始计时*/
				}
				
				/* Read one byte from the receive data register */
				/*如果担心中断中清掉以前没来得及的数据 可以设计一个FIFO*/
				usart2RxBuffer.queue[usart2RxCounter++] = USART_ReceiveData(USARTz);
				
				usart2RxBuffer.size = usart2RxCounter;	/*更新size size代表个数*/
				
				vUsart2TimerSetAlarm(3);	/*定时器开始计时*/
		}
		
		if(USART_GetITStatus(USARTz, USART_IT_TXE) != RESET)
		{   
			USART_ClearITPendingBit(USARTz, USART_IT_TXE);
#ifdef USART2_485
			GPIO_SetBits(GPIOG,GPIO_Pin_9);
#endif
			usart2TxBuffer.ready = 0;
			
			if(usart2TxCounter == usart2TxBuffer.size) /*此次并没有数据要发 只是为了关中断*/
			{	
					/* Disable the USARTz Transmit interrupt */
					USART_ITConfig(USARTz, USART_IT_TXE, DISABLE);
					USART_ClearITPendingBit(USARTz, USART_IT_TXE);
				
          USART_ITConfig(USARTz, USART_IT_TC, ENABLE);	/*开启发送完成中断*/
				
					usart2TxCounter = 0;		//清零便于下一次计数
					usart2TxBuffer.ready = 1;	/*缓存确实已经可以操作了 但实际还有一个字节在发送中*/
			}
			else
				/* Write one byte to the transmit data register 写入不等待 */
			{
					USART_SendData(USARTz, usart2TxBuffer.queue[usart2TxCounter++]);
			}
		}
		
		if (USART_GetITStatus(USARTz, USART_IT_TC) != RESET) 
		{
				USART_ClearITPendingBit(USARTz, USART_IT_TC);
				USART_ITConfig(USARTz, USART_IT_TC, DISABLE);
#ifdef USART2_485
				GPIO_ResetBits(GPIOG,GPIO_Pin_9);
#endif
		}
}

/**
  * @brief  This function handles USARTa global interrupt request.
  * @param  None
  * @retval None
  */
void USARTa_IRQHandler(void)
{
		if(USART_GetITStatus(USARTa, USART_IT_RXNE) != RESET)
		{
				USART_ClearITPendingBit(USARTa, USART_IT_RXNE);

				if(usart3RxBuffer.size == 0)	/*如果缓冲中没有数据*/
				{
						usart3RxCounter = 0;			/*清零计数器*/
						usart3RxBuffer.ready = 0;	/*现在的缓冲处于操作状态*/
						vUsart3TimerSetAlarm(3);	/*定时器开始计时*/
				}
				
				/* Read one byte from the receive data register */
				/*如果担心中断中清掉以前没来得及的数据 可以设计一个FIFO*/
				usart3RxBuffer.queue[usart3RxCounter++] = USART_ReceiveData(USARTa);

				usart3RxBuffer.size = usart3RxCounter;	/*更新size size代表个数*/
				
				vUsart3TimerSetAlarm(3);	/*定时器开始计时*/
		}
		
		if(USART_GetITStatus(USARTa, USART_IT_TXE) != RESET)
		{   
			USART_ClearITPendingBit(USARTa, USART_IT_TXE);
#ifdef USART3_485
			GPIO_SetBits(GPIOG,GPIO_Pin_8);
#endif
			usart3TxBuffer.ready = 0;
			
			if(usart3TxCounter == usart3TxBuffer.size) /*此次并没有数据要发 只是为了关中断*/
			{	
					/* Disable the USARTy Transmit interrupt */
					USART_ITConfig(USARTa, USART_IT_TXE, DISABLE);
					USART_ClearITPendingBit(USARTa, USART_IT_TXE);
				
          USART_ITConfig(USARTa, USART_IT_TC, ENABLE);	/*开启发送完成中断*/
				
					usart3TxCounter = 0;		//清零便于下一次计数
					usart3TxBuffer.ready = 1;	/*缓存确实已经可以操作了 但实际还有一个字节在发送中*/
			}
			else
				/* Write one byte to the transmit data register 写入不等待 */
			{
					USART_SendData(USARTa, usart3TxBuffer.queue[usart3TxCounter++]);
			}
		}
		
		if (USART_GetITStatus(USARTa, USART_IT_TC) != RESET) 
		{
				USART_ClearITPendingBit(USARTa, USART_IT_TC);
				USART_ITConfig(USARTa, USART_IT_TC, DISABLE);
#ifdef USART3_485
				GPIO_ResetBits(GPIOG,GPIO_Pin_8);
#endif
		}
}

/**
  * @brief  This function handles USARTb global interrupt request.
  * @param  None
  * @retval None
  */
void USARTb_IRQHandler(void)
{
    if(USART_GetITStatus(USARTb, USART_IT_RXNE) != RESET)
		{
				USART_ClearITPendingBit(USARTb, USART_IT_RXNE);
				//usart1SendByte(0xaa);
				if(usart4RxBuffer.size == 0)	/*如果缓冲中没有数据*/
				{
						usart4RxCounter = 0;			/*清零计数器*/
						usart4RxBuffer.ready = 0;	/*现在的缓冲处于操作状态*/
						vUsart4TimerSetAlarm(4);	/*定时器开始计时*/
				}
				
				/* Read one byte from the receive data register */
				/*如果担心中断中清掉以前没来得及的数据 可以设计一个FIFO*/
				usart4RxBuffer.queue[usart4RxCounter++] = USART_ReceiveData(USARTb);

				usart4RxBuffer.size = usart4RxCounter;	/*更新size size代表个数*/
				
				vUsart4TimerSetAlarm(4);	/*定时器开始计时*/
		}
		
		if(USART_GetITStatus(USARTb, USART_IT_TXE) != RESET)
		{   
			USART_ClearITPendingBit(USARTb, USART_IT_TXE);
#ifdef USART4_485
			GPIO_SetBits(GPIOG,GPIO_Pin_7);
#endif
			usart4TxBuffer.ready = 0;
			
			if(usart4TxCounter == usart4TxBuffer.size) /*此次并没有数据要发 只是为了关中断*/
			{	
					/* Disable the USARTy Transmit interrupt */
					USART_ITConfig(USARTb, USART_IT_TXE, DISABLE);
					USART_ClearITPendingBit(USARTb, USART_IT_TXE);
				
          USART_ITConfig(USARTb, USART_IT_TC, ENABLE);	/*开启发送完成中断*/
				
					usart4TxCounter = 0;		//清零便于下一次计数
					usart4TxBuffer.ready = 1;	/*缓存确实已经可以操作了 但实际还有一个字节在发送中*/
			}
			else
			/* Write one byte to the transmit data register 写入不等待 */
			{
					USART_SendData(USARTb, usart4TxBuffer.queue[usart4TxCounter++]);
			}
		}
		
		if (USART_GetITStatus(USARTb, USART_IT_TC) != RESET) 
		{
				USART_ClearITPendingBit(USARTb, USART_IT_TC);
				USART_ITConfig(USARTb, USART_IT_TC, DISABLE);
#ifdef USART4_485
				GPIO_ResetBits(GPIOG,GPIO_Pin_7);
#endif
		}
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
