/* Includes ------------------------------------------------------------------*/
#include "pgn.h"

/* Exported types ------------------------------------------------------------*/
/*参数表，和bms通信的基础，他还要与很多其他表格联系*/
extern pgnGroup pgnValue;

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define NUMBER_OF_MESSAGE 22
/* Exported functions ------------------------------------------------------- */

/*变量名：messageTable
  描述：此表记录了所有报文的全部信息，用来遍历查询*/
typedef struct
{
	uint8_t identifier;	//代号
	uint16_t pgn;		//参数组
	uint8_t priority;	//优先级
	uint8_t length;		//长度
	uint16_t cycle;		//周期
}messageItem;

/*如果接收到了can的消息，通过遍历PGN可以映射到代号上
知道了代号就可以告诉conditionsUpdate接收到了什么帧，
conditionUpdate通过代号映射到条件列表以进行状态迁移
但是 每一种CAN根据数据不同就不可能仅仅是CHM或BHM这种
简单的情况，根据数据，可以分为正常和异常，因此还要再
细化为正常情况、异常情况两种基本情况*/
/*有的can消息，其内容是影响迁移的，有的则不影响，按理
应该把CAN消息类型+CAN消息内容 一起在消息中发送给条件
队列*/

messageItem messageTable[NUMBER_OF_MESSAGE] = {
{CHM, 0x2600, 6, 3, 250},
{BHM, 0x2700, 6, 2, 250},

{CRM, 0x0100, 6, 8, 250},
{BRM, 0x0200, 7, 11, 250},

{BCP, 0x0600, 7, 13, 500},
{CTS, 0x0700, 6, 7, 500},
{CML, 0x0800, 6, 8, 250},
{BRO, 0x0900, 4, 1, 250},
{CRO, 0x0a00, 4, 1, 250},

{BCL, 0x1000, 6, 5, 50},
{BCS, 0x1100, 7, 9, 250},
{CCS, 0x1200, 6, 8, 50},
{BSM, 0x1300, 6, 7, 250},
{BMV, 0x1500, 7, 0, 10000},
{BMT, 0x1600, 7, 0, 10000},
{BSP, 0x1700, 7, 0, 10000},
{BST, 0x1900, 4, 4, 10},
{CST, 0x1A00, 4, 4, 10},

{BSD, 0x1C00, 6, 7, 250},
{CSD, 0x1D00, 6, 8, 250},

{BEM, 0x1E00, 2, 4, 250},
{CEM, 0x1F00, 2, 4, 250}
};
/**
  * @brief  PgnGroupInit初始化所有参数到默认状态
  * @param  
  * @retval 
  */
void PgnGroupInit()
{
		memset(&pgnValue,0,sizeof(pgnValue));
}
/**
  * @brief  FormulaOnPgnGroup定义pgnGroup上的计算 表达的是表数据之间的关系
		通过实时任务的方式，可以在极短的时间内更新表的状态，从而使数据有效 同时
		也会伴有容错处理
  * @param  
  * @retval 
  */
void FormulaOnPgnGroup()
{
		if(pgnValue.SPN2565 != 0)				/*如果版本号不为V0.0 例如V1.0 V1.1*/
		{
				pgnValue.SPN2560 = 0xAA;		/*代表可以识别*/
		}
		if(1/*充电机全部处于开状态*/)
		{
				pgnValue.SPN2830 = 0xaa;		/*代表充电状态准备就绪*/
		}
}
/**
  * @brief  PgnToCode pgn到编码
  * @param  None
  * @retval None
  */
uint8_t PgnToCode(uint8_t pgn)
{
		switch(pgn)
		{
			case BHM:return 0xB1;
			case BRM:return 0xB2;
			case BCP:return 0xB3;
			case BRO:return 0xB4;
			case BCS:return 0xB5;
			case BCL:return 0xB6;
			case BSM:return 0xB7;
			case BST:return 0xB8;
			case BSD:return 0xB9;
			default:break;
		}
}

/*报文记录队列 所有接收到的报文都会在这里记录 同时转换成编码*/
conditionQueue canTriggerQueue;

/**
  * @brief  NormalMessageParse 将接收到的帧解析到参数表 此函数只是将接收到的can帧中的
	数据填入到参数表中，没有任何计算和容错，只是最基本的操作功能。
  * @param  CanRxMsg
  * @retval None
  */
