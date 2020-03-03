/**
  ******************************************************************************
  * @file    ADC/3ADCs_DMA/main.c 
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
#include "adc.h"
#include "systick.h"
#include "usart.h"
/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup ADC_3ADCs_DMA
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define ADC2_DR_Address    ((uint32_t)0x4001284C)
#define ADC3_DR_Address    ((uint32_t)0x40013C4C)

/* Private macro -------------------------------------------------------------*/
#define MOV_AVG_BUFFER_SIZE 20 

/* Private variables ---------------------------------------------------------*/
ADC_InitTypeDef ADC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
__IO uint16_t ADC1ConvertedValue = 0, ADC3ConvertedValue = 0;
__IO uint16_t ADC2ConvertedValue = 0;
__IO uint16_t ADC1AvgValue = 0;
__IO uint16_t ADC3AvgValue = 0;
__IO uint16_t ADC2AvgValue = 0;
uint16_t adc1Buffer[MOV_AVG_BUFFER_SIZE];
uint16_t adc2Buffer[MOV_AVG_BUFFER_SIZE];
uint16_t adc3Buffer[MOV_AVG_BUFFER_SIZE];

struct message adcMessage;	//滴答用的消息实例
/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration_Adc(void);
void GPIO_Configuration_Adc(void);
void NVIC_Configuration_Adc(void);
uint16_t adc1Value(void);	//读取ADC1的转换结果
uint16_t adc2Value(void);	//读取ADC2的转换结果
/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
void adcInit(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* System clocks configuration ---------------------------------------------*/
  RCC_Configuration_Adc();

  /* NVIC configuration ------------------------------------------------------*/
  NVIC_Configuration_Adc();

  /* GPIO configuration ------------------------------------------------------*/
  GPIO_Configuration_Adc();

  /* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC1ConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);

  /* DMA2 channel5 configuration ----------------------------------------------*/
  DMA_DeInit(DMA2_Channel5);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC3_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC3ConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA2_Channel5, &DMA_InitStructure);  
  /* Enable DMA2 channel5 */
  DMA_Cmd(DMA2_Channel5, ENABLE);

  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
  /* ADC1 regular channels configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_28Cycles5);    
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

  /* ADC2 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;	//本来DISABLE 错误
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC2, &ADC_InitStructure);
  /* ADC2 regular channels configuration */ 
  ADC_RegularChannelConfig(ADC2, ADC_Channel_13, 1, ADC_SampleTime_13Cycles5);
  /* Enable ADC2 EOC interrupt */
  ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);

  /* ADC3 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC3, &ADC_InitStructure);
  /* ADC3 regular channel13 configuration */ 
  ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_13Cycles5);
  /* Enable ADC3 DMA */
  ADC_DMACmd(ADC3, ENABLE); 

  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));

  /* Enable ADC2 */
  ADC_Cmd(ADC2, ENABLE);

  /* Enable ADC2 reset calibration register */   
  ADC_ResetCalibration(ADC2);
  /* Check the end of ADC2 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC2));

  /* Start ADC2 calibration */
  ADC_StartCalibration(ADC2);
  /* Check the end of ADC2 calibration */
  while(ADC_GetCalibrationStatus(ADC2));


  /* Enable ADC3 */
  ADC_Cmd(ADC3, ENABLE);

  /* Enable ADC3 reset calibration register */   
	ADC_ResetCalibration(ADC3);
  /* Check the end of ADC3 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC3));

  /* Start ADC3 calibration */
  ADC_StartCalibration(ADC3);
  /* Check the end of ADC3 calibration */
  while(ADC_GetCalibrationStatus(ADC3));

  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  /* Start ADC2 Software Conversion */ 
 // ADC_SoftwareStartConvCmd(ADC2, ENABLE);
  /* Start ADC3 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC3, ENABLE);

}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_Configuration_Adc(void)
{
  /* ADCCLK = PCLK2/4 36/2 = 18MHz*/
  RCC_ADCCLKConfig(RCC_PCLK2_Div4); 
    
  /* Enable peripheral clocks ------------------------------------------------*/
  /* Enable DMA1 and DMA2 clocks */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_DMA2, ENABLE);

  /* Enable ADC1, ADC2, ADC3 and GPIOC clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 |
                         RCC_APB2Periph_ADC3 | RCC_APB2Periph_GPIOC, ENABLE);
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void GPIO_Configuration_Adc(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Configure PC.01, PC.02 and PC.03 (ADC Channel12, ADC Channel13 and 
     ADC Channel14) as analog inputs */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
  * @brief  Configures Vector Table base location.
  * @param  None
  * @retval None
  */
void NVIC_Configuration_Adc(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure and enable ADC interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  adc采样是不停进行的，AVGTask通过滑动平均值算法 处理采样值 放入经过
	平均化处理的变量内
  * @param  None
  * @retval None
  */
#define STATE_WAIT					0
#define STATE_ADC						1
static uint8_t aAppState = STATE_WAIT;    // State tracking variable
static uint8_t adc1Counter = 0;
static uint8_t adc3Counter = 0;
void AVGTask()
{

		switch(aAppState)
		{
			case STATE_WAIT:
				vAdcTimerSetValue(0);
				aAppState = STATE_ADC;
				break;
			case STATE_ADC:
				if(vAdcTimerGetValue() > 20)
				{
						AdcFilter();
						aAppState = STATE_WAIT;
				}
				break;
			default:break;
		}
}
void AdcFilter(void)
{
   uint8_t   count;
   uint32_t  sum;
	 sum = 0;
   adc1Buffer[adc1Counter++] = ADC1ConvertedValue;
   if(adc1Counter == MOV_AVG_BUFFER_SIZE)   
		 adc1Counter = 0;
   for(count=0;count<MOV_AVG_BUFFER_SIZE;count++)
      sum += adc1Buffer[count];
   ADC1AvgValue = (uint16_t)(sum/MOV_AVG_BUFFER_SIZE);
	 
	 sum = 0;
	 adc3Buffer[adc3Counter++] = ADC3ConvertedValue;
   if(adc3Counter == MOV_AVG_BUFFER_SIZE)   
		 adc3Counter = 0;
   for(count=0;count<MOV_AVG_BUFFER_SIZE;count++)
      sum += adc3Buffer[count];
   ADC3AvgValue = (uint16_t)(sum/MOV_AVG_BUFFER_SIZE);
}
uint16_t adc1Value(void)
{
	return ADC1AvgValue;
}

uint16_t adc2Value(void)
{
	return ADC2AvgValue;
}

uint16_t adc3Value(void)
{
	return ADC3AvgValue;
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

  /* Infinite loop */
  while (1)
  {
  }
}

#endif

/**
  * @}
  */ 

/**
  * @brief  This function handles ADC1 and ADC2 global interrupts requests.
  * @param  None
  * @retval None
  */
void ADC1_2_IRQHandler(void)
{
  /* Get injected channel13 converted value */
  ADC_ClearITPendingBit(ADC2, ADC_IT_EOC);
  ADC2ConvertedValue = ADC_GetConversionValue(ADC2);
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
