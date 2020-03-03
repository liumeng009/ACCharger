#ifndef __SPIBYSOFT_H_
#define __SPIBYSOFT_H_

#include "stm32f10x.h"

#define RST  1<<8 			   				//PA8---RST---RST
		   
#define  CLR_RC522RST  //GPIOA->ODR=(GPIOA->ODR&~RST)|(0 ? RST:0)
#define  SET_RC522RST  //GPIOA->ODR=(GPIOA->ODR&~RST)|(1 ? RST:0)

#define DOUT GPIOC->IDR&(1<<0x06)	// PC6---DOUT---MISO			 
#define TRC_DIN  (1<<7)						// PC7---DIN---MOSI
#define TRC_CLK  (1<<8) 					// PC8---CLK---SCK
#define TRC_CS   (1<<9) 					// PC9---CS---SDA		    
#define TRC_DIN_SET(x) 	GPIOC->ODR=(GPIOC->ODR&~TRC_DIN)|(x ? TRC_DIN:0)
#define TRC_CLK_SET(x) 	GPIOC->ODR=(GPIOC->ODR&~TRC_CLK)|(x ? TRC_CLK:0)													    
#define TRC_CS_SET(x)  	GPIOC->ODR=(GPIOC->ODR&~TRC_CS)|(x ? TRC_CS:0) 

void SpiBySoftInit();

void GPIO_RC522_Configuration(void);
void NVIC_RC522_Configuration(void);

void SPIWriteByte(uint8_t num);
uint8_t SPIReadByte(void);						
uint8_t ReadRawRC(uint8_t Address);			
void WriteRawRC(uint8_t  Address, uint8_t  value);						
void SetBitMask(uint8_t reg, uint8_t mask);
void ClearBitMask(uint8_t reg, uint8_t  mask);
#endif