void NormalMessageParse(CanRxMsg rxMessage)
{
	uint32_t extId;
	uint32_t pgn;
	if (rxMessage.IDE == CAN_ID_STD)	/*得到的帧是标准帧 错误返回*/
  {
			return;
  }
	
	extId = rxMessage.ExtId;			/*得到实际ID值*/
	pgn = (extId >> 16) & 0x000000ff; /*从ID中提取pgn*/

	/*转换为条件编码 压入队列 以供conditionUpdate调用*/
	canTriggerQueue.queue[canTriggerQueue.size] = PgnToCode(pgn);
	canTriggerQueue.size++;
	
	switch(pgn)		/*根据PGN来更新相应的参数组*/
	{
		case BHM:	pgnValue.SPN2601 = ((rxMessage.Data[1])<<8)|\
																	(rxMessage.Data[0]);		/*最高允许充电总电压*/
							/*首先更新数据*/
							
							/*然后更新定时器 记录数据实时性*/
							//...
							break;
		case BRM:	pgnValue.SPN2565 = ((rxMessage.Data[2])<<16)|\
																 ((rxMessage.Data[1])<<8)|\
																  (rxMessage.Data[0]);
							pgnValue.SPN2566 =  (rxMessage.Data[3]);
							pgnValue.SPN2567 = ((rxMessage.Data[5])<<8)|\
																 ((rxMessage.Data[4]));
							pgnValue.SPN2568 = ((rxMessage.Data[7])<<8)|\
																 ((rxMessage.Data[6]));
							break;
		case BRO:	pgnValue.SPN2829 = ((rxMessage.Data[0]));break;
		case BCL:	pgnValue.SPN3072 = ((rxMessage.Data[1])<<8)|\
																 ((rxMessage.Data[0]));
							pgnValue.SPN3073 = ((rxMessage.Data[3])<<8)|\
																 ((rxMessage.Data[2]));
							pgnValue.SPN3074 = ((rxMessage.Data[4]));
							break;
		case BSM:	pgnValue.SPN3085 = ((rxMessage.Data[0]));
							pgnValue.SPN3086 = ((rxMessage.Data[1]));
							pgnValue.SPN3087 = ((rxMessage.Data[2]));
							pgnValue.SPN3088 = ((rxMessage.Data[3]));
							pgnValue.SPN3089 = ((rxMessage.Data[4]));
							pgnValue.SPN3090 = ((rxMessage.Data[5])&0x03);
							pgnValue.SPN3091 = ((rxMessage.Data[5]>>2)&0x03);
							pgnValue.SPN3092 = ((rxMessage.Data[5]>>4)&0x03);
							pgnValue.SPN3093 = ((rxMessage.Data[5]>>6)&0x03);
							pgnValue.SPN3094 = ((rxMessage.Data[6])&0x03);
							pgnValue.SPN3095 = ((rxMessage.Data[6]>>2)&0x03);
							pgnValue.SPN3096 = ((rxMessage.Data[6]>>4)&0x03);
							break;
		case BST:	pgnValue.SPN3511 = ((rxMessage.Data[0]));
							pgnValue.SPN3512 = ((rxMessage.Data[2])<<8)|\
																 ((rxMessage.Data[1]));
							pgnValue.SPN3513 = ((rxMessage.Data[3]));
							break;
		case BSD:	pgnValue.SPN3601 = ((rxMessage.Data[0]));
							pgnValue.SPN3602 = ((rxMessage.Data[2])<<8)|\
																 ((rxMessage.Data[1]));
							pgnValue.SPN3603 = ((rxMessage.Data[4])<<8)|\
																 ((rxMessage.Data[3]));
							pgnValue.SPN3604 = ((rxMessage.Data[5]));
							pgnValue.SPN3605 = ((rxMessage.Data[6]));
							break;
		case BEM:	pgnValue.SPN3901 = ((rxMessage.Data[0])&0x03);
							pgnValue.SPN3902 = ((rxMessage.Data[0]>>2)&0x03);
							pgnValue.SPN3903 = ((rxMessage.Data[1])&0x03);
							pgnValue.SPN3904 = ((rxMessage.Data[1]>>2)&0x03);
							pgnValue.SPN3905 = ((rxMessage.Data[2])&0x03);
							pgnValue.SPN3906 = ((rxMessage.Data[2]>>2)&0x03);
							pgnValue.SPN3907 = ((rxMessage.Data[3])&0x03);
							break;
		
	}
}
/**
  * @brief  LongMessageParse 将接收到的帧解析到参数表 此函数只是将接收到的can帧中的
	数据填入到参数表中，没有任何计算和容错，只是最基本的操作功能。
  * @param  CanRxMsg
  * @retval None
  */
