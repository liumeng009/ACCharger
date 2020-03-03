/**
  ******************************************************************************
  * @file    mvc.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "mvc.h"
#include "string.h"
#include "iDM100E.h"

#define STATE_SIZE            30		//总共有多少个state 就是有多少个界面
#define STATE_TRANSITION_SIZE 31  	//迁移条目的个数
#define STATE_CONDITION_SIZE	7			//条件类型的数目
#define STATE_CONDITION_CODE (STATE_CONDITION_SIZE+1)

conditionQueue mvcTriggerQueue;				/*条件编码队列*/

struct message mvcTransitionMessage;	/*迁移用的消息实例*/

/****************************************************
变量名：transTable
描述: 基本model 操作的基本单元
****************************************************/
TRANSACTION transactionTable;

/****************************************************
函数名：transactionTableUpdate（实时运行）
形参：无
返回值：无
函数功能：在此函数中，transactionTable作为最基本的数据结构
transactionTableUpdate就负责将所有数据实时更新到表格中，注意
此函数中不允许等待，不允许有耗时操作，所有外设必须将API准备
好，以供此函数调用！
			此函数牵扯到数据之间逻辑的表达，这个逻辑是和状态机逻辑
一致，状态机只是表达状态之间的逻辑，但是数据之间的逻辑也是要
表达的，比如，金额是不是足够
			此处不再采用协同多任务的写法，因为执行太快了
****************************************************/
void transactionTableUpdate()
{
		static uint8_t tmpTimeCounter;		/*临时值 用于500ms的计数*/
		uint32_t accumulateElectricityOld;
		/*首先更新硬件更新值*/
	
		/*更新电压 电流 电量*/
		transactionTable.voltage = GetVoltageValue();
		transactionTable.current = GetCurrentValue();
		transactionTable.w = GetWValue()*10;		/*0.1度*/
		
		transactionTable.rate = 1;
	
		transactionTable.cardStatus = GetCardStatus();
		transactionTable.lockStatus = GetLockStatus();
			
		transactionTable.keyValue =  GetKeyValue();
	
		transactionTable.chargeStatus =  GetSuperState();
	
		transactionTable.CCstate =	AdcValueToCCstate();
	
		transactionTable.rtcTime = GetTime();
	
		/*确定当前充电模式*/
		if(MvcGetActiveState() == 2)
			transactionTable.chargeMode = 0;
		if(MvcGetActiveState() == 3)
			transactionTable.chargeMode = 2;
		if(MvcGetActiveState() == 4)
			transactionTable.chargeMode = 1;
		if(MvcGetActiveState() == 5)
			transactionTable.chargeMode = 3;
	
		/*软件更新值 根据所处的状态进行更新 就是说每一个状态下的刷新内容不一样*/
		/*目前mvc状态机的状态可以通过MvcGetActiveState()得到*/
		/*但是状态机的状态太多了，transactionStatus只需要基本状态就可以了*/
		/*实际上transactionStatus是super state，其实就4个状态*/
		/*配置----充电----缴费----结束*/
	
		if(transactionTable.transactionStatus == 0)		/*如果在配置阶段*/
		{
				if(transactionTable.cardStatus.status == 1)		/*卡在线*/
						memcpy(&(transactionTable.transactionSponsor),\
									&(transactionTable.cardStatus.serialNum),\
									sizeof(transactionTable.cardStatus.serialNum));
				//else
				//		memset(&(transactionTable.transactionSponsor),0,sizeof(transactionTable.cardStatus.serialNum));
				
				/*在配置状态下 基准电能值不停更新 在其他状态下不更新*/
				transactionTable.electricEnergy = transactionTable.w;
				
				switch(transactionTable.chargeMode)
				{
					case 0:/*电量*/
						transactionTable.chargeMoney = transactionTable.chargeElectricity * 1;
						transactionTable.chargeTime =  transactionTable.chargeElectricity * 2;
						break;
					case 1:/*时间*/
						transactionTable.chargeMoney = transactionTable.chargeTime / 2;
						transactionTable.chargeElectricity = transactionTable.chargeTime / 2;
						break;
					case 2:/*金额*/
						transactionTable.chargeTime = transactionTable.chargeMoney / 3.0;
						transactionTable.chargeElectricity = transactionTable.chargeMoney * 1;
						break;
					default:break;
				}
		}
		if(transactionTable.transactionStatus == 1)		/*如果在充电阶段*/
		{
				if(transactionTable.startTime.year == 0)
						transactionTable.startTime = GetTime();	/*记录开始时间*/
				if(transactionTable.electricEnergy == 0)
						transactionTable.electricEnergy = GetWValue();	/*记录基准电能值*/
				
				/*保存旧值*/
				accumulateElectricityOld = transactionTable.accumulateElectricity;
				transactionTable.accumulateElectricity = \
				transactionTable.w - transactionTable.electricEnergy;
				
				if((transactionTable.accumulateElectricity - accumulateElectricityOld) == 1)
				{
						transactionTable.accumulateMoney += transactionTable.rate;
				}
				
				tmpTimeCounter++;
				if(tmpTimeCounter >= 2)
				{
					transactionTable.accumulateTime++;	/*以2 X 500ms 也就是1s为单位*/
					tmpTimeCounter = 0;
				}
				
				if(transactionTable.chargeMode == 0)		/*电量模式*/
				{
						if(transactionTable.accumulateElectricity >= \
							 transactionTable.chargeElectricity)
						{
								transactionTable.matchStatus = 1;			/*达到状态了*/
						}
						else
						{
								transactionTable.matchStatus = 0;
						}
				}
				
				if(transactionTable.chargeMode == 1)		/*时间*/
				{
						if(transactionTable.accumulateTime >= \
							 transactionTable.chargeTime * 60)
						{
								transactionTable.matchStatus = 1;			
						}
						else
						{
								transactionTable.matchStatus = 0;
						}
				}
				
				if(transactionTable.chargeMode == 2)		 /*金额*/
				{
						if(transactionTable.accumulateMoney >= \
							 transactionTable.chargeMoney)
						{
								transactionTable.matchStatus = 1;			
						}
						else
						{
								transactionTable.matchStatus = 0;
						}
				}
		}
		if(transactionTable.transactionStatus == 2)		/*如果在缴费阶段*/
		{
				
				if(transactionTable.endTime.year == 0)
						transactionTable.endTime = GetTime();		/*记录结束时间*/
				if(transactionTable.cardStatus.status == 1)
				{
						if(transactionTable.cardStatus.rxWalletValue > transactionTable.accumulateMoney)
							transactionTable.moneyOK =1;

						memcpy(&(transactionTable.transactionPayer),\
									&(transactionTable.cardStatus.serialNum),\
									sizeof(transactionTable.cardStatus.serialNum));
				}
				else
					transactionTable.moneyOK = 0;
		}
		if(transactionTable.transactionStatus == 3)		/*如果在结束阶段*/
		{
				/*首先将事务信息保存*/
				/*清零此次事务的所有数据*/
				memset(&transactionTable,0,sizeof(transactionTable));
		}
}

