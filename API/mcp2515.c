/**
  ******************************************************************************
  * @file    mcp2515.c 
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
#include "message.h"
#include "usart.h"
#include "mcp2515.h"
#include "spi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  
  * @param  
  * @retval 
  */
void Mcp2515Init(void)
{
		uint8_t temp[4] = {0,0,0,0};
		
		/*GPIO init:
						 CS:	GPIOG 14 output
						INT:	GPIOE 5  input
		*/
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
		GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOE,&GPIO_InitStructure);

		
		/* MCP2515 软件复位 */
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz,SPI_RESET);
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
	 
		/*适当延时 初始化不用介意*/
		Delay(0xffffff);
		
		//位指令MCP2515 进入配置模式
		//CANCTRL的REQOP[2:0]改为100
		Mcp2515BitModify(CANCTRL, 0xE0, (1<<REQOP2));
		Delay(65535);
	 	
	 //计算并设置MCP2515的位时间
	 
	 //  时钟频率   Fosc  = 16MHz
	 //  分频控制器 CNF1.BRP[5:0] = 7
	 //  最小时间份额 TQ = 2 * ( BRP + 1 ) / Fosc   = 2*(7+1)/16M = 1uS
	 //  同步段 Sync Seg   = 1TQ
	 //  传播段 Prop Seg   = ( PRSEG + 1 ) * TQ  = 1 TQ
	 //  相位缓冲段 Phase Seg1 = ( PHSEG1 + 1 ) * TQ = 3 TQ
	 //  相位缓冲段 Phase Seg2 = ( PHSEG2 + 1 ) * TQ = 3 TQ
	 //  同步跳转长度设置为 CNF1.SJW[1:0] = 00,  1TQ
	 //  总线波特率 NBR = Fbit =  1/(sync seg + Prop seg + PS1 + PS2 )
	 //                       = 1/(8TQ) = 1/8uS = 125kHz
	 
	 //???????CNF1.BRP[5:0] = 7,????????? CNF1.SJW[1:0] = 00
	 Mcp2515WriteByte( CNF1, (1<<BRP0)|(1<<BRP1)|(1<<BRP2) );
	 // ????? Prop Seg ?00,?1TQ,????? Phase Seg1???3TQ
	 Mcp2515WriteByte( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	 // ?? ????? Phase Seg2? 3TQ , ???????
	 Mcp2515WriteByte( CNF3, (1<<PHSEG21) );
	 
	 
	 
	 //设置为 500kbps ,TQ = 1/8us
	 //CNF1.BRP[5:0] = 0,????????? CNF1.SJW[1:0] = 01
	// Mcp2515WriteByte( CNF1, (1<<BRP0)|(1<<SJW0) );    // 500kbps
	 //Mcp2515WriteByte( CNF1, (1<<SJW0) );               //1Mbps
		// Prop Seg 00,1TQ, Phase Seg1 3TQ
	 //Mcp2515WriteByte( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	 // Phase Seg2  3TQ , 
	 //Mcp2515WriteByte( CNF3, (1<<PHSEG21) );
	 
	 
	 //MCP2515
	// Mcp2515WriteByte( CANINTE, /*(1<<RX1IE)|(1<<RX0IE)*/ 0 );
	 
	 //设置中断控制器 使能MCP2515接收缓冲器中断
	 Mcp2515WriteByte( CANINTE, (1<<RX1IE)|(1<<RX0IE) );
	 
	 
	 // 关闭接收缓冲器0的滤波功能 RXM[1:0]=11, 接收所有报文
	 Mcp2515WriteByte( RXB0CTRL, (1<<RXM1)|(1<<RXM0) );
	 
	 // 关闭接收缓冲器1的滤波功能 RXM[1:0]=11 接收所有报文
	 Mcp2515WriteByte( RXB1CTRL, (1<<RXM1)|(1<<RXM0) );
	 
	 //设置6个验收滤波寄存器为0
	 Mcp2515WriteArray( RXF0SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF1SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF2SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF3SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF4SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF5SIDH, temp, 4 );
	 
	 //设置2个验收滤波寄存器为0
	 Mcp2515WriteArray( RXM0SIDH, temp, 4 );
	 Mcp2515WriteArray( RXM1SIDH, temp, 4 );
	 
	 //设置接收相关引脚配置寄存器 禁止第二功能
	 Mcp2515WriteByte(BFPCTRL, 0);
	 
	 //调试使用 设置BFPCTRL的RX0BF,RX1BF为数字输出
	 Mcp2515BitModify( BFPCTRL, (1<<B1BFE)|(1<<B0BFE)|(1<<B1BFM)|(1<<B0BFM), (1<<B1BFE)|(1<<B0BFE) );
	 
	 
	 //设置发送相关引脚配置寄存器 禁止第二功能
	 Mcp2515WriteByte(TXRTSCTRL, 0);
	 
	 
	 //MCP2515环回模式 进入功能测试
	 //Mcp2515BitModify( CANCTRL, 0XE0, (1<<REQOP1) );
	 
	 //MCP2515进入正常模式
	  Mcp2515BitModify(CANCTRL, 0xE0, 0);
		Delay(65535);
		
		Mcp2515WriteByte(CANINTF,0);
}


