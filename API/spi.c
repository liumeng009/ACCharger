/**
  ******************************************************************************
  * @file    SPI/FullDuplex_SoftNSS/main.c 
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
#include "spi.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup SPI_FullDuplex_SoftNSS
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SPI_InitTypeDef   SPI_InitStructure;

/* Private functions ---------------------------------------------------------*/
void RCC_SPI_Configuration(void);
void GPIO_SPI_Configuration(uint16_t SPIz_Mode);
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
void SpiInit(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* System clocks configuration ---------------------------------------------*/
  RCC_SPI_Configuration();

  /* GPIO configuration ------------------------------------------------------*/
  GPIO_SPI_Configuration(SPI_Mode_Master);
  
  /* SPIz Config -------------------------------------------------------------*/
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;	/*适当降低*/
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPIz, &SPI_InitStructure);

  /* Enable SPIz */
  SPI_Cmd(SPIz, ENABLE);
	
}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_SPI_Configuration(void)
{
  /* PCLK1 = HCLK/2 36MHz*/
  //RCC_PCLK2Config(RCC_HCLK_Div2); 

	/* Enable peripheral clocks --------------------------------------------------*/
  /* Enable SPIz Periph clock */
  RCC_APB2PeriphClockCmd(SPIz_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(SPIz_CLK, ENABLE);
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
}

/**
  * @brief  Configures the different SPIy and SPIz GPIO ports.               
  * @param  SPIz_Mode: Specifies the SPIz operating mode. 
  *            This parameter can be:
  *              -  SPIz_Mode_Master
  *              -  SPIz_Mode_Slave 
  * @retval None
  */
void GPIO_SPI_Configuration(uint16_t SPIz_Mode)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Configure SPIz pins: SCK, MISO and MOSI ---------------------------------*/
  GPIO_InitStructure.GPIO_Pin = SPIz_PIN_SCK | SPIz_PIN_MOSI;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  if(SPIz_Mode == SPI_Mode_Slave)
  {
    /* Configure SCK and MOSI pins as Input Floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  }
  else
  { 
    /* Configure SCK and MOSI pins as Alternate Function Push Pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  }
  GPIO_Init(SPIz_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPIz_PIN_MISO;
  if(SPIz_Mode == SPI_Mode_Slave)
  {
    /* Configure MISO pin as Alternate Function Push Pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  }
  else
  { /* Configure MISO pin as Input Floating  */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  }
  GPIO_Init(SPIz_GPIO, &GPIO_InitStructure);
	
	/*cs由SPI.C统一管理 否则会出现问题*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOG,&GPIO_InitStructure);
	/*默认情况是高*/
	GPIO_SetBits(GPIOG,GPIO_Pin_14);
	
	/*SPI总线上的其他 目前是W5500的cs*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	/*默认情况是高*/
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

/**
  * @brief  SPIByte发送一个字节数据的同时 返回一个字节数据 这是
	SPI最基本的操作。
  * @param  None
  * @retval None
  */
uint16_t SPIByte(SPI_TypeDef* SPIx, uint8_t byte)
{
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE)==RESET);
		SPI_I2S_SendData(SPIx,byte);
		while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE)==RESET);
		return SPI_I2S_ReceiveData(SPIx);                
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
  {}
}

#endif

/**
  * @}
  */ 

/**
  * @}
  */ 


/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

/**
  * @brief  This function handles SPI1 or SPI3 global interrupt request.
  * @param  None
  * @retval None
  */
void SPI1_IRQHandler(void)
{
//  if (SPI_I2S_GetITStatus(SPIy, SPI_I2S_IT_TXE) != RESET)
//  {
    /* Send SPIy data */
//    SPI_I2S_SendData(SPIy, SPIy_Buffer_Tx[TxIdx++]);

    /* Disable SPIy TXE interrupt */
//    if (TxIdx == BufferSize)
//    {
//      SPI_I2S_ITConfig(SPIy, SPI_I2S_IT_TXE, DISABLE);
//    }
//  }
}

/**
  * @brief  This function handles SPI2 global interrupt request.
  * @param  None
  * @retval None
  */
void SPI2_IRQHandler(void)
{
  /* Store SPIz received data */
  //SPIz_Buffer_Rx[RxIdx++] = SPI_I2S_ReceiveData(SPIz);
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
