#ifndef _KEY_H_
#define _KEY_H_
#include "stm32f10x.h"
//uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
#define KEY1 GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)
#define KEY4 GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)

void GPIO_KEY_Init(void);

u8 Get_Key(void);

#endif
