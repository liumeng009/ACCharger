/**
  ******************************************************************************
  * @file    SysTick/TimeBase/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body.
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
#include "systick.h"
#include "can.h"

/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup SysTick_TimeBase
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t sysMonitorTimer;	/*系统监视定时器*/

static __IO uint32_t TickCount;
static __IO uint32_t tick_10ms;
static __IO uint32_t tick_100ms;
static __IO uint32_t tick_300ms;
static __IO uint32_t tick_500ms;
static __IO uint32_t tick_1000ms;

static __IO uint32_t tick_1ms; //针对时间轴的带有偏移的时序控制
// 这个变量十分重要 开始___时间点1__时间点2___时间点3___结束（开始）
// 每一个偏移量可以定义为const固定值 也可以用变量保存偏移（ms级）

static uint8_t offset1 = 60;
static uint8_t offset2 = 120;
static uint8_t offset3 = 250;

/*Megmeet 使用的时间序列变量 通过它可以任意修改时序*/
static uint8_t sequenceValueMegmeet = 0;
/*用于Megmeet判断的ms级虚拟定时器*/
static uint16_t vMegmeetTimer = 0;	
/*用于adc判断的ms级虚拟定时器*/
static uint8_t vAdcTimer = 0;	
/*用于RC522判断的ms级虚拟定时器*/
static uint8_t vRC522Timer = 0;	
/*用于W5500判断的ms级虚拟定时器*/
static uint8_t vW5500Timer = 0;	
/*用于自锁装置判断的ms级虚拟定时器*/
static uint16_t vLockTimer = 0;	
/*用于led判断的ms级虚拟定时器*/
static uint16_t vLedTimer = 0;	

static uint8_t vUsart2TimerOn = 0;	/*开关*/
static uint8_t vUsart2Timer = 0;	/*用于usart2判断的ms级虚拟定时器*/
static uint8_t vUsart2TimerAlarmValue = 0;

static uint8_t vUsart3TimerOn = 0;	/*开关*/
static uint8_t vUsart3Timer = 0;	/*用于usart3判断的ms级虚拟定时器*/
static uint8_t vUsart3TimerAlarmValue = 0;

static uint8_t vUsart4TimerOn = 0;	/*开关*/
static uint8_t vUsart4Timer = 0;	/*用于usart4判断的ms级虚拟定时器*/
static uint8_t vUsart4TimerAlarmValue = 0;

static uint8_t vCanTimerOn = 0;	/*开关*/
static uint8_t vCanTimer = 0;	/*用于can protocal判断的ms级虚拟定时器*/
static uint8_t vCanTimerAlarmValue = 0;


struct message sysTickMessage;	//滴答用的消息实例

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  init program.
  * @param  None
  * @retval None
  */
int sysTickInit(void)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     


  /* Setup SysTick Timer for 1 msec interrupts.
     ------------------------------------------
    1. The SysTick_Config() function is a CMSIS function which configure:
       - The SysTick Reload register with value passed as function parameter.
       - Configure the SysTick IRQ priority to the lowest value (0x0F).
       - Reset the SysTick Counter register.
       - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
       - Enable the SysTick Interrupt.
       - Start the SysTick Counter.
    
    2. You can change the SysTick Clock source to be HCLK_Div8 by calling the
       SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8) just after the
       SysTick_Config() function call. The SysTick_CLKSourceConfig() is defined
       inside the misc.c file.

    3. You can change the SysTick IRQ priority by calling the
       NVIC_SetPriority(SysTick_IRQn,...) just after the SysTick_Config() function 
       call. The NVIC_SetPriority() is defined inside the core_cm3.h file.

    4. To adjust the SysTick time base, use the following formula:
                            
         Reload Value = SysTick Counter Clock (Hz) x  Desired Time base (s)
    
       - Reload Value is the parameter to be passed for SysTick_Config() function
       - Reload Value should not exceed 0xFFFFFF
   */
  if (SysTick_Config(SystemCoreClock / 1000))
  { 
    /* Capture error */ 
    while (1);
  }
  
  return 0;
}

uint32_t GetTickCount()
{
		return TickCount;
}
void vUsart2TimerSetAlarm(uint8_t alarmValue)
{
		vUsart2Timer = 0;												/*计时回到0点*/
		vUsart2TimerAlarmValue = alarmValue;		/*填入闹钟值*/
		vUsart2TimerOn = 1;											/*打开计时器*/
}

