/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MESSAGE_H
#define __MESSAGE_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported macro ------------------------------------------------------------*/
#define QUEUE_SIZE  50	//消息队列的大小
/* Exported types ------------------------------------------------------------*/
struct message		//传递的消息通用结构
{
	uint8_t sourceAddress;			//源模块地址
	uint8_t destinationAddress;    //目的模块地址
	uint8_t cmd;					//命令
	uint8_t parameter_1;			//4个参数 可以是温度 可以是mac
	uint8_t parameter_2;
	uint8_t parameter_3;
	uint8_t parameter_4;
};
struct queue		//基于消息通用结构的顺序存储循环队列
{
	struct message message_list[QUEUE_SIZE];
	uint8_t front;
	uint8_t rear;
	uint8_t size;
};
/* Exported constants --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

void queueInit(void);
uint8_t queueEnqueue(uint8_t queueSwitch ,struct message data);
uint8_t queueDequeue(uint8_t queueSwitch ,struct message * data_pointer);
extern void messagePost(struct message data_come);	
extern void messagePostIt(struct message data_come);
void sysMonitor(void);
#endif /* __SYSTICK_H */
