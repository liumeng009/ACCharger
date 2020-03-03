#ifndef __IDM100E_H_
#define __IDM100E_H_

#include "stm32f10x.h"
#include "message.h"
void iDM100ECommunication(struct message data);
float * GetVoltage(void);
float * GetCurrent(void);
float * GetW(void);

float GetVoltageValue(void);
float GetCurrentValue(void);
float GetWValue(void);

#endif
