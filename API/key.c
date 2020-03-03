#include "stm32f10x.h"
#include "key.h"
#include "led.h"


static void Delay(u32 i)
{
	for(;i>0;i--);
}

void GPIO_KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	//按键都是输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
}

u8 Get_Key(void)
{
	if(!KEY1 || KEY4)
	{
		Delay(50000);
		if(!KEY1)
		{
			return 1;
		}
		if(KEY4)
		{
			return 4;
		}
	}
	return 0;
}
