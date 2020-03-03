#include "AM2301.h"
#include "time.h"
#include "usart.h"

uint8_t data[40];
s16 temperatureValue;
uint16_t crcTemperatureValue;
uint16_t humidityValue;
uint16_t crcHumidityValue;
uint8_t crcValue;
uint8_t crcCaclulate;

/*这两个值是最终结果 可以写两个读取函数供外部访问*/
float temperature;
float humidity;

/*单总线对时许要求比较高 消息机制可能无法满足
	因此在消息的最后引入协同多任务 协同多任务在
	消息机制的间隙执行
*/
#define STATE_WAIT					0
#define STATE_OUTPUT_LOW		1
#define STATE_OUTPUT_HIGH		2
#define STATE_INPUT_LOW			3
#define STATE_INPUT_HIGH		4
#define STATE_DATA_LOW			5
#define STATE_DATA_HIGH			6
#define STATE_DATA_TIME			7
#define STATE_INPUT_DELAY   8
#define STATE_DATA_DELAY_1  9
#define STATE_DATA_DELAY_2  10
static uint8_t gAppState = STATE_WAIT;    // State tracking variable


/****************************************************
@函数名：AM2301Task 此函数用了占用了timer6用来1us延时
					但timer6还是会产生中断 65ms一次
@输入：无
@输出：无
****************************************************/
void AM2301Task(void)
{
		static uint16_t timeCounter;
		static uint16_t counter;
		static uint8_t	i;
    switch (gAppState)
    {
				case STATE_WAIT:
						i = 0;
						timeCounter = 0;
						counter = 0;
						Tim6SetCounterValue(0);									/*开始定时*/
            gAppState = STATE_OUTPUT_LOW;
            break;
				case STATE_OUTPUT_LOW:
						if(Tim6GetCounterValue() > 64000)				/*等待60ms 时间到了*/
						{
								i++;
								if(i == 20)
								{
									i = 0;
									AM2301_OUT_Init();									/*引脚配置为输出*/
									GPIO_ResetBits(GPIOG,GPIO_Pin_11);	/*拉低*/
									Tim6SetCounterValue(0);							/*开始定时*/
									gAppState = STATE_OUTPUT_HIGH;
								}
								else
									Tim6SetCounterValue(0);
						}
            break;
        case STATE_OUTPUT_HIGH:											
						if(Tim6GetCounterValue() > 25000)				/*低电平持续25ms 时间到了*/
						{
								GPIO_SetBits(GPIOG,GPIO_Pin_11);		/*拉高 代表开始信号结束*/
								Tim6SetCounterValue(0);							/*开始定时*/
								gAppState = STATE_INPUT_LOW;
						}
            break;
        case STATE_INPUT_LOW:
						if(Tim6GetCounterValue() > 30)			/*高电平持续30us*/
						{
								AM2301_IN_Init();								/*变为输入*/
								Tim6SetCounterValue(0);							/*开始定时*/
								gAppState = STATE_INPUT_DELAY;
						}
            break;
				case STATE_INPUT_DELAY:
						if(Tim6GetCounterValue() > 10)			/*延时10us*/
						{
								Tim6SetCounterValue(0);
								gAppState = STATE_INPUT_HIGH;
						}
				case STATE_INPUT_HIGH:
						if(Tim6GetCounterValue() < 60000)
						{
							if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_11)==1)	/*等待为高电平*/
							{
									gAppState = STATE_DATA_LOW;
									Tim6SetCounterValue(0);
							}
						}
						else
							gAppState = STATE_WAIT;
            break;
				case STATE_DATA_LOW:
						if(Tim6GetCounterValue() < 60000)
						{
							if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_11)==0)	/*等待为低电平*/
							{
									gAppState = STATE_DATA_HIGH;
									Tim6SetCounterValue(0);
							}
						}
						else
							gAppState = STATE_WAIT;
            break;
				case STATE_DATA_HIGH:
						if(Tim6GetCounterValue() < 60000)
						{
							if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_11)==1)	/*等待变为高 代表数据开始*/
							{
									Tim6SetCounterValue(0);		/*变为高以后开始计时*/
									gAppState = STATE_DATA_TIME;
							}
						}
						else
							gAppState = STATE_WAIT;
            break;
				case STATE_DATA_TIME:
						if(Tim6GetCounterValue() < 60000)
						{
							if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_11)==0)	/*等待变为低 代表数据结束*/
							{
									timeCounter = Tim6GetCounterValue();
								
									/*根据counter的值 判断是0还是1 填入到数组中*/
									
									if(timeCounter > 40)
									{
											data[counter] = 1;
									}
									if(timeCounter < 40)
									{
											data[counter] = 0;
									}
									
									//data[counter] = timeCounter;
									counter++;	/*收到的位数加一*/
									
									/*状态有两种选择*/
									if(counter == 40)
									{
											gAppState = STATE_WAIT;	/*已经采集完成了*/
											AM2301Data();/*每次采集完 立刻处理*/
									}
									else
											gAppState = STATE_DATA_HIGH;								
							}
						}
						else
						{
							gAppState = STATE_WAIT;
						}
						break;
    }
} 

/***********************************************
@函数名: AM2301_IN_Init 初始化单总线的输入配置
@输入：无
@输出：无
************************************************/
void AM2301_IN_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStruct;

	/*打开时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG ,ENABLE);
	
	/*初始化管脚--PG11---浮空输入*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOG,&GPIO_InitStruct);
	
}

void AM2301_OUT_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStruct;

	/*打开时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG ,ENABLE);
	
	/*初始化管脚--PG11--复用推挽输出*/
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStruct);
	
}

/*整个读取并转换*/
void AM2301Data(void)
{
	uint8_t i;
	temperatureValue = 0;
	humidityValue = 0;

	for(i=0;i<16;i++)
	{
		crcHumidityValue<<= 1;
		crcHumidityValue+=data[i];
	}
	for(i=16;i<32;i++)
	{
		crcTemperatureValue <<= 1;
		crcTemperatureValue+=data[i];
	}
	for(i=32;i<40;i++)			/*8bit校验位*/
	{
		crcValue <<= 1;
		crcValue+=data[i];
	}
	crcHumidityValue = crcHumidityValue + (crcHumidityValue>>8);
	crcTemperatureValue = crcTemperatureValue + (crcTemperatureValue>>8);
	crcCaclulate = crcHumidityValue + crcTemperatureValue;
	
	if(crcCaclulate != crcValue)			/*如果错误 返回 不进行任何数据处理*/
			return;
	
	for(i=0;i<16;i++)
	{
		humidityValue<<= 1;
		humidityValue+=data[i];
	}
	
	humidity=(float)(humidityValue)/10;
	
	//温度值
	for(i=17;i<32;i++)
	{
		temperatureValue <<= 1;
		temperatureValue+=data[i];
	}
	if(data[16])
		temperatureValue=0-temperatureValue;
	temperature=(float)(temperatureValue)/10;
	
	//usart1SendByte(temperature);
	//usart1SendByte(humidity);
}

/**
  * @brief  读取当前温度
  * @param  None
  * @retval None
  */
float * GetTemperature(void)
{
		return &temperature;
}
/**
  * @brief  读取当前湿度指针
  * @param  None
  * @retval None
  */
float * GetHumidity(void)
{
		return &humidity;
}