/****************************************************
变量名：mvcStateMachineSwitcher
描述：状态机的开关 0：关闭 1：启动
****************************************************/
uint32_t mvcStateMachineSwitcher;

/****************************************************
变量名：mvcTimeBase 
描述：以1ms为单位的时间轴 最长可以计算到49.71026天/充电
用来给每一个状态的时间戳提供一个基准，当BMS开始时启动
在BMS结束时清0，为状态机加入时间属性
****************************************************/
uint32_t mvcTimeBase;	

/****************************************************
变量名：mvcActiveState 
描述：活动的状态
****************************************************/
uint8_t mvcActiveState;	

/****************************************************
变量名：mvcStateChanged 
描述：1；状态已经改变 0；状态没有改变
置一动作是在迁移表中进行
清零动作是在时间表中进行（一定可以清零）
****************************************************/
uint8_t mvcStateChanged;	

/****************************************************
变量名：stateTransitionItem  状态迁移项
描述：源状态+触发条件+目标状态+动作
触发条件只能是消息（自己产生的时间消息也可）
动作只能是消息
****************************************************/
typedef struct 
{
	uint8_t sourceState;		//源状态
	uint8_t triggerCondition;	//触发条件
	uint8_t targetState;		//目标状态
	uint8_t action;				//动作
}stateTransitionItem;

