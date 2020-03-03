/**
  ******************************************************************************
  * @file    RTC/Calendar/main.c 
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
#include "usart.h"
#include "rtc.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup RTC_Calendar
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define RTCClockOutput_Enable  /* RTC Clock/64 is output on tamper pin(PC.13) */

/* Private macro -------------------------------------------------------------*/
#define  BKP_TIME_FLAG            BKP_DR1
#define  BKP_TIME_YEAR            BKP_DR2
#define  BKP_TIME_MONTH           BKP_DR3
#define  BKP_TIME_DAY             BKP_DR4
/* Private variables ---------------------------------------------------------*/
__IO uint32_t TimeDisplay = 0;

T_STRUCT  Real_Time;	/*实时时间值*/
u32 TimeVar;					/*实时时间值*/
/* Private function prototypes -----------------------------------------------*/
void RTC_Configuration(void);
void NVIC_RTC_Configuration(void);
void Time_Adjust(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int RtcInit(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     

  /* NVIC configuration */
  NVIC_RTC_Configuration();

  if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
  {
    /* Backup data register value is not correct or not yet programmed (when
       the first time the program is executed) */

    /* RTC Configuration */
    RTC_Configuration();

		Time_Regulate(2018,2,6,9,00,00);
		
    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
  }
  else
  {
    /* Check if the Power On Reset flag is set */
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
    {
				/*防死机*/
				//RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); 
				//PWR_BackupAccessCmd(ENABLE); 
    }
    /* Check if the Pin Reset flag is set */
    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
    {

    }

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); 
    PWR_BackupAccessCmd(ENABLE); 
		
    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();

    /* Enable the RTC Second */
    //RTC_ITConfig(RTC_IT_SEC, ENABLE);
		
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
  }

#ifdef RTCClockOutput_Enable
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Disable the Tamper Pin */
  BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper
                                 functionality must be disabled */

  /* Enable RTC Clock Output on Tamper Pin */
  BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
#endif

  /* Clear reset flags */
  RCC_ClearFlag();
	
	return 0;
}


/**
  * @brief  TestRtc是rtc测试代码 此函数最好每1s驱动一次 
  * @param  None
  * @retval None
  */
void TestRtc()
{
		TimeUpdate();
/*		usart1SendByte(Real_Time.year);
		usart1SendByte(Real_Time.month);
		usart1SendByte(Real_Time.day);
		usart1SendByte(Real_Time.hour);
		usart1SendByte(Real_Time.minute);
		usart1SendByte(Real_Time.second);*/
	
}

/**
  * @brief  GetTime 返回当前的rtc时间值 供外部调用的API
  * @param  None
  * @retval None
  */