void vUsart3TimerSetAlarm(uint8_t alarmValue)
{
		vUsart3Timer = 0;												/*计时回到0点*/
		vUsart3TimerAlarmValue = alarmValue;		/*填入闹钟值*/
		vUsart3TimerOn = 1;											/*打开计时器*/
}

void vUsart4TimerSetAlarm(uint8_t alarmValue)
{
		vUsart4Timer = 0;												/*计时回到0点*/
		vUsart4TimerAlarmValue = alarmValue;		/*填入闹钟值*/
		vUsart4TimerOn = 1;											/*打开计时器*/
}

void vCanTimerSetAlarm(uint8_t alarmValue)
{
		vCanTimer = 0;												/*计时回到0点*/
		vCanTimerAlarmValue = alarmValue;			/*填入闹钟值*/
		vCanTimerOn = 1;											/*打开计时器*/
}
void vCanTimerClose()
{
		vCanTimerOn = 0;
}

void vAdcTimerSetValue(uint8_t initValue)
{
		vAdcTimer = initValue;						 /*赋值后会开始自动+1 永远不停*/
}
uint16_t vAdcTimerGetValue()
{
		return vAdcTimer;
}

void vMegmeetTimerSetValue(uint8_t initValue)
{
		vMegmeetTimer = initValue;						 /*赋值后会开始自动+1 永远不停*/
}
uint16_t vMegmeetTimerGetValue()
{
		return vMegmeetTimer;
}

void vRC522TimerSetValue(uint8_t initValue)
{
		vRC522Timer = initValue;						 /*赋值后会开始自动+1 永远不停*/
}
uint16_t vRC522TimerGetValue()
{
		return vRC522Timer;
}

void vW5500TimerSetValue(uint8_t initValue)
{
		vW5500Timer = initValue;						 /*赋值后会开始自动+1 永远不停*/
}
uint16_t vW5500TimerGetValue()
{
		return vW5500Timer;
}

