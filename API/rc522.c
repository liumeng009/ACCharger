#include "spibysoft.h"
#include "rc522.h"
#include "string.h"
#include "usart.h"


char PcdComMF522(uint8_t Command, 
                 uint8_t *pInData, 
                 uint8_t InLenByte,
                 uint8_t *pOutData, 
                 uint32_t  *pOutLenBit);
void CalulateCRC(uint8_t *pIndata,uint8_t len,uint8_t *pOutData);
void MFRC522_AntennaOn(void);

CARD_STATUS cardStatus;

    /*********************************************************************
    * Function:        void delay_ns(UINT16 ns)
    *
    * PreCondition:     none
    *
    * Input:		    UINT16 ns - nanoseconds to delay               
    *
    * Output:		    nothing
    *    
    * Side Effects:	    none
    *
    * Overview:		    This function delay 
    *
    * Note:			    it is not precise and needs to be fixed
    ********************************************************************/
void delay_ns(uint16_t ns)
{
  uint16_t i;
  for(i=0; i<ns; i++)
  {
     __ASM("NOP");
  }
}

   /*********************************************************************
    * Function:        char MFRC522_Request(uint8_t req_code,uint8_t *pTagType)
    *
    * PreCondition:     SPI has been configured 
    *
    * Input:		    uint8_t req_code - 
    *               uint8_t *pTagType - 
    *
    * Output:		    char - return MI_OK if success
    *               pTagType - return card type 
    *                                           0x4400 = Mifare_UltraLight       
    *                                           0x0400 = Mifare_One(S50)
    *                                           0x0200 = Mifare_One(S70)
    *                                           0x0800 = Mifare_Pro(X)
    *                                           0x4403 = Mifare_DESFire
    *    
    * Side Effects:	    none
    *
    * Overview:		    This function search card and return card types
    *
    * Note:			    None
    ********************************************************************/
char MFRC522_Request(uint8_t req_code,uint8_t *pTagType)
{
	char status;  
	uint32_t unLen;
	uint8_t ucComMF522Buf[MAXRLEN]; 

	ClearBitMask(Status2Reg,0x08);
	WriteRawRC(BitFramingReg,0x07);
	SetBitMask(TxControlReg,0x03);
 
	ucComMF522Buf[0] = req_code;

	status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

	if ((status == MI_OK) && (unLen == 0x10))
	{    
		*pTagType     = ucComMF522Buf[0];
		*(pTagType+1) = ucComMF522Buf[1];
	}
	else
	{   status = MI_ERR;   }
   
	return status;
}
 
  /*********************************************************************
    * Function:        char MFRC522_Anticoll(uint8_t *pSnr)
    *
    * PreCondition:     none 
    *
    * Input:		    uint8_t *pSnr  
    *
    * Output:		    return MI_OK if success
    *               return the 4 bytes serial number     
    *    
    * Side Effects:	    none
    *
    * Overview:		    This function prevent conflict and return the 4 bytes serial number
    *
    * Note:			    None
    ********************************************************************/
