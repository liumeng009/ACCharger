/**
  ******************************************************************************
  * @file    TIM/TimeBase/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "time.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup TIM_TimeBase
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStruct;

uint16_t PrescalerValue = 0;

/* Private function prototypes -----------------------------------------------*/
void RCC_Time_Configuration(void);
void NVIC_Time_Configuration(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void TimeInit(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */ 
	GPIO_InitTypeDef GPIO_InitStruct;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO ,ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode =GPIO_Mode_AF_PP; 
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
  /* System Clocks Configuration */
  RCC_Time_Configuration();

  /* NVIC Configuration */
  NVIC_Time_Configuration();

  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) (72000000 / 1000000) - 1;

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 0xffff;
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	/*从0曾加*/

  TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	/* Time3 pwm configuration */
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
	TIM_TimeBaseStructure.TIM_Period = 1000-1;//设置在下一个更新事件装入活动的自动重装载寄存器周期的值 (._Period这个即是设置TIMx_ARR的值，即用来设置pwm的频率)
	TIM_TimeBaseStructure.TIM_Prescaler =72-1;//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	
	//配置输出控制模式为PWM模式
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;//选择定时器模式:TIM脉冲宽度调制模式1
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStruct.TIM_Pulse = 2;
	TIM_OC1Init(TIM3,&TIM_OCInitStruct);
	
	TIM_ARRPreloadConfig(TIM3,ENABLE);//使能TIMx在ARR上的预装载寄存器
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable); //使能TIMx在CCR1上的预装载寄存器
	
  /* TIM IT enable */
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);	
	

  /* TIM6 enable counter */
  TIM_Cmd(TIM6, ENABLE);
	TIM_Cmd(TIM3 ,ENABLE);
}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_Time_Configuration(void)
{
  /* PCLK1 = HCLK/4 */
  //RCC_PCLK1Config(RCC_HCLK_Div4);

  /* TIM6 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
	
	/* TIM3 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
}

/**
  * @brief  Configure the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Time_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the TIM6 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the TIM3 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

uint16_t Tim6GetCounterValue()
{
		return TIM_GetCounter(TIM6);
}
void Tim6SetCounterValue(uint16_t value)
{
		TIM_SetCounter(TIM6,value);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  while (1)
  {}
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
/**
  * @brief  This function handles TIM6 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM6_IRQHandler(void)
{
		/*每65.535ms中断一次*/
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3 ,TIM_IT_Update)==SET)
	{	
			TIM_ClearITPendingBit(TIM3 ,TIM_IT_Update);	
			TIM_SetCompare1(TIM3,530); 								/*占空比 与period 1000的比值*/
	}
}