void LongMessageParse(struct linkStatus * pD)
{
	uint8_t temp[40],i,j;
	uint32_t pgn;
	
	pgn = pD->PGNCode;
	
	for(i = 0, j = 0; i < pD->numOfPackage; i++, j = j + 7)
	{
			memcpy(temp+j,((pD->buffer[i]).data + 1),7);
	}
	
	/*转换为条件编码 压入队列 以供conditionUpdate调用*/
	canTriggerQueue.queue[canTriggerQueue.size] = PgnToCode(pgn);
	canTriggerQueue.size++;
	
	switch(pgn)		/*根据PGN来更新相应的参数组*/
	{
		case BCP:	pgnValue.SPN2816 = ((temp[1])<<8)|\
																 ((temp[0]));
							pgnValue.SPN2817 = ((temp[3])<<8)|\
																 ((temp[2]));
							pgnValue.SPN2818 = ((temp[5])<<8)|\
																 ((temp[4]));
							pgnValue.SPN2819 = ((temp[7])<<8)|\
																 ((temp[6]));
							pgnValue.SPN2820 = ((temp[8]));
							pgnValue.SPN2821 = ((temp[10])<<8)|\
																 ((temp[9]));
							pgnValue.SPN2822 = ((temp[12])<<8)|\
																 ((temp[11]));
							break;
		case BCS:	pgnValue.SPN3075 = (temp[1] << 8)| temp[0];
							pgnValue.SPN3076 = (temp[3] << 8)| temp[2];
							pgnValue.SPN3077 = (temp[5] << 8)| temp[4];
							pgnValue.SPN3078 =  temp[6];
							pgnValue.SPN3079 = (temp[8] << 8)| temp[7];
		case BMV:	
							break;
		case BMT:	
							break;
		case BSP:	
							break;
	}
}

/**
  * @brief  ActionHandle (第二层)负责将参数表中的数据发送出去 此函数
		只是单纯地将can参数表中的内容填入can帧中，然后发送，没有任何计算和
		容错机制，属于最基础的操作功能。
  * @param  
  * @retval None
  */
