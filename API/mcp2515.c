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
  * ´ËÎÄ¼þÃèÊöÖÜÁ¢¹¦CANCOM_100EµÄ¸ñÊ½×ª»»Ä£Ê½µÄÓÃ·¨£¬Ò²¾ÍÊÇ·¢ËÍ¹æ¶¨ÐÎÊ½µÄ485Êý¾Ý
  *	Ö¡£¬ºÍ½ÓÊÕ¹æ¶¨ÐÎÊ½µÄ485Êý¾ÝÖ¡£¬ÒÔ¼°·¢ËÍÖ®Ç°ÈçºÎÍ¨¹ý½á¹¹Ìå¹¹³ÉÖ¡£¬ºÍ½ÓÊÕÖ®ºó
	*	ÈçºÎ½âÎöÖ¡
	* Õû¸ö¹ý³ÌÎÞÊ±¼ä¿ØÖÆ Ö÷ÒªÊÇ Ö¡½á¹¹µÄ¶¨ÒåºÍÒ»¸öÊµÊ±½ÓÊÜÈÎÎñ
	* ±¾ÎÄ¼þ²ÉÓÃµÄÓ²¼þ½Ó¿ÚÊÇUsart3 ±ðµÄ²»ÒªÔÙÊ¹ÓÃÁË
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

		
		/* MCP2515 Èí¼þ¸´Î» */
		GPIO_ResetBits(GPIOG,GPIO_Pin_14);
		SPIByte(SPIz,SPI_RESET);
		GPIO_SetBits(GPIOG,GPIO_Pin_14);
	 
		/*ÊÊµ±ÑÓÊ± ³õÊ¼»¯²»ÓÃ½éÒâ*/
		Delay(0xffffff);
		
		//Î»Ö¸ÁîMCP2515 ½øÈëÅäÖÃÄ£Ê½
		//CANCTRLµÄREQOP[2:0]¸ÄÎª100
		Mcp2515BitModify(CANCTRL, 0xE0, (1<<REQOP2));
		Delay(65535);
	 	
	 //¼ÆËã²¢ÉèÖÃMCP2515µÄÎ»Ê±¼ä
	 
	 //  Ê±ÖÓÆµÂÊ   Fosc  = 16MHz
	 //  ·ÖÆµ¿ØÖÆÆ÷ CNF1.BRP[5:0] = 7
	 //  ×îÐ¡Ê±¼ä·Ý¶î TQ = 2 * ( BRP + 1 ) / Fosc   = 2*(7+1)/16M = 1uS
	 //  Í¬²½¶Î Sync Seg   = 1TQ
	 //  ´«²¥¶Î Prop Seg   = ( PRSEG + 1 ) * TQ  = 1 TQ
	 //  ÏàÎ»»º³å¶Î Phase Seg1 = ( PHSEG1 + 1 ) * TQ = 3 TQ
	 //  ÏàÎ»»º³å¶Î Phase Seg2 = ( PHSEG2 + 1 ) * TQ = 3 TQ
	 //  Í¬²½Ìø×ª³¤¶ÈÉèÖÃÎª CNF1.SJW[1:0] = 00,  1TQ
	 //  ×ÜÏß²¨ÌØÂÊ NBR = Fbit =  1/(sync seg + Prop seg + PS1 + PS2 )
	 //                       = 1/(8TQ) = 1/8uS = 125kHz
	 
	 //???????CNF1.BRP[5:0] = 7,????????? CNF1.SJW[1:0] = 00
	 Mcp2515WriteByte( CNF1, (1<<BRP0)|(1<<BRP1)|(1<<BRP2) );
	 // ????? Prop Seg ?00,?1TQ,????? Phase Seg1???3TQ
	 Mcp2515WriteByte( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	 // ?? ????? Phase Seg2? 3TQ , ???????
	 Mcp2515WriteByte( CNF3, (1<<PHSEG21) );
	 
	 
	 
	 //ÉèÖÃÎª 500kbps ,TQ = 1/8us
	 //CNF1.BRP[5:0] = 0,????????? CNF1.SJW[1:0] = 01
	// Mcp2515WriteByte( CNF1, (1<<BRP0)|(1<<SJW0) );    // 500kbps
	 //Mcp2515WriteByte( CNF1, (1<<SJW0) );               //1Mbps
		// Prop Seg 00,1TQ, Phase Seg1 3TQ
	 //Mcp2515WriteByte( CNF2, (1<<BTLMODE)|(1<<PHSEG11) );
	 // Phase Seg2  3TQ , 
	 //Mcp2515WriteByte( CNF3, (1<<PHSEG21) );
	 
	 
	 //MCP2515
	// Mcp2515WriteByte( CANINTE, /*(1<<RX1IE)|(1<<RX0IE)*/ 0 );
	 
	 //ÉèÖÃÖÐ¶Ï¿ØÖÆÆ÷ Ê¹ÄÜMCP2515½ÓÊÕ»º³åÆ÷ÖÐ¶Ï
	 Mcp2515WriteByte( CANINTE, (1<<RX1IE)|(1<<RX0IE) );
	 
	 
	 // ¹Ø±Õ½ÓÊÕ»º³åÆ÷0µÄÂË²¨¹¦ÄÜ RXM[1:0]=11, ½ÓÊÕËùÓÐ±¨ÎÄ
	 Mcp2515WriteByte( RXB0CTRL, (1<<RXM1)|(1<<RXM0) );
	 
	 // ¹Ø±Õ½ÓÊÕ»º³åÆ÷1µÄÂË²¨¹¦ÄÜ RXM[1:0]=11 ½ÓÊÕËùÓÐ±¨ÎÄ
	 Mcp2515WriteByte( RXB1CTRL, (1<<RXM1)|(1<<RXM0) );
	 
	 //ÉèÖÃ6¸öÑéÊÕÂË²¨¼Ä´æÆ÷Îª0
	 Mcp2515WriteArray( RXF0SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF1SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF2SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF3SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF4SIDH, temp, 4 );
	 Mcp2515WriteArray( RXF5SIDH, temp, 4 );
	 
	 //ÉèÖÃ2¸öÑéÊÕÂË²¨¼Ä´æÆ÷Îª0
	 Mcp2515WriteArray( RXM0SIDH, temp, 4 );
	 Mcp2515WriteArray( RXM1SIDH, temp, 4 );
	 
	 //ÉèÖÃ½ÓÊÕÏà¹ØÒý½ÅÅäÖÃ¼Ä´æÆ÷ ½ûÖ¹µÚ¶þ¹¦ÄÜ
	 Mcp2515WriteByte(BFPCTRL, 0);
	 
	 //µ÷ÊÔÊ¹ÓÃ ÉèÖÃBFPCTRLµÄRX0BF,RX1BFÎªÊý×ÖÊä³ö
	 Mcp2515BitModify( BFPCTRL, (1<<B1BFE)|(1<<B0BFE)|(1<<B1BFM)|(1<<B0BFM), (1<<B1BFE)|(1<<B0BFE) );
	 
	 
	 //ÉèÖÃ·¢ËÍÏà¹ØÒý½ÅÅäÖÃ¼Ä´æÆ÷ ½ûÖ¹µÚ¶þ¹¦ÄÜ
	 Mcp2515WriteByte(TXRTSCTRL, 0);
	 
	 
	 //MCP2515»·»ØÄ£Ê½ ½øÈë¹¦ÄÜ²âÊÔ
	 //Mcp2515BitModify( CANCTRL, 0XE0, (1<<REQOP1) );
	 
	 //MCP2515½øÈëÕý³£Ä£Ê½
	  Mcp2515BitModify(CANCTRL, 0xE0, 0);
		Delay(65535);
		
		Mcp2515WriteByte(CANINTF,0);
}


