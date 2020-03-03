/******************与电表进行485通信**************************
UART4_TX: PC10 
UART4_RX: PC11
CON:      PG7
这个文件中 设计了所有与iDM100E型电能计量模块所有的数据，而与UART4
的设置没有关系，发送的时候写入FIFO就可以，然后周期检测接收标志，
进行处理。
*************************************************************/

#include "DJZY102.h"
#include "message.h"
#include "usart.h"

uint8_t TX_VALUE_V[16] ={0x68,0x50,0x00,0x01,0x07,0x01,0x16,0x68,0x11,0x04,0x33,0x34,0x34,0x35,0x24,0x16};
uint8_t TX_VALUE_A[16] ={0x68,0x50,0x00,0x01,0x07,0x01,0x16,0x68,0x11,0x04,0x33,0x34,0x35,0x35,0x25,0x16};
uint8_t TX_VALUE_KW[16]={0x68,0x50,0x00,0x01,0x07,0x01,0x16,0x68,0x11,0x04,0x33,0x33,0x34,0x33,0x21,0x16};

uint16_t RX_VALUE_V[2] ;
uint16_t RX_VALUE_A[3] ;
uint16_t RX_VALUE_KW[4] ;
uint8_t buffer[24];

/*这3个变量存储的便是当前电压电流电量 每3秒更新一次 
	用extern声明可以作为外部访问的接口 也可以写3个成员函数
	来访问 */
float VALUE_V ;
float VALUE_A ;
float VALUE_KW ;

float VALUE_KW_Start ;
float VALUE_KW_End; 

/**
  * @brief  iDM100ECommunication负责所有与电表模块的通信
						每1s 进行一次发送 电能-电流-电压
						每10ms 进行一次接收检测 紧接着就处理
						但是没有考虑 无返回的异常情况 也就是说每个数据都是有
						有效时限 超过这个时间值就认为无效
  * @param  None
  * @retval None
  */
void iDM100ECommunication(struct message data)
{
		static uint8_t mode = 0;  /*mode 0: 电能 1：电流 2：电压*/
		uint8_t i;
		switch(data.cmd)					/*cmd 0: 发送  1：接收*/
		{
			case 0:if(mode == 0)
						 {
								UsartWrite(4,TX_VALUE_KW,16);
								mode = 1;
								break;
						 }
						 if(mode == 1)
						 {
								UsartWrite(4,TX_VALUE_A,16);
								mode = 2;
								break;
						 }
						 if(mode == 2)
						 {
								UsartWrite(4,TX_VALUE_V,16);
								mode = 0;
								break;
						 }
			case 1:	if(UsartRead(4, buffer) == 0)
									break;
							switch(buffer[13])
							{
								case 0x06:for(i=0;i<2;i++)
													{
															RX_VALUE_V[i]=buffer[19-i]-0x33;
													} 
													VALUE_V=((RX_VALUE_V[0]&0xF0)>>4)*100+(RX_VALUE_V[0]&0x0F)*10+((RX_VALUE_V[1]&0xF0)>>4)+(float )(RX_VALUE_V[1]&0x0F)/10;
													//usart1SendByte(VALUE_V);
													break;
								case 0x07:for(i=0;i<3;i++)
													{
															RX_VALUE_A[i]=buffer[20-i]-0x33;
													} 
													VALUE_A=((RX_VALUE_A[0]&0xF0)>>4)*100+(RX_VALUE_A[0]&0x0F)*10+((RX_VALUE_A[1]&0xF0)>>4)+(float )(RX_VALUE_A[1]&0x0F)/10+(float )((RX_VALUE_A[2]&0xF0)>>4)/100+(float )(RX_VALUE_A[2]&0x0F)/1000;
													//usart1SendByte(VALUE_A);
													break;
								case 0x08:for(i=0;i<4;i++)
													{
															RX_VALUE_KW[i]=buffer[21-i]-0x33;
													}
													VALUE_KW_End=((RX_VALUE_KW[0]&0xF0)>>4)*100000+(RX_VALUE_KW[0]&0x0F)*10000+((RX_VALUE_KW[1]&0xF0)>>4)*1000+(RX_VALUE_KW[1]&0x0F)*100+((RX_VALUE_KW[2]&0xF0)>>4)*10\
													+(RX_VALUE_KW[2]&0x0F)+(float )((RX_VALUE_KW[3]&0xF0)>>4)/10+(float )(RX_VALUE_KW[3]&0x0F)/100;
													VALUE_KW=VALUE_KW_End-VALUE_KW_Start+0.001;//为保持精度，增加0.001
													//usart1SendByte(VALUE_KW);
													break;
								default:break;
							}
			default:break;
		}
}
/**
  * @brief  读取当前电压指针
  * @param  None
  * @retval None
  */
float * GetVoltage(void)
{
		return &VALUE_V;
}

/**
  * @brief  读取当前电流指针
  * @param  None
  * @retval None
  */
float * GetCurrent(void)
{
		return &VALUE_A;
}

/**
  * @brief  读取当前电能指针
  * @param  None
  * @retval None
  */
float * GetW(void)
{
		return &VALUE_KW;
}