/**
  * @brief  Mcp2515WriteByte写控制寄存器
  * @param  
  * @retval 
  */
void Mcp2515WriteByte(uint8_t address, uint8_t data)
{
	// CS low ,MCP2515 enable
	GPIO_ResetBits(GPIOG,GPIO_Pin_14);
 
	SPIByte(SPIz, SPI_WRITE);
	SPIByte(SPIz, address);
	SPIByte(SPIz, data);
 
	//CS high ,MCP2515 disable
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
  * @brief  Mcp2515ReadByte读控制寄存器
  * @param  
  * @retval 
  */
uint8_t Mcp2515ReadByte(uint8_t address)
{
	uint8_t data;
 
	// CS low ,MCP2515 enable
	GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	
	SPIByte(SPIz, SPI_READ); 
	SPIByte(SPIz, address); 
	data = SPIByte(SPIz, 0xff); 
 
	//CS high ,MCP2515 disable
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
 
	return data;
}

/**
  * @brief  Mcp2515ReadRxBuffer读接收缓冲
  * @param  
  * @retval 
  */
uint8_t Mcp2515ReadRxBuffer(uint8_t address)
{
		uint8_t data;
	
		if (address & 0xF9)
			return 0;
 
		// CS low 
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
 
		SPIByte(SPIz, SPI_READ_RX | address); 
		data = SPIByte(SPIz, 0xff);
 
		//CS high 
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
 
		return data;
}

/**
  * @brief  Mcp2515BitModify位修改
  * @param  
  * @retval 
  */
void Mcp2515BitModify(uint8_t address, uint8_t mask, uint8_t data)
{
	// CS low ,MCP2515 enable
	GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	
	SPIByte(SPIz, SPI_BIT_MODIFY); 
	SPIByte(SPIz, address);    
	SPIByte(SPIz, mask);     
	SPIByte(SPIz, data);    
 
	//CS high ,MCP2515 disable
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
  * @brief  Mcp2515WriteArray 对连续寄存器进行连续写操作
  * @param  
  * @retval 
  */
void Mcp2515WriteArray( uint8_t address, uint8_t *data, uint8_t length )
{
		uint8_t i;
 
		// CS low 
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	 
		SPIByte(SPIz, SPI_WRITE);
		SPIByte(SPIz, address);  
		for (i=0; i<length ;i++ )
			SPIByte(SPIz, *data++);  
	 
		//CS high
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
  * @brief  Mcp2515ReadArray 对连续寄存器进行连续读操作
  * @param  
  * @retval 
  */
void Mcp2515ReadArray( uint8_t address, uint8_t *data, uint8_t length )
{
	uint8_t i;
 
	// CS low ,MCP2515 enable
	GPIO_ResetBits(GPIOG,GPIO_Pin_14);
 
	SPIByte(SPIz, SPI_READ); 
	SPIByte(SPIz, address);  
	for (i=0; i<length ;i++ )
		*data++ = SPIByte(SPIz, 0xff);  
 
	//CS high ,MCP2515 disable
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
  * @brief  SendCANmsg 发送can报文 这里比较简单 只是发送 不涉及缓冲 意义判断
  * @param  
  * @retval 
  */
void sendCANmsg()
{
		Mcp2515BitModify(TXB0CTRL, 0x08, 0x00);
	
		Mcp2515WriteByte(TXB0SIDH,0x01);
		Mcp2515WriteByte(TXB0SIDL,0xe0);
	
		Mcp2515WriteByte(TXB0DLC, 8);
	
		Mcp2515WriteByte(TXB0D0, 1);
		Mcp2515WriteByte(TXB0D1, 2);
		Mcp2515WriteByte(TXB0D2, 3);
		Mcp2515WriteByte(TXB0D3, 4);
		Mcp2515WriteByte(TXB0D4, 5);
		Mcp2515WriteByte(TXB0D5, 6);
		Mcp2515WriteByte(TXB0D6, 7);
		Mcp2515WriteByte(TXB0D7, 8);
	
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz, SPI_READ);
		SPIByte(SPIz, 0x30);
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		usart1SendByte(SPIByte(SPIz,0x00));
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
	
		// CS low ,MCP2515 enable
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	
		SPIByte(SPIz,SPI_RTS|0x01);
		
		//CS high ,MCP2515 disable
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/** Send a CAN message
* \param bi transmit buffer index 0, 1 ,2
* \param id message identifier
* \param data pointer to data to be stored
* \param prop message properties, the octet has following structure:
* - bits 7:6 - message priority (higher the better)
* - bit 5 - if set, message is remote request (RTR)
* - bit 4 - if set, message is considered to have ext. id.
* - bits 3:0 - message length (0 to 8 bytes) */
/* EID17 ----- EID0 共18位  SID10 ------SID0 共11位  18+11=29 */
void SendCANmsg(uint8_t bi,
								unsigned long id,
								uint8_t * data,
								uint8_t prop)
{
		uint8_t i;
		uint32_t SID;
		uint32_t EID;
		
		/*默认情况下 id的排列是 SID + EID 高位在高位*/
		/*此函数中   id的排列是 EID + SID 高位在高位*/
		SID = id >> 18;
		EID = id & 0x03ffff;
		id = (EID << 11)|SID;
	
		/* Initialize reading of the receive buffer */
		Mcp2515BitModify(TXBnCTRL(bi), 0x08, 0x00);
	
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		/* Send header and address */
		SPIByte(SPIz, SPI_WRITE);
		SPIByte(SPIz, TXBnCTRL(bi));
	
		/* Setup message priority */
		SPIByte(SPIz,prop >> 6);		/*只用67位*/
	
		/* Setup standard or extended identifier */
		/* std 先发 然后 ext再发 优先发送的不一样*/
		/* EID17 ----- EID0 共18位  SID10 ------SID0 共11位  18+11=29 */
		if(prop & 0x10)				/*扩展帧*/
		{
				SPIByte(SPIz,(uint8_t)(id>>3));
				SPIByte(SPIz,(uint8_t)(id<<5)|(1<<EXIDE)|(uint8_t)(id>>27));
				SPIByte(SPIz,(uint8_t)(id>>19));
				SPIByte(SPIz,(uint8_t)(id>>11));
		} 
		else 								 /*标准帧*/	
		{
				SPIByte(SPIz,(uint8_t)(id>>3));
				SPIByte(SPIz,(uint8_t)(id<<5)|(0<<EXIDE)|(uint8_t)(id>>27));
				SPIByte(SPIz,(uint8_t)(id>>19));
				SPIByte(SPIz,(uint8_t)(id>>11));
		}
		
		/* Setup message length and RTR bit */
		SPIByte(SPIz,(prop & 0x0F) | ((prop & 0x20) ? (1 << RTR) : 0));
		
		/* Store the message into the buffer */
		for(i = 0; i < (prop & 0x0F); i++)
			SPIByte(SPIz,data[i]);
		
		/* Release the bus */
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
		
		/* Send request to send */
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz, SPI_READ);
		SPIByte(SPIz, 0x30);
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
		
		// CS low ,MCP2515 enable
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz,SPI_RTS| (1 << bi));
		//CS high ,MCP2515 disable
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
}

/**
* Example code - simple CAN listener.
*
*/
#define getData(n) msgReceived[6+i];
#define getId (unsigned short)((msgReceived[1]<<3)|(msgReceived[2]>>5));
#define getLength msgReceived[5] >> 4;
#define setRollover(v) Mcp2515BitModify(RXB0CTRL, 1 << BUKT, v << BUKT);
#define getMode (Mcp2515ReadByte(CANSTAT) >> 5);
#define setMode(mode) { Mcp2515BitModify(CANCTRL, (7 << REQOP0), \
									(mode << REQOP0)); while(getMode != mode); }

