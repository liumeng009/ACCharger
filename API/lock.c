/**
  ******************************************************************************
  * @file    lock.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *	此函数处理自锁装置，自锁装置只有两种状态 开和关，通过正转和反转实现
		由于电机正反转需要一定时间，因此操作被放在操作队列中等待执行，不支持
		中断操作
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "lock.h"
#include "usart.h"
#include "string.h"
#include "systick.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define CMD_QUEUE_SIZE  10
/* Private variables ---------------------------------------------------------*/
uint8_t lockStatus; 			/*自锁装置目前的状态 0：打开 1：关闭*/
struct 
{
	uint8_t cmdQueue[CMD_QUEUE_SIZE];
	uint8_t front;
	uint8_t rear;
	uint8_t size;
}lockCmdQueue;

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  此函数插入消息队列 不考虑会不会被中断影响
  * @param  输入 切换标志位 消息的值 
  * @retval 输出 0 成功 1 消息队列已满 
	* @usage  LockSet(LOCK);  LockSet(UNLOCK);
  */
uint8_t LockSet(uint8_t cmd)	
{
		uint8_t temp;
		if((((lockCmdQueue.rear) + 1)%CMD_QUEUE_SIZE) == lockCmdQueue.front)		//满了
		{
				return 1;
		}
		else
		{
				temp = lockCmdQueue.rear;
				lockCmdQueue.cmdQueue[temp] = cmd;
				lockCmdQueue.rear = (temp + 1) % CMD_QUEUE_SIZE;
				return 0;
		}
}
/**
  * @brief  此函数提取消息队列 不考虑会不会被中断影响
  * @param  输入 消息队列的指针 消息的指针（调用程序维护） 
  * @retval 输出 0 成功 1 消息队列已空
  */
uint8_t cmdDequeue(uint8_t * data_pointer)
{
		uint8_t temp;
		if(lockCmdQueue.front == lockCmdQueue.rear)							//空了
		{
				return 1;
		}
		else
		{
				temp = lockCmdQueue.front;
				*data_pointer = lockCmdQueue.cmdQueue[temp];
				lockCmdQueue.front = (temp + 1)%CMD_QUEUE_SIZE;
				return 0;
		}
}
/**
  * @brief  GPIO_Lock_Configuration 配置相关引脚
  * @param  
  * @retval 
  */
void GPIO_Lock_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//对GPIOA GPIOB GPIOE口的时钟打开
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);

	/*正转反转引脚*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	/*mos管引脚*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_9);
	
	/*监测引脚*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
}

/**
  * @brief LockInit 初始化状态
  * @param  
  * @retval 
  */
void LockInit(void)
{
		GPIO_Lock_Configuration();
		memset(&lockCmdQueue,0,sizeof(lockCmdQueue));	/*命令队列初始化*/
		lockStatus = 0;																/*锁状态初始化*/
}

/**
  * @brief GetLockStatus 获得当前自锁开关的开闭状态 0：开 1：闭
  * @param  
  * @retval 
  */
uint8_t GetLockStatus(void)
{				
		return lockStatus;
}

/**
  * @brief LockTask读取命令，执行关于自锁装置的动作
  * @param  
  * @retval 
  */
#define STATE_WAIT					0
#define STATE_READ_CMD			1
#define STATE_CMD_EXE				2
#define STATE_READ_STATUS		3
#define STATE_CMDX_EXE			4


static uint8_t lockAppState = STATE_WAIT;    // State tracking variable

void LockTask(void)
{
		static uint8_t cmd;

    switch(lockAppState)
    {
				case STATE_WAIT:
						lockAppState = STATE_READ_CMD;
						break;
				case STATE_READ_CMD:
						if(cmdDequeue(&cmd) == 0)			/*有命令*/
						{
								/*此时状态和命令不相符*/
								if(lockStatus != cmd)
								{
										lockAppState = STATE_CMD_EXE;
								}
						}
						break;
				case STATE_CMD_EXE:
						if(cmd == 0)
						{
								GPIO_SetBits(GPIOB,GPIO_Pin_8);
								GPIO_SetBits(GPIOB,GPIO_Pin_9);
						}
						else
						{
								GPIO_ResetBits(GPIOB,GPIO_Pin_8);
								GPIO_SetBits(GPIOB,GPIO_Pin_9);
						}
						lockAppState = STATE_READ_STATUS;
						break;
				case STATE_READ_STATUS:
						/*如果到达临界点了*/
						if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13) == cmd)
						{
								vLockTimerSetValue(0);				/*开始计时*/
								lockAppState = STATE_CMDX_EXE;
						}
						break;
				case STATE_CMDX_EXE:
						if(cmd == 0)
						{
								if(vLockTimerGetValue() > 500)
								{
										GPIO_ResetBits(GPIOB,GPIO_Pin_9);
										lockStatus = cmd;
										lockAppState = STATE_WAIT;
								}
						}
						else
						{
								if(vLockTimerGetValue() > 30)
								{
										GPIO_ResetBits(GPIOB,GPIO_Pin_9);
										lockStatus = cmd;
										lockAppState = STATE_WAIT;
								}
						}
						break;
				default:break;
		}
}