/****************************************************
变量名：mvcStateTransitionTable  状态迁移表
描述：由n个stateTransitionItem组成，描述了所有的状态
迁移，包括逻辑上的和时间上的 
----<原状态，条件编码，目的状态，动作>-----
****************************************************/
stateTransitionItem mvcStateTransitionTable[STATE_TRANSITION_SIZE] = 
{
		{1, 	 0xe1,  2,   0},			/*确定电量模式*/
		{2, 	 0xe2,  1,   0},			/*返回*/
		{2, 	 0xe3,  6,   0},			/*确定*/
		{6, 	 0xe2,  2,   0},			/*返回*/
		{6, 	 0xe8,  2,   0},			/*返回*/
		
		{6, 	 0xe7, 10, 	 0xc1},		/*闭锁 开状态机*/
		{10, 	 0xe4, 14, 	 0xc2},		/*关闭状态机*/
		{10, 	 0xe5, 14, 	 0xc2},		/*关闭状态机*/
		{10, 	 0xe9, 14, 	 0xc2},	
		{10, 	 0xD2, 14, 	 0xc2},	
		{10, 	 0xD3, 14, 	 0xc2},	
		{10, 	 0xD4, 14, 	 0xc2},	
		{10, 	 0xD5, 14, 	 0xc2},
		{10, 	 0xD6, 14, 	 0xc2},		/*紧急中止*/
		{14, 	 0xe6, 22, 	 0xc3},		/*扣费 开锁*/
		{22, 	 0xe8,  1,   0xc4},		/*记录此次充电过程*/
		
		{1, 	 0xea,  3,   0},			/*确定金额模式*/
		{3, 	 0xe2,  1,   0},			/*返回*/
		{3, 	 0xe3,  6,   0},			/*确定*/
		{6, 	 0xe2,  3,   0},			/*返回*/
		{6, 	 0xe8,  3,   0},			/*返回*/
		
		{1, 	 0xeb,  4,   0},			/*确定时间模式*/
		{4, 	 0xe2,  1,   0},			/*返回*/
		{4, 	 0xe3,  6,   0},			/*确定*/
		{6, 	 0xe2,  4,   0},			/*返回*/
		{6, 	 0xe8,  4,   0},			/*返回*/
		
		{1, 	 0xec,  5,   0},			/*确定自动模式*/
		{5, 	 0xe2,  1,   0},			/*返回*/
		{5, 	 0xd0,  1,   0},			/*返回*/
		{5, 	 0xe8,  1,   0},			/*返回*/
		{5, 	 0xe7,  10,  0xc1}		/*确定*/
};

/****************************************************
变量名：stateTimeItem  状态时间戳
描述：记录了状态所有可能的时间戳 通过这个表可以判断
状态是不是超时，经历的次数是不是太多，规定时间内有没有
完成某一动作等等，是迁移表的时间属性。
****************************************************/
typedef struct 
{
	uint32_t firstTime;		//状态最开始时间
	uint32_t latestTime;	//状态最近一次时间
	uint32_t duration;		//状态最近一次持续时间
	uint32_t times;				//状态总共次数
}stateTimeItem;

/****************************************************
变量名：mvcStateTimeTable  状态时间表
描述：由n个stateTimeItem组成，描述了所有的状态
的时间
****************************************************/
stateTimeItem mvcStateTimeTable[STATE_SIZE];

/****************************************************
变量名：conditionsQueue  条件队列
描述：在某个state活动的条件下，所有产生的条件，之所以
用一个队列，是因为条件的产生频率有可能大于迁移更新的频率
为了防止丢掉了条件，所以采用了队列
在发生了成功的迁移之后，要把队列中剩余的条件清零，防止
影响下一个状态，发生逻辑错误
****************************************************/
conditionQueue mvcTriggerQueue;