unsigned char * msgReceived = (void *)0;
									
						
uint8_t rbuffer[2][14]; /* 2 RX buffers, each have 14B */
uint8_t ready[2];				/* 标记数据接受情况*/
									
/**
  * @brief  获得接收状态 看是1rx 还是 2rx
  * @param  
  * @retval 
  */						
uint8_t getRXState()
{
		uint8_t rxStatus;
		// CS low ,MCP2515 enable
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz,SPI_RX_STATUS);
		rxStatus = SPIByte(SPIz,0x00);
		//CS high ,MCP2515 disable
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
		return rxStatus;
}

/* get receive buffer index (we don't consider that both buffer
contain message, this situation in our environment cannot happen �
message is directly copied from the buffer and released in IRQ)*/
void Mcp2515Receive(void)
{
		uint8_t i;
		uint8_t bi = getRXState() >> 6;
	
		/* Copy the message from the device and release buffer */
	
		if(GPIO_ReadOutputDataBit(GPIOE,5) == SET)
			return;
				
		//usart1SendByte(Mcp2515ReadByte(CANINTF));
	
		// CS low ,MCP2515 enable
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
	
		SPIByte(SPIz,SPI_READ_RX);
		SPIByte(SPIz,RXBnCTRL(bi));
		/* Make the local copy */
		for(i = 0; i < 14; i++)
				rbuffer[bi][i] = SPIByte(SPIz,0x00);
	
		//CS high ,MCP2515 disable
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
	
		msgReceived = rbuffer[bi];
		
		//usart1SendByte(Mcp2515ReadByte(CANINTF));
		
		for(i = 0; i < 14; i++)
				usart1SendByte(rbuffer[bi][i]);
}