void vLockTimerSetValue(uint16_t initValue)
{
		vLockTimer = initValue;						 /*赋值后会开始自动+1 永远不停*/
}
uint16_t vLockTimerGetValue()
{
		return vLockTimer;
}
void vLedTimerSetValue(uint16_t initValue)
{
		vLedTimer = initValue;						 /*赋值后会开始自动+1 永远不停*/
}
uint16_t vLedTimerGetValue()
{
		return vLedTimer;
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
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
		/*每 1ms 要执行的操作 注意此操作过于频繁 如果主程序有超过nms的延时 那么
		消息队列就有可能积累很多消息 影响到一些实时任务 造成系统不稳定*/
		sysTickMessage.destinationAddress = 1;	
		sysTickMessage.cmd = 0;
		messagePostIt(sysTickMessage);
		
		tick_1ms++;
	
		/*以下是普通定时器*/
		vMegmeetTimer++;				/*Megmeet任务专用的定时器*/
		vAdcTimer++;
		vRC522Timer++;
		vW5500Timer++;
		vLockTimer++;
		vLedTimer++;

		/*以下是闹钟定时器*/
		if(vUsart2TimerOn == 1)
		{
				vUsart2Timer++;	
				if(vUsart2Timer > vUsart2TimerAlarmValue)	/*说明超时了*/
				{
						/*如果到这里 说明超时情况发生了 那么认为这就是一帧*/
						/*于是 ready = 1 这里可以采用两种方式把ready置一*/
						/*第一 发送一个消息给状态机 告诉他接收超时 一帧已经收到*/		
						/*第二 直接置ready为1 这样耦合性提高 但不会来不及处理*/
						Usart2SetReady();

						vUsart2TimerOn = 0;	/*使用完成 关闭闹钟*/
				}
		}
		if(vUsart3TimerOn == 1)
		{
				vUsart3Timer++;	
				if(vUsart3Timer > vUsart3TimerAlarmValue)	/*说明超时了*/
				{
						/*如果到这里 说明超时情况发生了 那么认为这就是一帧*/
						/*于是 ready = 1 这里可以采用两种方式把ready置一*/
						/*第一 发送一个消息给状态机 告诉他接收超时 一帧已经收到*/		
						/*第二 直接置ready为1 这样耦合性提高 但不会来不及处理*/
						Usart3SetReady();
						//usart1SendByte(0xcc);
						vUsart3TimerOn = 0;	/*使用完成 关闭闹钟*/
				}
		}
		if(vUsart4TimerOn == 1)
		{
				vUsart4Timer++;	
				if(vUsart4Timer > vUsart4TimerAlarmValue)	/*说明超时了*/
				{
						/*如果到这里 说明超时情况发生了 那么认为这就是一帧*/
						/*于是 ready = 1 这里可以采用两种方式把ready置一*/
						/*第一 发送一个消息给状态机 告诉他接收超时 一帧已经收到*/		
						/*第二 直接置ready为1 这样耦合性提高 但不会来不及处理*/
						Usart4SetReady();
						//usart1SendByte(0xcc);
						vUsart4TimerOn = 0;	/*使用完成 关闭闹钟*/
				}
		}
		if(vCanTimerOn == 1)
		{
				vCanTimer++;	
				if(vCanTimer > vCanTimerAlarmValue)	/*说明超时了*/
				{
						/*如果到这里 说明超时情况发生了 那么认为连接中断了*/
						
						/*第一 发送一个消息给状态机 告诉他接收超时 连接中断*/		
						/*第二 直接置ready为1 这样耦合性提高 连接中断*/
						LinkClose();
						
						vCanTimerOn = 0;	/*使用完成 关闭闹钟*/
				}
		}
		
		if (tick_1ms == offset1)
		{

		}
		if (tick_1ms == offset2)
		{
				
		}
		if (tick_1ms == offset3)
		{
				
				tick_1ms = 0;
		}
		
		
    if (++tick_10ms >= 10)   // 10ms计时
    {
        tick_10ms = 0;
            
        // 每 10ms 要执行的操作
        // ...
    }
		if (++tick_100ms >= 100)   // 100ms计时
    {
        tick_100ms = 0;
        sequenceValueMegmeet++;
        // 每 100ms 要执行的操作
        // ...
				sysTickMessage.destinationAddress = 6;	/*每100ms进行一次 电压或电流或电量检测*/
				sysTickMessage.cmd = 1;								/*cmd = 0：发送 1：接收*/
				messagePostIt(sysTickMessage);

				/*基于sequenceValueMegmeet的执行序列*/
				if(sequenceValueMegmeet == 2)
				{
						sysTickMessage.destinationAddress = 7;	/*每100ms进行一次 */
						sysTickMessage.cmd = 0;								
						messagePostIt(sysTickMessage);
				}
				if(sequenceValueMegmeet == 3)
				{
						sysTickMessage.destinationAddress = 7;	/*每100ms进行一次 */
						sysTickMessage.cmd = 1;								
						messagePostIt(sysTickMessage);
				}
				if(sequenceValueMegmeet == 4)
				{
						sysTickMessage.destinationAddress = 7;	/*每500ms进行一次 */
						sysTickMessage.cmd = 0;								
						messagePostIt(sysTickMessage);
				}
				if(sequenceValueMegmeet == 6)
				{
						sysTickMessage.destinationAddress = 7;	/*每500ms进行一次 */
						sysTickMessage.cmd = 2;								
						messagePostIt(sysTickMessage);
						
						sequenceValueMegmeet = 0;
				}
			
    }
		if (++tick_300ms >= 300)   // 300ms计时
    {
        tick_300ms = 0;
            
        // 每 300ms 要执行的操作
        // ...
				sysTickMessage.destinationAddress = 2;
				sysTickMessage.cmd = 0;
				messagePostIt(sysTickMessage);

				sysTickMessage.destinationAddress = 8;	/*sysMonitor*/
				sysTickMessage.cmd = 0;								
				messagePostIt(sysTickMessage);
    }
		if (++tick_500ms >= 500)   // 500ms计时
    {
        tick_500ms = 0;
            
        // 每 500ms 要执行的操作
        // ...
				sysTickMessage.destinationAddress = 3;
				sysTickMessage.cmd = 0;
				messagePostIt(sysTickMessage);
				
				sysTickMessage.destinationAddress = 10;	/*RFID*/
				sysTickMessage.cmd = 0;								
				messagePostIt(sysTickMessage);	
			
				sysTickMessage.destinationAddress = 13;	/*紧急开关检测*/
				sysTickMessage.cmd = 0;								
				messagePostIt(sysTickMessage);	
    }
		if (++tick_1000ms >= 1000)   // 1000ms计时
    {
        tick_1000ms = 0;
			
        TickCount++;  // 时间滴答值  (1s)  
        // 每 1s 要执行的操作
        // ...
			
				sysTickMessage.destinationAddress = 6;	/*每1s进行一次 电压或电流或电量检测*/
				sysTickMessage.cmd = 0;								/*cmd = 0：发送 1：接收*/
				messagePostIt(sysTickMessage);
			
				sysTickMessage.destinationAddress = 12;	/*每1s进行一次 rtc*/
				sysTickMessage.cmd = 0;								
				messagePostIt(sysTickMessage);

    }
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