/****************************************************
函数名：tableInit
形参：无
返回值：无
函数功能：在此函数中，初始化状态机的各项变量，回到
状态机最开始的状态 
****************************************************/
void MvcTableInit()
{
	uint8_t counter;
	mvcStateMachineSwitcher = 0; //关闭状态机
	mvcTimeBase = 0;			  //时间轴清零
	mvcActiveState = 1;		  //初始状态
	for(counter = 0; counter < STATE_SIZE; counter++)
	{
		mvcStateTimeTable[counter].firstTime = 0;
		mvcStateTimeTable[counter].latestTime = 0;
		mvcStateTimeTable[counter].duration = 0;
		mvcStateTimeTable[counter].times = 0;
	}
	mvcTriggerQueue.size = 0;	  //清空条件队列
}

/****************************************************
函数名：MvcStateMachineOpen
形参：无
返回值：无
函数功能：在此函数中，初始化状态机的各项变量，回到
状态机最开始的状态 
****************************************************/
void MvcStateMachineOpen()
{
	//MvcTableInit();
	mvcStateMachineSwitcher = 1; //打开状态机
}

/****************************************************
函数名：MvcStateMachineClose
形参：无
返回值：无
函数功能：在此函数中，初始化状态机的各项变量，回到
状态机最开始的状态 
****************************************************/
void MvcStateMachineClose()
{
	mvcStateMachineSwitcher = 0; //关闭状态机
}

/****************************************************
函数名：stateTransitionUpdate（10ms消息驱动）
形参：无
返回值：无
函数功能：在此函数中，每10ms执行一次迁移
先基于活动状态 然后再结合条件 在迁移表中查找，没找到就算了
如果找到，活动状态先更改 然后再执行动作（消息） 
迁移表只强调逻辑:
1.它并没有对时间和动作做出规定。
2.多个条件可以迁移到一个状态，一个状态可以有多个条件（图结构）
****************************************************/
void MvcStateTransitionUpdate()
{
	uint8_t tCounter = 0;
	uint8_t cCounter = 0;
	
	if(mvcStateMachineSwitcher == 0)	//说明状态机处于关闭状态
		return;
		
	//首先查找条件队列 如果有 继续 如果没有 return
	if(mvcTriggerQueue.size == 0)
		return; 
		
	//对表格进行遍历 寻找与mvcActiveState相同的条目
	//也就是说 查找mvcActiveState的触发条件都是什么
	for(tCounter = 0; tCounter < STATE_TRANSITION_SIZE; tCounter++)
	{
		if(mvcStateTransitionTable[tCounter].sourceState == mvcActiveState)
		{	//找到源状态==mvcActiveState的迁移项
			for(cCounter = 0 ; cCounter < mvcTriggerQueue.size; cCounter++)
			{	
				//和条件队列里的条件一一对比
				if(mvcStateTransitionTable[tCounter].triggerCondition == \
					mvcTriggerQueue.queue[cCounter])
				{
					//执行触发后的动作....
					//首先更改活动状态
					mvcActiveState = mvcStateTransitionTable[tCounter].targetState;
					//将条件队列清零
					mvcTriggerQueue.size = 0;
					//状态改变标志位
					mvcStateChanged = 1;
					//执行某些动作....
					if(mvcStateTransitionTable[tCounter].action != 0) /*确实有有效的动作编码*/
					{
							mvcTransitionMessage.destinationAddress = 5;
							mvcTransitionMessage.cmd = 2;
							mvcTransitionMessage.parameter_1 = mvcStateTransitionTable[tCounter].action;
							messagePost(mvcTransitionMessage);
					}
				}
				else	/*迁移条件没有被满足*/
				{
					//通知系统无用的条件数量，用于调试
				}
				if(mvcStateChanged == 1)
					break;
			}
		}
		if(mvcStateChanged == 1)
				break;
	}
	
	/*经过遍历之后没有针对mvcActiveState的条件被满足，也就是说
	当下这些在队列里的条件都不是针对mvcActiveState的，那么这些
	条件对于状态机来说无意义，应该被舍弃*/
	/*就算是有迁移发生 也应该被清零*/
	
	mvcTriggerQueue.size = 0;
	
	/*在迁移执行之后，状态有可能被改变了，于是要周期性的更新宏观状态*/
	/*看看是处于配置 充电 缴费 还是结束 状态中的一个 以便于为值的更新
	提供条件*/
	MvcGetSuperState();
}

