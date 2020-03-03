/**
  ******************************************************************************
  * @file    SPI/Simplex_Interrupt/platform_config.h 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Evaluation board specific configuration file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_H
#define __SPI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/


/*≤…”√µƒ≈‰÷√*/
  #define SPIz                    SPI2
  #define SPIz_CLK                RCC_APB1Periph_SPI2
  #define SPIz_GPIO               GPIOB
  #define SPIz_GPIO_CLK           RCC_APB2Periph_GPIOB
  #define SPIz_PIN_SCK            GPIO_Pin_13
  #define SPIz_PIN_MISO           GPIO_Pin_14
  #define SPIz_PIN_MOSI           GPIO_Pin_15
	#define SPIz_GPIO_CS		        RCC_APB2Periph_GPIOG	
	#define SPIz_PIN_CS		          GPIO_Pin_14	
  #define SPIz_IRQn               SPI2_IRQn
          

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SpiInit(void);
uint16_t SPIByte(SPI_TypeDef* SPIx,uint8_t byte);
#endif /* __SPI_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
