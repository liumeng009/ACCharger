/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H
#define __RTC_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "message.h"
/* Exported macro ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
typedef struct 
{
		uint32_t year;
		uint32_t month;
		uint32_t day;
		uint32_t hour;
		uint32_t minute;
		uint32_t second;
}T_STRUCT;

int RtcInit(void);
void TestRtc(void);
void TimeUpdate(void);
T_STRUCT GetTime(void);
uint32_t GetTimeSecond(void);
u32 Time_Regulate(u32 Tmp_Year,\
									u32 Tmp_Month,\
									u32 Tmp_Date,\
									u32 Tmp_HH,\
									u32 Tmp_MM,\
									u32 Tmp_SS); 
#endif