/****************************************************
函数名：stateTimeUpdate（1ms消息驱动）
形参：无
返回值：无
函数功能：在此函数中，每1ms更新一次mvcTimeBase
判断活动的状态是否改变了mvcStateChanged变量
如果变成了新的状态，那么清零新状态的duration
如果变成了新的状态，那么次数times++
只要firstTime为0，那么就记录为mvcTimeBase
正在活动的状态的duration++（记录）
正在活动的状态latestTime记录为mvcTimeBase
时间表是迁移表的时间属性：
1.定义了每一种状态的详细时间属性
****************************************************/
void MvcStateTimeUpdate()
{
	uint8_t counter;			/*普通变量*/
	
	if(mvcStateMachineSwitcher == 0)	//说明状态机处于关闭状态
		return;

	mvcTimeBase++;					//每1ms加一
	if(mvcStateChanged)    //状态表发生了迁移,状态改变了
	{
		mvcStateTimeTable[mvcActiveState].duration = 0;
		mvcStateTimeTable[mvcActiveState].times++;

		//如果此状态的firstTime为0 
		if(mvcStateTimeTable[mvcActiveState].firstTime == 0)
		{
			//打上时间戳
			mvcStateTimeTable[mvcActiveState].firstTime = mvcTimeBase;
		}
		mvcStateChanged = 0;	//将标志位清零 对于时间控制来说已经用完
	}
	
	mvcStateTimeTable[mvcActiveState].duration++; //状态持续周期
	mvcStateTimeTable[mvcActiveState].latestTime = mvcTimeBase; //更新最新时间
	
}
/****************************************************
变量名：mvcTimeOutTable 
描述：停留在每一种状态的timeOut时间 此处根据STATE_SIZE初始化值
			定义数组的长度
****************************************************/
const uint32_t mvcTimeOutTable[STATE_SIZE] = 
{
 1000,1000,1000,1000,1000,1000,1000,10,10,10,
 1000,10,250,10,10,250,10,10,10,10,
 10,10,10000,10,10,250,10,10,10,10
};
/****************************************************
变量名：mvcConditionTable 
描述：根据卡状态 按键键值 充电状态机状态 的值 映射到 条件编码
****************************************************/
const uint8_t mvcConditionTable[24][STATE_CONDITION_CODE] = 
{//card  money   status  stateM  key_value   time   gun   code
	{0,      0,      0,       0 ,   0x03,       0,     0,   0xE1 },/*电量键*/
	{0,      0,      0,       0 ,   0x04,       0,     0,   0xEA },/*金额键*/
	{0,      0,      0,       0 ,   0x05,       0,     0,   0xEB },/*时间键*/
	{0,      0,      0,       0 ,   0x06,       0,     0,   0xEC },/*自动键*/
	{0,      0,      0,       0 ,   0x07,       0,     0,   0xED },/*user键*/
	{0,      0,      0,       0 ,   0x08,       0,     0,   0xEE },/*root键*/
	{0,      0,      0,       0 ,   0x09,       0,     0,   0xEF },/*deve键*/
	{0,      0,      0,       0 ,   0x02,       0,     0,   0xE2 },/*返回键*/
	{0,      0,      0,       0 ,   0x01,       0,     0,   0xE3 },/*确定键*/
	{0,      0,      1,       3 ,      0,       0,   	 0,   0xE4 },/*正在充电 match了*/
	{0,      0,      2,       0 ,      0,       0,     0,   0xE5 },/**/
	{1,      1,      0,       0 ,      0,       0,     0,   0xE6 },/*卡在线 钱也够*/
	{0,      0,      0,       0 ,   0x01,       0,  0x01,   0xE7 },/*抢插了 确定键*/
	{0,      0,      0,       0 ,   0x02,       0,  0x01,   0xE8 },/*抢插了 返回键*/
	{1,      0,      0,       0 ,   0x01,       0,     0,   0xD1 },/*卡在线 确定*/
	{1,      0,      0,       0 ,   0x02,       0,     0,   0xD0 },/*卡在线 返回*/
	{0,      0,      0,     0x03,   0x02,       0,     0,   0xD6 },/*充电中 返回*/
	{1,      0,      0,       0 ,      0,       0,     0,   0xE7 },/*卡在线*/
	{0,      0,      0,       0 ,      0,      22,     0,   0xE8 },/*22超时*/
	{0,      0,      0,       4 ,      0,       0,     0,   0xE9 },/**/
	{0,      0,      1,       1 ,      0,       0,     0,   0xD2 },/**/
	{0,      0,      1,       2 ,      0,       0,     0,   0xD3 },/**/
	{0,      0,      1,       3 ,      0,       0,     0,   0xD4 },/**/
	{0,      0,      1,       4 ,      0,       0,     0,   0xD5 },/**/
};
/****************************************************
变量名：conditionMask
描述：根据卡状态 按键键值 充电状态机状态 的值 映射到 条件编码
****************************************************/
const uint8_t conditionMask[23][STATE_CONDITION_SIZE] = 
{//card  money   status  stateM  key_value   time      gun
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S0*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S1*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S2*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S3*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0},/*S4*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0xff},/*S5*/
	{0,      0,      0,       0 ,      0xff,       0  ,   0xff},/*S6*/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/*S9调试界面*/
	{0,      0,   0xff,     0xff,   0xff,       0  ,      0},/*S10*/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0xff, 0xff,      0,       0 ,     0,        0,        0},/*S14*/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0xff,0xff,      0,       0 ,      0xff,    0  ,      0},/*S18*/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0  ,      0},/**/
	{0,      0,      0,       0 ,      0,       0xff,     0}/*S22*/
};
/****************************************************
函数名：conditionUpdate 条件收集（10ms消息驱动）
形参：无
返回值：无
函数功能：在此函数中，每1ms产生一个条件 这是定的
时间也是可以产生迁移条件的，在此函数中也要把时间考虑
因此可能产生两个条件 或者更多
总结就是2个 :逻辑迁移和时间迁移！
****************************************************/
void MvcConditionUpdate()
{
	uint8_t cCounter;		//条件用变量
	uint8_t tCounter;		//时间用变量
	uint8_t startFlag;	//有效标志
	uint8_t resultFlag;
	uint8_t timeOutFlag;
	uint8_t conditionArray[7];		/*条件列表*/
	
	if(mvcStateMachineSwitcher == 0)	//说明状态机处于关闭状态
		return;
	
	/*提取实时条件*/
	conditionArray[0] = transactionTable.cardStatus.status;		/*确定状态*/
	conditionArray[1] = transactionTable.moneyOK;							/*不一定是有效状态*/
	conditionArray[2] = transactionTable.matchStatus;					/*0未达到 1达到*/
	conditionArray[3] = transactionTable.chargeStatus;				/*充电桩状态机所属状态 待完善*/
	conditionArray[4] = transactionTable.keyValue;
	/*conditionArray[5] 在下面*/
	conditionArray[6] = transactionTable.CCstate;							/*0未连接 1连接*/
	
	/*扫描时间条件（针对mvcActiveState的）*/
	/*时间条件可以使duration持续时间太长了，也可以是不连接状态之间
	的时间太长了，更可以是firsttime到某另一状态的最新时间，使用起
	来比较灵活，除了持续时间duration，其他情况由于用数据结构表示的
	话，会比较复杂，所以因各自程序的差异，这里自由实现。duration还
	是用表格表示。*/
	
	/*首先检查durition的情况*/
	if(mvcStateTimeTable[mvcActiveState].duration >= mvcTimeOutTable[mvcActiveState])
	{
			//产生超时的条件编码...
			conditionArray[5] = mvcActiveState;
	}
	
	/*得到所有实时条件之后 按理说 应该和条件表对照得到条件编码*/
	/*即 条件-----> 条件编码 映射的过程*/
	/*但由于每一种状态它所关注的条件是不同的 重点是不同的，我们先针对活动状态
		进行条件掩码处理 防止干扰出现*/
	/*针对活动状态，经过过滤得到条件*/
	for(cCounter = 0; cCounter < STATE_CONDITION_SIZE; cCounter++)
	{
			conditionArray[cCounter] = conditionArray[cCounter] & \
																 conditionMask[mvcActiveState][cCounter];
	}
	/*检索条件--->条件编码映射表 得到编码*/
	for(cCounter = 0; cCounter < 24; cCounter++)
	{
		if(conditionArray[0] == mvcConditionTable[cCounter][0] && \
		   conditionArray[1] == mvcConditionTable[cCounter][1] && \
		   conditionArray[2] == mvcConditionTable[cCounter][2] && \
			 conditionArray[3] == mvcConditionTable[cCounter][3] && \
			 conditionArray[4] == mvcConditionTable[cCounter][4] && \
			 conditionArray[5] == mvcConditionTable[cCounter][5] && \
		   conditionArray[6] == mvcConditionTable[cCounter][6] )
		{
			//将触发条件写入
			mvcTriggerQueue.queue[mvcTriggerQueue.size] = mvcConditionTable[cCounter][STATE_CONDITION_SIZE];
			mvcTriggerQueue.size++;
			break; //找到一个条件就退出 因为多个条件无意义
		}
	}
	

	//再继续检查各种跨状态超时的特殊情况... 暂时不需要
#if 0
	if(mvcActiveState == 2)
	{
			if((mvcStateTimeTable[2].latestTime-mvcStateTimeTable[1].firstTime)> 5000)
			{
					ConditionInsert(&mvcTriggerQueue, 0xE1); /*条件编码从0xe1开始*/
			}
	}
	//..

	
	/*这里收集can接收报文的情况*/
	while(canmvcTriggerQueue.size--)
	{
			ConditionInsert(&mvcTriggerQueue, canmvcTriggerQueue.queue[canmvcTriggerQueue.size-1]);
	}
	
	/*这里收集关于megmeet模块的信息*/
	//...
	
	/*以下是针对部分报文时间的条件检测*/
	/*主题思想是先检测有效位的times，如果times没有满足要求，那么再检查超时*/
	/*重置初始状态*/
	startFlag = 0;
	resultFlag = 1;			 /*假定成功接收了*/
	timeOutFlag = 0;
	/*先检查有没有有效位*/
	for(tCounter = 0; tCounter < 3; tCounter++)
	{
			if(messageTimeTable[tCounter].start == 1)
					startFlag = 1;
	}
	if(startFlag == 1)	/*如果检测到有要检测的报文*/
	{
			for(tCounter = 0; tCounter < 3; tCounter++)
			{
					if((messageTimeTable[tCounter].start == 1)&&(messageTimeTable[tCounter].times == 0))
							resultFlag = 0;		/*有人接收失败*/							
					
			}
			if(resultFlag == 0)	/*没有成功接收 就检查超时的情况*/
			{
					for(tCounter = 0; tCounter < 3; tCounter++)
					{
							if((messageTimeTable[tCounter].start == 1)&&\
								(messageTimeTable[tCounter].accumulatedTime >\
									messageTimeTable[tCounter].overFlowTime))
							{
									timeOutFlag = 1;	/*表明已经超时了 接下来的判断就没有意义了*/
									
									/*通知状态机 发送超时信号 形式是进入触发队列*/
									ConditionInsert(&mvcTriggerQueue, 0xEB);
								
									/*关闭此次计时*/
									memset(messageTimeTable,0,sizeof(messageTimeTable));
							}
					}
			}
			else		/*成功接受了*/
			{
					/*通知状态机 发送成功接收信号*/
					ConditionInsert(&mvcTriggerQueue, 0xEC);
			}
	}  /*if(startFlag == 1)*/
	else
	{
			/*没有要检测的报文 就什么也不做*/
	}
#endif
}
/****************************************************
函数名：conditionInsert
形参：无
返回值：无
函数功能：在此函数中，向指定的条件队列插入一个条件编码
****************************************************/
void MvcConditionInsert(conditionQueue * pC ,uint8_t conditionCode)
{
		pC->queue[pC->size] = conditionCode; 
		pC->size++;
}