void ActionHandle(struct message txMessage)
{
		uint8_t pgn;
		CanTxMsg canTxMessage;
	
		pgn = txMessage.parameter_1;
	
		switch(pgn)
		{
			case 0xc1:	//canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CHM<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//数据帧
								canTxMessage.IDE = CAN_ID_EXT;			//扩展帧
								canTxMessage.DLC = 3;							//数据长度
								canTxMessage.Data[0] = pgnValue.SPN2600;				//写入数据
								canTxMessage.Data[1] = pgnValue.SPN2600 >> 8;		//写入数据
								canTxMessage.Data[2] = pgnValue.SPN2600 >> 16;  //写入数据
			
								CanQueueTxEnqueue(canTxMessage);	/*投入队列准备发送*/
								break;
			
			case 0xc2: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CRM<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//数据帧
								canTxMessage.IDE = CAN_ID_EXT;			//扩展帧
								canTxMessage.DLC = 8;							//数据长度
								canTxMessage.Data[0] = pgnValue.SPN2560;				//写入数据
								canTxMessage.Data[1] = pgnValue.SPN2561;		//写入数据
								canTxMessage.Data[2] = pgnValue.SPN2561 >> 8;  //写入数据
								canTxMessage.Data[3] = pgnValue.SPN2561 >> 16;
								canTxMessage.Data[4] = pgnValue.SPN2561 >> 24;
								canTxMessage.Data[5] = pgnValue.SPN2562; 
								canTxMessage.Data[6] = pgnValue.SPN2562 >> 8;
								canTxMessage.Data[7] = pgnValue.SPN2562 >> 16;
								
								CanQueueTxEnqueue(canTxMessage);	/*投入队列准备发送*/
								break;
			case 0xc3: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CTS<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//数据帧
								canTxMessage.IDE = CAN_ID_EXT;			//扩展帧
								canTxMessage.DLC = 8;							//数据长度
								canTxMessage.Data[0] = pgnValue.SPN2823[0];			/*BCD时间*/
								canTxMessage.Data[1] = pgnValue.SPN2823[1];	
								canTxMessage.Data[2] = pgnValue.SPN2823[2]; 
								canTxMessage.Data[3] = pgnValue.SPN2823[3];
								canTxMessage.Data[4] = pgnValue.SPN2823[4];
								canTxMessage.Data[5] = pgnValue.SPN2823[5]; 
								canTxMessage.Data[6] = pgnValue.SPN2823[6];
								
								CanQueueTxEnqueue(canTxMessage);	/*投入队列准备发送*/
								break;
			case 0xc4: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CML<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//数据帧
								canTxMessage.IDE = CAN_ID_EXT;			//扩展帧
								canTxMessage.DLC = 8;							//数据长度
								canTxMessage.Data[0] = pgnValue.SPN2824;	
								canTxMessage.Data[1] = pgnValue.SPN2824 >> 8;		
								canTxMessage.Data[2] = pgnValue.SPN2825; 
								canTxMessage.Data[3] = pgnValue.SPN2825 >> 8;
								canTxMessage.Data[4] = pgnValue.SPN2826;
								canTxMessage.Data[5] = pgnValue.SPN2826 >> 8; 
								canTxMessage.Data[6] = pgnValue.SPN2827;
								canTxMessage.Data[7] = pgnValue.SPN2827 >> 8;
								
								CanQueueTxEnqueue(canTxMessage);	/*投入队列准备发送*/
								break;
			case 0xc5: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CRO<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//数据帧
								canTxMessage.IDE = CAN_ID_EXT;			//扩展帧
								canTxMessage.DLC = 8;							//数据长度
								canTxMessage.Data[0] = pgnValue.SPN2830;
								
								CanQueueTxEnqueue(canTxMessage);	/*投入队列准备发送*/
								
								/*CRO一旦发送 就开始等待BCL BCS*/
								messageTimeTableSet(0);
								messageTimeTableSet(1);
			
								break;
			case 0xc6: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CCS<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//数据帧
								canTxMessage.IDE = CAN_ID_EXT;			//扩展帧
								canTxMessage.DLC = 8;							//数据长度
								canTxMessage.Data[0] = pgnValue.SPN3081;
								canTxMessage.Data[1] = pgnValue.SPN3081 >> 8;
								canTxMessage.Data[2] = pgnValue.SPN3082;
								canTxMessage.Data[3] = pgnValue.SPN3082 >> 8;
								canTxMessage.Data[4] = pgnValue.SPN3083;
								canTxMessage.Data[5] = pgnValue.SPN3083 >> 8; 
								canTxMessage.Data[6] = pgnValue.SPN3929 & 0x03;
								
								CanQueueTxEnqueue(canTxMessage);	/*投入队列准备发送*/
			
								/*CRO一旦发送 就开始等待BCL BCS BSM 这破坏了程序的耦合性 高了*/
								messageTimeTableSet(0);
								messageTimeTableSet(1);
								messageTimeTableSet(2);
								
								break;
			case 0xc7: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CST<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//数据帧
								canTxMessage.IDE = CAN_ID_EXT;			//扩展帧
								canTxMessage.DLC = 8;							//数据长度
								canTxMessage.Data[0] = pgnValue.SPN3521;
								canTxMessage.Data[1] = pgnValue.SPN3522;
								canTxMessage.Data[2] = pgnValue.SPN3522 >> 8;
								canTxMessage.Data[3] = pgnValue.SPN3523;
								
								CanQueueTxEnqueue(canTxMessage);	/*投入队列准备发送*/
								break;
			case 0xc8: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CSD<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//数据帧
								canTxMessage.IDE = CAN_ID_EXT;			//扩展帧
								canTxMessage.DLC = 8;							//数据长度
								canTxMessage.Data[0] = pgnValue.SPN3611;
								canTxMessage.Data[1] = pgnValue.SPN3611 >> 8;
								canTxMessage.Data[2] = pgnValue.SPN3612;
								canTxMessage.Data[3] = pgnValue.SPN3612 >> 8;
								canTxMessage.Data[4] = pgnValue.SPN3613;
								canTxMessage.Data[5] = pgnValue.SPN3613 >> 8; 
								canTxMessage.Data[6] = pgnValue.SPN3613 >> 16;
								canTxMessage.Data[7] = pgnValue.SPN3613 >> 24;
								
								CanQueueTxEnqueue(canTxMessage);	/*投入队列准备发送*/
								break;
			case 0xc9: //canTxMessage.StdId = 0x321;
								canTxMessage.ExtId = (6<<26)|(0<<25)|(0<<24)|(CEM<<16)|(244<<8)|(86);
								canTxMessage.RTR = CAN_RTR_DATA;		//数据帧
								canTxMessage.IDE = CAN_ID_EXT;			//扩展帧
								canTxMessage.DLC = 8;							//数据长度
								canTxMessage.Data[0] = pgnValue.SPN3921;
								canTxMessage.Data[1] = pgnValue.SPN3922 & 0x03;
								canTxMessage.Data[1] |= (pgnValue.SPN3923 << 2) & 0x0c;
								canTxMessage.Data[2] = pgnValue.SPN3924 & 0x03;
								canTxMessage.Data[2] |= (pgnValue.SPN3925 << 2) & 0x0c;
								canTxMessage.Data[2] |= (pgnValue.SPN3926 << 4) & 0x30;
								canTxMessage.Data[3] = pgnValue.SPN3927 >> 8;
								
								CanQueueTxEnqueue(canTxMessage);	/*投入队列准备发送*/
								break;
			default:break;
		}
}
