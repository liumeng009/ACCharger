#include "stm32f10x.h"
#include "systick.h"
#include "gpio.h"
#include "table.h"
#include "mvc.h"

/****************************************************
函数名：GPIOInit(void)
形参：无
返回值：无
函数功能：初始化系统的GPIO 但是自功能自己负责初始化！
****************************************************/
void GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//对GPIOA GPIOB GPIOE口的时钟打开
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);
	//把IO口配置为输出模式
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 |GPIO_Pin_4;
//	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	/*系统监视灯*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/*CP 的电平 PWM切换电路 PE3*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	//GPIO_SetBits(GPIOE,GPIO_Pin_3);
	
	/*绿色指示灯 PE2*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	/*黄色指示灯 PA0*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/*红色指示灯 PA1*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/*急停开关检测 PE6*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	/*继电器开关 PE4为低*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOE,GPIO_Pin_4);
	
	/*继电器开关 PE3为低*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOE,GPIO_Pin_3);
}

/****************************************************
函数名：LedController(void)
形参：无
返回值：无
函数功能：
****************************************************/
#define STATE_WAIT					0
#define STATE_UP						1
#define STATE_DOWN					2

static uint8_t ledAppState = STATE_WAIT;    // State tracking variable

void LedTask()
{
		switch(ledAppState)
		{
			case STATE_WAIT:
				vLedTimerSetValue(0);
				ledAppState = STATE_UP;
				break;
			case STATE_UP:
				if(vLedTimerGetValue()>500)
				{
						//绿灯处理 
						if(GetSuperState() == 1 || GetSuperState() == 4)	//正在准备或开始充电
							GPIO_SetBits(GPIOE,GPIO_Pin_2);
						else
							GPIO_ResetBits(GPIOE,GPIO_Pin_2);
						/*
						if(GPIO_ReadOutputDataBit(GPIOE,GPIO_Pin_2) == 0)
							GPIO_SetBits(GPIOE,GPIO_Pin_2);
						else
							GPIO_ResetBits(GPIOE,GPIO_Pin_2);*/
						
						//黄灯处理
						if(GetSuperState() == 3)	//正在准备或开始充电
							GPIO_SetBits(GPIOA,GPIO_Pin_0);
						else
							GPIO_ResetBits(GPIOA,GPIO_Pin_0);
						
						ledAppState = STATE_WAIT;
				}
				break;
	//		case STATE_DOWN:
			default:break;
		}
}

/****************************************************
函数名：急停开关检测 如果按下 就恢复状态机到初始状态 PE6 每500ms执行一次
形参：无
返回值：无
函数功能：
****************************************************/
void EmergencyStop()
{
		if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6) == 0)
		{
				MvcStateMachineOpen(); //打开
			
				//红灯处理
				GPIO_ResetBits(GPIOA,GPIO_Pin_1);
		}
		else
		{
				MvcTableInit();	//清理
				tableInit();
			
				//红灯处理
				GPIO_SetBits(GPIOA,GPIO_Pin_1);

		}
}