/**
  * @brief  MCP2515Task主要接收mcp2515收到的数据,
  * @param  
  * @retval 
  */
#define STATE_WAIT					0
#define STATE_INT_LOW				1
#define STATE_INT_HIGH			2

static uint8_t mcpAppState = STATE_WAIT;    // State tracking variable

void MCP2515Task(void)
{
		static uint8_t i;
		static uint8_t bi;

    switch(mcpAppState)
    {
				case STATE_WAIT:
						bi = 0;
						mcpAppState = STATE_INT_HIGH;
						break;
				
				case STATE_INT_HIGH:
						if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5) == RESET)	/*有数据:INT低电平*/
						{		
								mcpAppState = STATE_INT_LOW;
						}
						break;
		
        case STATE_INT_LOW:	
						/*首先判断是哪个缓存01 10 11三种情况*/
						bi = getRXState() >> 6;
		
						if(bi == 0x00)
						{
								mcpAppState = STATE_WAIT;
								break;
						}
						else if(bi == 0x01 || bi == 0x10)
						{
								/* Copy the message from the device and release buffer */
								/* CS low ,MCP2515 enable */
								GPIO_ResetBits(GPIOG,GPIO_Pin_14);
			
								/*使用SPI_READ_RX 可以自动清除相关标志位*/
								SPIByte(SPIz,SPI_READ_RX | ((bi>>1)<<2));

								/* Make the local copy */
								for(i = 0; i < 14; i++)
								{
										/*在接受到的同时 用一个标记进行记录 表明状态*/
										/*一个缓冲区只有两种状态 接收到了 没有接受到*/
										rbuffer[(bi>>1)][i] = SPIByte(SPIz,0x00);
								}
								ready[(bi>>1)] = 1;		/*已经接收到了*/
			
								/*CS high ,MCP2515 disable*/
								GPIO_SetBits(GPIOG,GPIO_Pin_14);
								
								/*采用SPI_READ_RX命令就不用Mcp2515WriteByte(CANINTF,0);了*/
						
								for(i = 0; i < 14; i++)
									usart1SendByte(rbuffer[(bi>>1)][i]);
						}
						else
						{
								/* Copy the message from the device and release buffer */
								/* CS low ,MCP2515 enable */
								GPIO_ResetBits(GPIOG,GPIO_Pin_14);
			
								/*使用SPI_READ_RX 可以自动清除相关标志位*/
								SPIByte(SPIz,SPI_READ_RX | (0<<2));

								/* Make the local copy */
								for(i = 0; i < 14; i++)
								{
										/*在接受到的同时 用一个标记进行记录 表明状态*/
										/*一个缓冲区只有两种状态 接收到了 没有接受到*/
										rbuffer[0][i] = SPIByte(SPIz,0x00);
								}
								ready[0] = 1;		/*已经接收到了*/
			
								/*CS high ,MCP2515 disable*/
								GPIO_SetBits(GPIOG,GPIO_Pin_14);
								
								/*采用SPI_READ_RX命令就不用Mcp2515WriteByte(CANINTF,0);了*/
						
								for(i = 0; i < 14; i++)
									usart1SendByte(rbuffer[0][i]);
								
								/* Copy the message from the device and release buffer */
								/* CS low ,MCP2515 enable */
								GPIO_ResetBits(GPIOG,GPIO_Pin_14);
			
								/*使用SPI_READ_RX 可以自动清除相关标志位*/
								SPIByte(SPIz,SPI_READ_RX | (1<<2));

								/* Make the local copy */
								for(i = 0; i < 14; i++)
								{
										/*在接受到的同时 用一个标记进行记录 表明状态*/
										/*一个缓冲区只有两种状态 接收到了 没有接受到*/
										rbuffer[1][i] = SPIByte(SPIz,0x00);
								}
								ready[1] = 1;		/*已经接收到了*/
			
								/*CS high ,MCP2515 disable*/
								GPIO_SetBits(GPIOG,GPIO_Pin_14);
								
								/*采用SPI_READ_RX命令就不用Mcp2515WriteByte(CANINTF,0);了*/
						
								for(i = 0; i < 14; i++)
									usart1SendByte(rbuffer[1][i]);
						}
				
						mcpAppState = STATE_WAIT;
						break;
				default:break;
		}
}

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

/**
  * @brief  MCP2515Read()读取接受到的数据 先判断接受到没有 再提取数据
  * @param  
  * @retval 
  */
uint8_t MCP2515Read(CanTo485 * frame)
{
		if(ready[0] == 1)
		{
				
		}
		else if(ready[1] == 1)
		{
				
		}
}