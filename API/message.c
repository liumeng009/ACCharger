#include "message.h"
#include "usart.h"
struct queue messageQueueA;		//双缓冲队列机制
struct queue messageQueueB;		//


uint8_t queueSwitch;		//正在处理的消息系统 0：A 1：B

/**
  * @brief  此函数初始化消息队列
  * @param  输入 消息队列的指针 
  * @retval 输出 无
  */
void queueInit()
{
	messageQueueA.front = 0;
	messageQueueA.rear = 0;
	messageQueueB.front = 0;
	messageQueueB.rear = 0;
}
/**
  * @brief  此函数插入消息队列 不考虑会不会被中断影响
  * @param  输入 切换标志位 消息的值 
  * @retval 输出 0 成功 1 消息队列已满
  */
uint8_t queueEnqueue(uint8_t queueSwitch, struct message data)		//ok
{
	uint8_t temp;
	switch(queueSwitch)
	{
		case 0:
				if((((messageQueueA.rear) + 1)%QUEUE_SIZE) == messageQueueA.front)		//满了
				{
					return 1;
				}
				else
				{
					temp = messageQueueA.rear;
					messageQueueA.message_list[temp] = data;
					messageQueueA.rear = (temp + 1) % QUEUE_SIZE;
					return 0;
				}
				break;
		case 1:
				if((((messageQueueB.rear) + 1)%QUEUE_SIZE) == messageQueueB.front)		//满了
				{
					return 1;
				}
				else
				{
					temp = messageQueueB.rear;
					messageQueueB.message_list[temp] = data;
					messageQueueB.rear = (temp + 1) % QUEUE_SIZE;
					return 0;
				}
				break;
	}
}
/**
  * @brief  此函数提取消息队列 不考虑会不会被中断影响
  * @param  输入 消息队列的指针 消息的指针（调用程序维护） 
  * @retval 输出 0 成功 1 消息队列已空
  */
uint8_t queueDequeue(uint8_t queueSwitch,struct message * data_pointer)
{
	uint8_t temp;
	switch(queueSwitch)
	{
		case 0:
				if(messageQueueA.front == messageQueueA.rear)							//空了
				{
					return 1;
				}
				else
				{
					temp = messageQueueA.front;
					*data_pointer = messageQueueA.message_list[temp];
					messageQueueA.front = (temp + 1)%QUEUE_SIZE;
					return 0;
				}
				break;
		case 1:
				if(messageQueueB.front == messageQueueB.rear)							//空了
				{
					return 1;
				}
				else
				{
					temp = messageQueueB.front;
					*data_pointer = messageQueueB.message_list[temp];
					messageQueueB.front = (temp + 1)%QUEUE_SIZE;
					return 0;
				}
				break;
	}
}

/**
  * @brief  此函数发送消息 只供主程序中使用（中断不可用）
			由于双队列 发送时要根据正在提取的队列决定插入队列
  * @param  输入 消息值 
  * @retval 输出 无
  */
void messagePost(struct message data_come)		//发送消息
{
	switch(queueSwitch)							
	{
		case 0: queueEnqueue(1,data_come); //插入B
				break;
		case 1:	queueEnqueue(0,data_come); //插入A
				break;
	}
}
/**
  * @brief  此函数发送消息 只供中断用
			由于双队列 发送时要根据正在提取的队列决定插入队列
  * @param  输入 消息值 
  * @retval 输出 无
  */
void messagePostIt(struct message data_come)	//中断发送消息
{
	switch(queueSwitch)							
	{
		case 0: queueEnqueue(1,data_come);
				break;
		case 1:	queueEnqueue(0,data_come);
				break;
	}
}

/**
  * @brief  此函数监视消息队列 主要是每一次消息循环中 AB中消息的个数
						同时也检测实时任务系统中 每一秒被执行的时间 来看系统的利用率
						此函数放在while｛｝超级循环内
  * @param  输入 无
  * @retval 输出 无
  */
void sysMonitor()
{
		if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_5) == RESET)
			GPIO_SetBits(GPIOA,GPIO_Pin_5);
		else
			GPIO_ResetBits(GPIOA,GPIO_Pin_5);
		/*统计消息队列中到底有多少消息*/
}