/**
  * @brief  Mcp2515WriteByteÐ´¿ØÖÆ¼Ä´æÆ÷
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
  * @brief  Mcp2515ReadByte¶Á¿ØÖÆ¼Ä´æÆ÷
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
  * @brief  Mcp2515ReadRxBuffer¶Á½ÓÊÕ»º³å
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
  * @brief  Mcp2515BitModifyÎ»ÐÞ¸Ä
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
  * @brief  Mcp2515WriteArray ¶ÔÁ¬Ðø¼Ä´æÆ÷½øÐÐÁ¬ÐøÐ´²Ù×÷
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
  * @brief  Mcp2515ReadArray ¶ÔÁ¬Ðø¼Ä´æÆ÷½øÐÐÁ¬Ðø¶Á²Ù×÷
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
  * @brief  SendCANmsg ·¢ËÍcan±¨ÎÄ ÕâÀï±È½Ï¼òµ¥ Ö»ÊÇ·¢ËÍ ²»Éæ¼°»º³å ÒâÒåÅÐ¶Ï
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
/* EID17 ----- EID0 ¹²18Î»  SID10 ------SID0 ¹²11Î»  18+11=29 */
void SendCANmsg(uint8_t bi,
								unsigned long id,
								uint8_t * data,
								uint8_t prop)
{
		uint8_t i;
		uint32_t SID;
		uint32_t EID;
		
		/*Ä¬ÈÏÇé¿öÏÂ idµÄÅÅÁÐÊÇ SID + EID ¸ßÎ»ÔÚ¸ßÎ»*/
		/*´Ëº¯ÊýÖÐ   idµÄÅÅÁÐÊÇ EID + SID ¸ßÎ»ÔÚ¸ßÎ»*/
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
		SPIByte(SPIz,prop >> 6);		/*Ö»ÓÃ67Î»*/
	
		/* Setup standard or extended identifier */
		/* std ÏÈ·¢ È»ºó extÔÙ·¢ ÓÅÏÈ·¢ËÍµÄ²»Ò»Ñù*/
		/* EID17 ----- EID0 ¹²18Î»  SID10 ------SID0 ¹²11Î»  18+11=29 */
		if(prop & 0x10)				/*À©Õ¹Ö¡*/
		{
				SPIByte(SPIz,(uint8_t)(id>>3));
				SPIByte(SPIz,(uint8_t)(id<<5)|(1<<EXIDE)|(uint8_t)(id>>27));
				SPIByte(SPIz,(uint8_t)(id>>19));
				SPIByte(SPIz,(uint8_t)(id>>11));
		} 
		else 								 /*±ê×¼Ö¡*/	
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
uint8_t ready[2];				/* ±ê¼ÇÊý¾Ý½ÓÊÜÇé¿ö*/
									
/**
  * @brief  »ñµÃ½ÓÊÕ×´Ì¬ ¿´ÊÇ1rx »¹ÊÇ 2rx
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
contain message, this situation in our environment cannot happen –
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
  * @brief  MCP2515TaskÖ÷Òª½ÓÊÕmcp2515ÊÕµ½µÄÊý¾Ý,
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
						if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5) == RESET)	/*ÓÐÊý¾Ý:INTµÍµçÆ½*/
						{		
								mcpAppState = STATE_INT_LOW;
						}
						break;
		
        case STATE_INT_LOW:	
						/*Ê×ÏÈÅÐ¶ÏÊÇÄÄ¸ö»º´æ01 10 11ÈýÖÖÇé¿ö*/
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
			
								/*Ê¹ÓÃSPI_READ_RX ¿ÉÒÔ×Ô¶¯Çå³ýÏà¹Ø±êÖ¾Î»*/
								SPIByte(SPIz,SPI_READ_RX | ((bi>>1)<<2));

								/* Make the local copy */
								for(i = 0; i < 14; i++)
								{
										/*ÔÚ½ÓÊÜµ½µÄÍ¬Ê± ÓÃÒ»¸ö±ê¼Ç½øÐÐ¼ÇÂ¼ ±íÃ÷×´Ì¬*/
										/*Ò»¸ö»º³åÇøÖ»ÓÐÁ½ÖÖ×´Ì¬ ½ÓÊÕµ½ÁË Ã»ÓÐ½ÓÊÜµ½*/
										rbuffer[(bi>>1)][i] = SPIByte(SPIz,0x00);
								}
								ready[(bi>>1)] = 1;		/*ÒÑ¾­½ÓÊÕµ½ÁË*/
			
								/*CS high ,MCP2515 disable*/
								GPIO_SetBits(GPIOG,GPIO_Pin_14);
								
								/*²ÉÓÃSPI_READ_RXÃüÁî¾Í²»ÓÃMcp2515WriteByte(CANINTF,0);ÁË*/
						
								for(i = 0; i < 14; i++)
									usart1SendByte(rbuffer[(bi>>1)][i]);
						}
						else
						{
								/* Copy the message from the device and release buffer */
								/* CS low ,MCP2515 enable */
								GPIO_ResetBits(GPIOG,GPIO_Pin_14);
			
								/*Ê¹ÓÃSPI_READ_RX ¿ÉÒÔ×Ô¶¯Çå³ýÏà¹Ø±êÖ¾Î»*/
								SPIByte(SPIz,SPI_READ_RX | (0<<2));

								/* Make the local copy */
								for(i = 0; i < 14; i++)
								{
										/*ÔÚ½ÓÊÜµ½µÄÍ¬Ê± ÓÃÒ»¸ö±ê¼Ç½øÐÐ¼ÇÂ¼ ±íÃ÷×´Ì¬*/
										/*Ò»¸ö»º³åÇøÖ»ÓÐÁ½ÖÖ×´Ì¬ ½ÓÊÕµ½ÁË Ã»ÓÐ½ÓÊÜµ½*/
										rbuffer[0][i] = SPIByte(SPIz,0x00);
								}
								ready[0] = 1;		/*ÒÑ¾­½ÓÊÕµ½ÁË*/
			
								/*CS high ,MCP2515 disable*/
								GPIO_SetBits(GPIOG,GPIO_Pin_14);
								
								/*²ÉÓÃSPI_READ_RXÃüÁî¾Í²»ÓÃMcp2515WriteByte(CANINTF,0);ÁË*/
						
								for(i = 0; i < 14; i++)
									usart1SendByte(rbuffer[0][i]);
								
								/* Copy the message from the device and release buffer */
								/* CS low ,MCP2515 enable */
								GPIO_ResetBits(GPIOG,GPIO_Pin_14);
			
								/*Ê¹ÓÃSPI_READ_RX ¿ÉÒÔ×Ô¶¯Çå³ýÏà¹Ø±êÖ¾Î»*/
								SPIByte(SPIz,SPI_READ_RX | (1<<2));

								/* Make the local copy */
								for(i = 0; i < 14; i++)
								{
										/*ÔÚ½ÓÊÜµ½µÄÍ¬Ê± ÓÃÒ»¸ö±ê¼Ç½øÐÐ¼ÇÂ¼ ±íÃ÷×´Ì¬*/
										/*Ò»¸ö»º³åÇøÖ»ÓÐÁ½ÖÖ×´Ì¬ ½ÓÊÕµ½ÁË Ã»ÓÐ½ÓÊÜµ½*/
										rbuffer[1][i] = SPIByte(SPIz,0x00);
								}
								ready[1] = 1;		/*ÒÑ¾­½ÓÊÕµ½ÁË*/
			
								/*CS high ,MCP2515 disable*/
								GPIO_SetBits(GPIOG,GPIO_Pin_14);
								
								/*²ÉÓÃSPI_READ_RXÃüÁî¾Í²»ÓÃMcp2515WriteByte(CANINTF,0);ÁË*/
						
								for(i = 0; i < 14; i++)
									usart1SendByte(rbuffer[1][i]);
						}
				
						mcpAppState = STATE_WAIT;
						break;
				default:break;
		}
}

/*info ID1 ID2 ID3 ID4 data[0] data[1] data[2] data[3] data[4] data[5] data[6] data[7]*/
typedef struct						/*485µÄCAN½á¹¹*/
{
	uint8_t info;
	uint8_t ID1;
	uint8_t ID2;
	uint8_t ID3;
	uint8_t ID4;
	uint8_t data[8];				/*data[0]´ú±í¸ß×Ö½Ú data[7]´ú±íµÍ×Ö½Ú*/
}CanTo485;

/**
  * @brief  MCP2515Read()¶ÁÈ¡½ÓÊÜµ½µÄÊý¾Ý ÏÈÅÐ¶Ï½ÓÊÜµ½Ã»ÓÐ ÔÙÌáÈ¡Êý¾Ý
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