T_STRUCT GetTime()
{
		return Real_Time;
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_RTC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure one bit for preemption priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  /* Enable the RTC Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Configures the RTC.
  * @param  None
  * @retval None
  */
void RTC_Configuration(void)
{
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();

  /* Enable LSE */
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {}

  /* Select LSE as RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC registers synchronization */
  RTC_WaitForSynchro();

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Enable the RTC Second */
  //RTC_ITConfig(RTC_IT_SEC, ENABLE);

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Set RTC prescaler: set RTC period to 1sec */
  RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}

/******************************************************************************* 
 * Function Name  : Time_Display 
 * Description    : Displays the current time. 
 * Input          : - TimeVar: RTC counter value. 
 * Output         : None 
 * Return         : None 
 *******************************************************************************/  
 #define SecsPerComYear  3153600//(365*3600*24)  
 #define SecsPerLeapYear 31622400//(366*3600*24)  
 #define SecsPerFourYear 126230400//((365*3600*24)*3+(366*3600*24))  
 #define SecsPerDay      (3600*24)  
   
const s32 Year_Secs_Accu[5]={0,  
                       31622400,  
                       63158400,  
                       94694400,  
                       126230400};  
   
const s32 Month_Secs_Accu_C[13] = { 0,  
                             2678400,  
                             5097600,  
                             7776000,  
                             10368000,  
                             13046400,  
                             15638400,  
                             18316800,  
                             20995200,  
                             23587200,  
                             26265600,  
                             28857600,  
                             31536000};  
const s32 Month_Secs_Accu_L[13] = {0,  
                             2678400,  
                             5184000,  
                             7862400,    
                            10454400,  
                             13132800,  
                             15724800,  
                             18403200,  
                             21081600,  
                             23673600,  
                             26352000,  
                             28944000,  
                             31622400};  
   
void TimeUpdate()  
{     
		s32 Num4Y,NumY, OffSec, Off4Y = 0;  
		u32 i;  
		s32 NumDay;   
//		u32 TimeVar;
	
		TimeVar = RTC_GetCounter();
		RTC_WaitForLastTask();
	
		/*转化为几个4年*/
    Num4Y = TimeVar/SecsPerFourYear;  
		/*4年内的偏移量s*/
    OffSec = TimeVar%SecsPerFourYear;  
   
		/*4年范围内偏移了几年*/
		i=1;  
		while(OffSec > Year_Secs_Accu[i++])  
			Off4Y++;  
     
		/* Numer of Complete Year 有几年*/  
		NumY = Num4Y*4 + Off4Y;  
		/* 2000,2001,...~2000+NumY-1 complete year before, so this year is 2000+NumY*/  
		Real_Time.year = 2000+NumY;  
		BKP_WriteBackupRegister(BKP_DR2,Real_Time.year);
		
		OffSec = OffSec - Year_Secs_Accu[i-2];  
       
		/* Month (TBD with OffSec)*/  
		i=0;  
		if(Real_Time.year%4)  /*普通年*/
		{  
				while(OffSec > Month_Secs_Accu_C[i++]);  
				Real_Time.month = i-1;  
				OffSec = OffSec - Month_Secs_Accu_C[i-2];  
		}  
		else  		/*闰年*/
		{
				while(OffSec > Month_Secs_Accu_L[i++]);  
				Real_Time.month = i-1;  
				OffSec = OffSec - Month_Secs_Accu_L[i-2];  
		}  
		BKP_WriteBackupRegister(BKP_DR3,Real_Time.month);
       
		/* Date (TBD with OffSec) */  
		NumDay = OffSec/SecsPerDay;  
		OffSec = OffSec%SecsPerDay;  
		Real_Time.day = NumDay+1;  
		BKP_WriteBackupRegister(BKP_DR4,Real_Time.day);
   
    /* Compute  hours */  
		Real_Time.hour = OffSec/3600;  
		/* Compute minutes */  
		Real_Time.minute = (OffSec % 3600)/60;  
		/* Compute seconds */  
		Real_Time.second = (OffSec % 3600)% 60;  
}   
/******************************************************************************* 
 * Function Name  : GetTime()
 * Description    : Displays the current time. 
 * Input          : - TimeVar: RTC counter value. 
 * Output         : None 
 * Return         : None 
 *******************************************************************************/  
uint32_t GetTimeSecond()
{
		return TimeVar;
}
/******************************************************************************* 
 * Function Name  : Time_Regulate 
 * Description    : Returns the time entered by user, using Hyperterminal. 
 * Input          : None 
 * Output         : None 
 * Return         : Current time RTC counter value 
 *******************************************************************************/   
u32 Month_Days_Accu_C[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};  
u32 Month_Days_Accu_L[13] = {0,31,60,91,121,152,182,213,244,274,305,335,366};  
#define SecsPerDay (3600*24)  
   
u32 Time_Regulate(u32 Tmp_Year,\
									u32 Tmp_Month,\
									u32 Tmp_Date,\
									u32 Tmp_HH,\
									u32 Tmp_MM,\
									u32 Tmp_SS)
{ 
		u32 LeapY, ComY, TotSeconds, TotDays;  
     
   /* change Year-Month-Data-Hour-Minute-Seconds into X(Second) to set RTC->CNTR */  
    if(Tmp_Year==2000)  
       LeapY = 0;  
    else  
       LeapY = (Tmp_Year - 2000 -1)/4 +1;  
       
		ComY = (Tmp_Year - 2000)-(LeapY);  
     
		if (Tmp_Year%4)  
     //common year  
			TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_C[Tmp_Month-1] + (Tmp_Date-1);   
		else  
     //leap year  
			TotDays = LeapY*366 + ComY*365 + Month_Days_Accu_L[Tmp_Month-1] + (Tmp_Date-1);   
    
		TotSeconds = TotDays*SecsPerDay + (Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS);   
     
		/* Return the value to store in RTC counter register */  
		RTC_SetCounter(TotSeconds);
		
		return TotSeconds;  
		
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
  * @brief  This function handles RTC global interrupt request.
  * @param  None
  * @retval None
  */
void RTC_IRQHandler(void)
{
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    /* Clear the RTC Second interrupt */
    RTC_ClearITPendingBit(RTC_IT_SEC);
  }
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