char MFRC522_Anticoll(uint8_t *pSnr)
{
    char status;
    uint8_t i,snr_check=0;
    uint32_t unLen;
    uint8_t ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
				for (i=0; i<4; i++)
        {   
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {   
						status = MI_ERR;    
				}
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}

/*********************************************************************
    * Function:       char MFRC522_Select(uint8_t *pSnr)
    *
    * PreCondition:     SPI has been configured 
    *
    * Input:		    uint8_t *pSnr
    *
    * Output:		    char - return MI_OK if success
    *               
    *    
    * Side Effects:	    none
    *
    * Overview:		    
    *
    * Note:			    None
    ********************************************************************/
char MFRC522_Select(uint8_t *pSnr)
{
    char status;
    uint8_t i;
    uint32_t unLen;
    uint8_t ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}

 /*********************************************************************
    * Function:   char MFRC522_AuthState(uint8_t auth_mode,uint8_t addr,uint8_t *pKey,uint8_t *pSnr)
    *
    * PreCondition:     none 
    *
    * Input:		    uint8_t auth_mode -   Password Authentication Mode
    *                                        0x60 = A key authentication
                                             0x61 = B key authentication    
    *               uint8_t addr  -      Block Address
    *               uint8_t *pKey  -     Sector Password
    *               uint8_t *pSnr  -    4 bytes serial number
    *
    * Output:		    char - return MI_OK if success
    *               
    *    
    * Side Effects:	    none
    *
    * Overview:		    This function verify card password
    *
    * Note:			    None
    ********************************************************************/             
char MFRC522_AuthState(uint8_t auth_mode,uint8_t addr,uint8_t *pKey,uint8_t *pSnr)
{
    char status;
    uint32_t unLen;
    uint8_t i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   }
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
    //memcpy(&ucComMF522Buf[2], pKey, 6); 
    //memcpy(&ucComMF522Buf[8], pSnr, 4); 
    
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}

 /*********************************************************************
    * Function:        char MFRC522_Read(uint8_t addr, uint8_t *pData)
    *
    * PreCondition:     none
    *
    * Input:		    uint8_t addr   - block address
    *               uint8_t *pData  - block data
    *
    * Output:		    char - return MI_OK if success
    *    
    * Side Effects:	    none
    *
    * Overview:		    This function read block data
    *
    * Note:			    None
    ********************************************************************/
char MFRC522_Read(uint8_t addr, uint8_t *pData)
{
    char  status;
    uint32_t unLen;
    uint8_t i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
   // {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = ucComMF522Buf[i];   }
    }
    else
    {   status = MI_ERR;   }
    
    return status;
}

 /*********************************************************************
    * Function:      char MFRC522_Write(uint8_t addr,uint8_t *pData)
    *
    * PreCondition:     SPI has been configured 
    *
    * Input:		    uint8_t addr  - block address
    *               uint8_t *pData  - data to write
    *
    * Output:		    char - return MI_OK if success
    *                                           
    *    
    * Side Effects:	    none
    *
    * Overview:		    This function write a block of data to addr
    *
    * Note:			    None
    ********************************************************************/               
char MFRC522_Write(uint8_t addr,uint8_t *pData)
{
    uint8_t status;
    uint32_t unLen;
    uint8_t i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
       // memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {    
        	ucComMF522Buf[i] = *(pData+i);   
        }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}

/*
 * Function：MFRC522_Halt
 * Description：Command the cards into sleep mode
 * Input parameters：null
 * return：MI_OK
 */
 /*********************************************************************
    * Function:       char MFRC522_Halt(void)
    *
    * PreCondition:     none
    *
    * Input:		    none 
    *
    * Output:		    none
    *    
    * Side Effects:	    none
    *
    * Overview:		    This function command the cards into sleep mode
    *
    * Note:			    None
    ********************************************************************/
char MFRC522_Halt(void)
{
    char status;
    uint32_t unLen;
    uint8_t ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return MI_OK;
}

 /*********************************************************************
    * Function:   void CalulateCRC(uint8_t *pIndata,uint8_t len,uint8_t *pOutData)
    *
    * PreCondition:     none
    *
    * Input:		    uint8_t *pIndata - input datas
    *               uint8_t len       - data length
    *               uint8_t *pOutData  - output data
    *
    * Output:		    uint8_t - 2 bytes CRC result
    *    
    * Side Effects:	    none
    *
    * Overview:		    This function calculate the CRC
    *
    * Note:			    None
    ********************************************************************/
void CalulateCRC(uint8_t *pIndata,uint8_t len,uint8_t *pOutData)
{
    uint8_t i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   WriteRawRC(FIFODataReg, *(pIndata+i));   }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/*********************************************************************
    * Function:        char MFRC522_Reset(void)
    *
    * PreCondition:     none
    *
    * Input:		    none 
    *
    * Output:		    return MI_OK
    *    
    * Side Effects:	    reset the RC522
    *
    * Overview:		    This function reset the RC522
    *
    * Note:			    None
    ********************************************************************/
