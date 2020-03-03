#include "spibysoft.h"
#include "string.h"
#include "usart.h"
#include "systick.h"


void SpiBySoftInit()
{
		GPIO_RC522_Configuration();
}
 /*******************************************************************************
* 描  述  :  开启时钟及定义LED、USART1、RFID的IO口
* 输  入  :  无
* 输  出  :  无
* 返  回  :  无
*******************************************************************************/
void GPIO_RC522_Configuration(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
				 
	  /*SPI引脚设置*/
	 /*PC.6---OUT---MISO*/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	 /*PC.7---DIN---MOSI   PC.8---CLK---SCK*/
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8;   
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		 /*推挽输出*/
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	 /*PA8---RST---RST */
   GPIO_InitStructure.GPIO_Pin =GPIO_Pin_8;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
	 /*PC9---CS---SDA*/
	 GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9; 
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
	 
}
void NVIC_RC522_Configuration(void)
{

}
/*******************************************************************
@功    能：SPI写数据 由于采用IO操作数据操作过程中不可以中断 而且在
@										 中断中不能对SPI操作
@输    入： 无
@输    出：无返回值
*******************************************************************/ 
void SPIWriteByte(uint8_t num)    
{  
	uint8_t count=0;
	uint8_t i;     
	for(count=0;count<8;count++)  
	{ 	  
			if(num&0x80)
				TRC_DIN_SET(1);
			else 
				TRC_DIN_SET(0);
			num<<=1;    
			TRC_CLK_SET(0);				/*上升沿有效 */
			for(i=0;i<15;i++);	  /*延时*/
			TRC_CLK_SET(1);
			for(i=0;i<15;i++);		/*延时*/    
	}	    
} 
/*******************************************************************
@功    能：SPI读数据  由于采用IO操作数据操作过程中不可以中断 而且在
											中断中不能对SPI操作
@输    入： 无
@返    回:	 无
********************************************************************/ 
uint8_t SPIReadByte(void)	  
{ 	 
		uint8_t  SPICount,i;    // Counter used to clock out the data
		uint8_t  SPIData;                  
		SPIData = 0;                 /*下降沿有效*/
	
		for(SPICount = 0; SPICount < 8; SPICount++) // Prepare to clock in the data to be read
		{
				SPIData <<=1;                     // Rotate the data
			
				TRC_CLK_SET(1); 	
				for(i=0;i<15;i++); 
				TRC_CLK_SET(0); 		
				for(i=0;i<15;i++);               // Raise the clock to clock the data out of the MAX7456
				if(DOUT)
				{
						SPIData|=0x01;
				}   														 // Drop the clock ready for the next bit
		}                                    // and loop back
		return (SPIData);  
}    
/*******************************************************************
@功    能：读RC522寄存器 也就是某地址的值
@参数说明：Address[IN]:寄存器地址
@返    回：读出的值
********************************************************************/
uint8_t ReadRawRC(uint8_t Address)
{
    uint8_t  ucAddr,i;
    uint8_t ucResult=0;
		
    TRC_CS_SET(0);
    ucAddr = ((Address<<1)&0x7E)|0x80;	//读寄存器的时候，地址最高位为 1，最低位为0，1-6位取决于地址
		SPIWriteByte(ucAddr);
		ucResult=SPIReadByte();
    TRC_CS_SET(1);
		return ucResult;
}

/*******************************************************************
@功    能：写RC522寄存器	也就是写入某地址某值
@参数说明：Address[IN]:寄存器地址
@          value[IN]:写入的值
*******************************************************************/
void WriteRawRC(uint8_t  Address, uint8_t  value)
{  
    uint8_t ucAddr,i;

    TRC_CS_SET(0);
    ucAddr = ((Address<<1)&0x7E);	 //写寄存器的时候，地址最高位为 0，最低位为0，1-6位取决于地址
		SPIWriteByte(ucAddr);
		SPIWriteByte(value);
    TRC_CS_SET(1);
}

/*******************************************************************
@功    能：置RC522寄存器位 属于位操作
@参数说明：reg[IN]:寄存器地址
@          mask[IN]:置位值
*******************************************************************/
void SetBitMask(uint8_t reg, uint8_t mask)  
{
    int8_t  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/*******************************************************************
@功    能：清RC522寄存器位
@参数说明：reg[IN]:寄存器地址
@          mask[IN]:清位值
*******************************************************************/
void ClearBitMask(uint8_t reg, uint8_t  mask)  
{
    int8_t  tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 
