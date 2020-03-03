/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MVC_H
#define __MVC_H
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"	
#include "adc.h"
#include "usart.h"
#include "can.h"
#include "message.h"
#include "lock.h"
#include "rtc.h"
#include "rc522.h"
#include "modbus.h"
#include "table.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/****************************************************
变量名：TRANSATION 事务结构体 
描述：事务结构体描述了一次充电行为 mvc状态机操作的model就是
			事务，startTime记录正式充电过程开始的绝对时间，从RTC
			获得，endTime记录充电的结束时间。这两个事件随状态机的
			运行而更新,从进入充电状态（S6 S7 S8 S9）的时候记录。
			transactionStatus记录当前情况下的状态 配置--执行--挂起
			发起者和结算者主要记录卡号 只要有人结算就行
			累计时间主要根据事务状态累积，而累积电量和累积金额实际
			上是一回事，它取决于电表的值
			充电模式由状态机更新，根据用户选择进行更新
			充电时间、金额、电量也是由用户更新
			基准电能值 肯定由电表读出
			
			此结构体作为m，外部输入通过状态机转化为m中的内容，显示
			只是显示事务结构体中的内容。
****************************************************/
typedef struct 
{
		uint8_t transactionStatus;			/*事务状态 配置0 充电1 缴费2 结束3 由状态机逻辑改变*/
		uint8_t transactionSponsor[4];	/*发起者*/
		uint8_t transactionPayer[4];		/*结算者*/
	
		T_STRUCT rtcTime;				/*当前时间*/
		T_STRUCT startTime;			/*事务开始时间 当事务状态为1 且 startTime为0*/
		T_STRUCT endTime;				/*事务结束时间 当事务状态为0 且 endTime为0*/
	
		uint8_t  chargeMode;						/*充电模式 状态逻辑更新 0电量 1时间 2金额 3自动*/
		uint32_t chargeTime;						/*充电时间 状态逻辑更新*/
		uint32_t chargeMoney;						/*充电金额 状态逻辑更新*/
		uint32_t chargeElectricity;			/*充电电量 状态逻辑更新*/
	
		uint32_t accumulateTime;				/*累积时间 状态为充电 ++*/
		uint32_t accumulateMoney;				/*累积金额*/
		uint32_t accumulateElectricity;	/*累积电量 状态为充电 减去基准*/
	
		uint8_t  matchStatus;						/*时间 金额 电量 是否达到 0：未达到 1：达到*/
		uint32_t electricEnergy;				/*基准电能值 状态为充电 为0*/
		uint8_t rate;										/*当前费率*/
	
		uint8_t moneyOK;								/*余额够了吗*/
		
		/*当前电压 电流 电量*/
		uint16_t voltage;
		uint8_t  current;
		uint32_t w;
		
		/*枪连接状态*/
		uint8_t CCstate;
		/*卡状态*/
		CARD_STATUS cardStatus;
		
		/*按键键值 属于输入系统*/
		uint8_t keyValue;
		
		/*锁状态*/
		uint8_t lockStatus;
		
		/*充电状态机状态*/
		uint8_t chargeStatus;
		
}TRANSACTION;
/* Exported functions ------------------------------------------------------- */
void MvcTableInit(void);
void MvcStateMachineOpen(void);
void MvcStateMachineClose(void);
void MvcStateTimeUpdate(void);
void MvcConditionUpdate(void);
void MvcStateTransitionUpdate(void);
void transactionTableUpdate(void);

void MvcActionExecutor(struct message actionMessage);
uint8_t MvcGetActiveState(void);
uint8_t MvcGetSuperState(void);
TRANSACTION MvcGetTransation();
void MvcSetChargeValue(uint32_t value);
void MvcSetChargeMode(uint8_t mode);
#endif