/****************************************************
函数名：ActionExecutor（之所以不叫actionUpdate 是因为不用时间消息驱动 是立刻执行的）
形参：无
返回值：无
函数功能：在此函数中 主要接收conditionUpdate传来的各种
动作执行消息，这种消息主要是编码，所以编码一定要和动作
映射起来。由于动作执行的时间是极短的，因此不会对实时性
产生影响。此执行机构作为状态机的执行方面 十分重要
主要的动作有：1 can帧发送
							2 megmeet操作电压电流     这个通过表格已经解决
							3 继电器开关
							4 。。。。
****************************************************/
void MvcActionExecutor(struct message actionMessage)		/*动作类型千差万别 cmd就是编码*/
{
		switch(actionMessage.parameter_1)
		{
			case 0xc1:
				LockSet(LOCK);
				stateMachineOpen();
				break;
			case 0xc2:
				stateMachineClose();
				break;
			case 0xc3:
				WalletWrite(transactionTable.cardStatus.rxWalletValue - transactionTable.accumulateMoney);
				LockSet(UNLOCK);
				usart1SendByte(0xba);
				break;
			case 0xc4:
				/*记录此次充电过程*/
				break;
			default:break;
		}
		
}
/****************************************************
函数名：GetmvcActiveState 返回当前激活状态 API供外部调用
形参：无
返回值：无
函数功能：
****************************************************/
uint8_t MvcGetActiveState(void)
{
		return mvcActiveState;
}

