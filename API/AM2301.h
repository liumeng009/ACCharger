#ifndef __AM2301_H_
#define __AM2301_H_

#include "stm32f10x.h"

void AM2301_IN_Init(void);
void AM2301_OUT_Init(void);
void AM2301Data(void);
void AM2301Task(void);
float * GetTemperature(void);
float * GetHumidity(void);
#endif