char MFRC522_Reset(void)
{
  	SET_RC522RST;
    delay_ns(10);
	  CLR_RC522RST;
    delay_ns(10);
	  SET_RC522RST;
    delay_ns(10);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    delay_ns(10);
    WriteRawRC(ModeReg,0x3D);           
    WriteRawRC(TReloadRegL,30);           
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);
	  WriteRawRC(TxAutoReg,0x40);
    return MI_OK;
}
   /*********************************************************************
    * Function:        char MFRC522_ConfigISOType(uint8_t type)
    *
    * PreCondition:     none
    *
    * Input:		    uint8_t type
    *
    * Output:		  return MI_OK if type == 'A' 
    *          
    * Side Effects:	    none
    *
    * Overview:		    This function configure the ISO type
    *
    * Note:			    None
    ********************************************************************/
char MFRC522_ConfigISOType(uint8_t type)
{
   if (type == 'A')                 
   { 
       ClearBitMask(Status2Reg,0x08);
       WriteRawRC(ModeReg,0x3D);
       WriteRawRC(RxSelReg,0x86);
       WriteRawRC(RFCfgReg,0x7F);
   	   WriteRawRC(TReloadRegL,30); 
	     WriteRawRC(TReloadRegH,0);
       WriteRawRC(TModeReg,0x8D);
	     WriteRawRC(TPrescalerReg,0x3E);
	     delay_ns(1000);
       MFRC522_AntennaOn();
   }
   else{ return -1; }
   
   return MI_OK;
}
char PcdComMF522(uint8_t Command, 
                 uint8_t *pInData, 
                 uint8_t InLenByte,
                 uint8_t *pOutData, 
                 uint32_t *pOutLenBit)
{
    char status = MI_ERR;
    uint8_t irqEn   = 0x00;
    uint8_t waitFor = 0x00;
    uint8_t lastBits;
    uint8_t n;
    uint32_t i;
    switch (Command)
    {
        case PCD_AUTHENT:
			irqEn   = 0x12;
			waitFor = 0x10;
			break;
		case PCD_TRANSCEIVE:
			irqEn   = 0x77;
			waitFor = 0x30;
			break;
		default:
			break;
    }
   
    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   WriteRawRC(FIFODataReg, pInData[i]);    }
    WriteRawRC(CommandReg, Command);
   
    
    if (Command == PCD_TRANSCEIVE)
    {    SetBitMask(BitFramingReg,0x80);  }
    
		i = 2000;
    do 
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);

    if (i!=0)
    {    
        if(!(ReadRawRC(ErrorReg)&0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   status = MI_NOTAGERR;   }
            if (Command == PCD_TRANSCEIVE)
            {
               	n = ReadRawRC(FIFOLevelReg);
              	lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = ReadRawRC(FIFODataReg);    }
            }
        }
        else
        {   status = MI_ERR;   }
        
    }
    SetBitMask(ControlReg,0x80);           // stop timer now
    WriteRawRC(CommandReg,PCD_IDLE); 
    return status;
}

/*********************************************************************
    * Function:        void MFRC522_AntennaOn(void)
    *
    * PreCondition:     none
    *
    * Input:		    none
    *
    * Output:		    none
    *    
    * Side Effects:	    Antenna On
    *
    * Overview:	  	This function command the RC522 to switch on the antenna
    *
    * Note:			    None
    ********************************************************************/
void MFRC522_AntennaOn(void)
{
    uint8_t i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

/*********************************************************************
    * Function:        void MFRC522_AntennaOff(void)
    *
    * PreCondition:     none 
    *
    * Input:		   none
    *
    * Output:		    none
    *    
    * Side Effects:	  Antenna Off
    *
    * Overview:		 This function command the RC522 to switch off the antenna
    *
    * Note:			    None
    ********************************************************************/
void MFRC522_AntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}