/****************************************************
函数名：MvcGetSuperState 返回当前所属的宏观状态 
				此函数根据的是从属表，查表得出宏观状态
				从属表左边是宏观状态 右边是微观状态 代表从属关系
形参：无
返回值：无
函数功能：
****************************************************/
uint8_t subordinateTable[10][2] = 
{
		{0, 1},
		{0, 2},
		{0, 6},
		{1, 10},
		{2, 14},
		{2, 18},
		{3, 22},
};
uint8_t MvcGetSuperState(void)
{
		uint8_t counter;
		for(counter = 0; counter < 7; counter++)
		{
					if(subordinateTable[counter][1] == mvcActiveState)
					{
							  transactionTable.transactionStatus = subordinateTable[counter][0];
								return subordinateTable[counter][0];
					}
		}
}

/****************************************************
函数名：MvcGetTransation 返回当前事务结构体 供外部调用API
形参：无
返回值：无
函数功能：
****************************************************/
TRANSACTION MvcGetTransation()
{
		return transactionTable;
}
/****************************************************
函数名：MvcSetChargeValue 设置充电值 供外部调用API
形参：无
返回值：无
函数功能：0电量 1时间 2金额 3自动
****************************************************/
void MvcSetChargeValue(uint32_t value)
{
		switch(transactionTable.chargeMode)
		{
			case 0:	/*电量 0.1度*/
				transactionTable.chargeElectricity = value;	
				break;
			case 1:	/*时间 分*/
				transactionTable.chargeTime = value;
				break;
			case 2:	/*金额 角*/
				transactionTable.chargeMoney = value;
				break;
			default:break;
		}
}
/****************************************************
函数名：MvcSetChargeMode 设置充电值 供外部调用API
形参：无
返回值：无
函数功能：0电量 1时间 2金额 3自动
****************************************************/
void MvcSetChargeMode(uint8_t mode)
{
	transactionTable.chargeMode = mode;
}