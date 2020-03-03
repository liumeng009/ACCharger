#ifndef UART4_485_H_
#define UART4_485_H_

#include "stm32f10x.h"
#include "message.h"
void iDM100ECommunication(struct message data);
float * GetVoltage(void);
float * GetCurrent(void);
float * GetW(void);
#endif