void RFID_Init(void)
{
  MFRC522_Reset();
  MFRC522_AntennaOff();  
  MFRC522_AntennaOn();
  MFRC522_ConfigISOType( 'A' );
}

 /**
  * @brief  RC522Task主要负责有没有卡到来，卡到来了余额是多少，并且负责向卡写数据
目前总结为三个状态，在线/离线 读值 写值 每10ms刷新一次，
		在线/离线 可以通过寻卡命令完成，没有任何问题
		读值			也没有问题
		写值			只写一次 通过标记来记录 频繁的写会出现问题，也没有必要
  * @param  
  * @retval 
  */
#define BLOCKADDR 0x38
/*密钥*/
uint8_t key[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
void MFRC522Update()
{
		/*操作状态返回值*/
		uint8_t status;
		/*卡状态缓冲*/
		uint8_t info[10];
		uint8_t money[16];
	
		MFRC522_Reset();
		/*寻卡 并且将卡类型赋予 info[0] info[1]*/
		status = MFRC522_Request(PICC_REQIDL,info);
		if(status == MI_OK)
		{
				cardStatus.status = 1;
				//usart1SendByte(cardStatus.status);
		}
		else
		{
				/*所有卡状态都清零了*/
				memset(&cardStatus,0,sizeof(cardStatus));
				return;
		}
		
		/*防碰撞 同时也是 读卡号 info[2]  info[3]  info[4]  info[5]*/
		status = MFRC522_Anticoll(&info[2]);
		if(status == MI_OK)
		{
				//usart1SendByte(info[2]);
				//usart1SendByte(info[3]);
				//usart1SendByte(info[4]);
				//usart1SendByte(info[5]);
				memcpy(cardStatus.serialNum,&info[2],4);
		}
		else
		{
				memset(&cardStatus,0,sizeof(cardStatus));
				return;
		}
		
		/*选定卡*/
		status = MFRC522_Select(&info[2]);
		if(status == MI_OK)
		{
			
		}
		else
		{
				memset(&cardStatus,0,sizeof(cardStatus));
				return;
		}
		
		/*A方式操作 */
		status = MFRC522_AuthState(PICC_AUTHENT1A,BLOCKADDR, key, &info[2]);
		if(status == MI_OK)
		{
				cardStatus.authStatus = 1;
		}
		else
		{
				memset(&cardStatus,0,sizeof(cardStatus));
				return;
		}
		
		status = MFRC522_Read(BLOCKADDR, money);
		if(status == MI_OK)
		{
				//usart1SendByte(money[0]);
				//usart1SendByte(money[1]);
				//usart1SendByte(money[2]);
				//usart1SendByte(money[3]);
				cardStatus.rxWalletValue = (money[3]<<24)|(money[2]<<16)|(money[1]<<8)|(money[0]);
		}
		else
		{
				memset(&cardStatus,0,sizeof(cardStatus));
				return;
		}
		
		/*如果有数据要写*/
		if(cardStatus.readyToTx == 1)
		{
				money[0] = cardStatus.txWalletValue;
				money[1] = cardStatus.txWalletValue >> 8;
				money[2] = cardStatus.txWalletValue >> 16;
				money[3] = cardStatus.txWalletValue >> 24;
				MFRC522_Write(BLOCKADDR, money);
			
				cardStatus.readyToTx = 0;			
		}
}

 /**
  * @brief  WalletWrite将数值写入结构体 至于什么时候执行操作，要看周期走到了哪里
						属于外部调用API
  * @param  
  * @retval 
  */
void WalletWrite(uint32_t value)
{
		if((cardStatus.status == 1) && (cardStatus.authStatus == 1))
		{
				cardStatus.readyToTx = 1;
				cardStatus.txWalletValue = value;
		}
}

 /**
  * @brief  GetCardStatus 返回卡状态 供外部调用API
  * @param  
  * @retval 
  */
CARD_STATUS GetCardStatus(void)
{
		return cardStatus;
}