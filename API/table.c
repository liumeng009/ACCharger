#include "table.h"
#include "string.h"
#include "pgn.h"

#define STATE_SIZE            30		//总共有多少个state
#define STATE_TRANSITION_SIZE 23  	//迁移条目的个数

conditionQueue cantriggerQueue;
struct message transitionMessage;	//迁移用的消息实例

/****************************************************
变量名：stateMachineSwitcher
描述：状态机的开关 0：关闭 1：启动
****************************************************/
uint32_t stateMachineSwitcher;

/****************************************************
变量名：timeBase 
描述：以1ms为单位的时间轴 最长可以计算到49.71026天/充电
用来给每一个状态的时间戳提供一个基准，当BMS开始时启动
在BMS结束时清0，为状态机加入时间属性
****************************************************/
uint32_t timeBase;	

/****************************************************
变量名：activeState 
描述：活动的状态
****************************************************/
uint8_t activeState;	

/****************************************************
变量名：stateChanged 
描述：1；状态已经改变 0；状态没有改变
置一动作是在迁移表中进行
清零动作是在时间表中进行（一定可以清零）
****************************************************/
uint8_t stateChanged;	

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
变量名：stateTransitionTable  状态迁移表
描述：由n个stateTransitionItem组成，描述了所有的状态
迁移，包括逻辑上的和时间上的 
----<原状态，条件编码，目的状态，动作>-----
****************************************************/
stateTransitionItem stateTransitionTable[STATE_TRANSITION_SIZE] = 
{
		{0, 	 0, 1,    0},
		{1, 0xe1, 2, 0xb3},
		{1, 0xe2, 2, 0xb3},
		{1, 0xe3, 2, 0xb3},
		{2, 0xe2, 3, 0xb3},
		{3, 0xe3, 2, 0xb3},
		{2, 0xe4, 1, 0xb3},
		{2, 	 2, 5, 0xb1},
		{3,    3, 6, 0xb1},
		{6, 0xe5, 3, 0xb3},
		{6,    6, 6, 0xb2},
		{6, 0xe6, 5, 0xb1},
		{6, 0xea, 7, 0xb3 },
		{5, 0xe7, 4, 0xb1},
		{4, 0xe8, 1, 0xb3},
		{4, 0xe9, 5, 0xb1},
		{5, 	 5, 2, 0xb3},
		/*长超时情况*/
		{1, 0xf1, 7, 0},
		{2, 0xf2, 7, 0},
		{3, 0xf3, 7, 0},
		{4, 0xf4, 7, 0},
		{5, 0xf5, 7, 0},
		{2, 0xf6, 7, 0},
};
#if 0
stateTransitionItem stateTransitionTable[STATE_TRANSITION_SIZE] = 
{
	{0, 0, 1, 0},							/*0状态超时就进入1状态 执行动作为0（无）*/
	{1, 1, 2, 0xc1},					/*1状态超时10ms就进入2状态 执行发送CHM*/
	{2, 2, 1, 0},							/*2状态超时250ms就进入1状态*/
	{2, 0xb1, 4, 0},					/*2状态接收到BHM 就进入4状态*/
	{2, 0xe1, 3, 0},					/*2状态长超时5s就进入3状态*/
	{3, 3, 4, 0},							/*3状态超时10ms就进入4状态 相当于忽略了*/
	
	{4, 4, 5, 0xc2},					/*4状态超时10ms就进入5状态 执行发送CRM 0x00*/
	{5, 0xb2, 6, 0},					/*5状态接收到BRM 就进入4状态*/
	{5, 5, 4, 0},							/*5状态超时250ms就进入4状态*/
	{5, 0xe2, 8, 0},					/*5状态长超时5S就进入8状态*/
	{6, 6, 7, 0xc2},					/*4状态超时10ms就进入5状态 执行发送CRM 0xAA*/
	{5, 0xe2, 8, 0},
//	{8, 8, ERROR, ERROR_ACTION},/*8状态超时10ms就进握手失败告警状态 执行告警动作*/
	
	{7, 7, 6, 0},							/*7状态超时250ms就进入6状态 重新发送报文*/
	{7, 0xe3, 10, 0},					/*7状态长超时5s就进入10状态*/
	{7, 0xb3, 8, 0},					/*7状态收到BCP 就进入8状态*/
	{8, 8, 9, 0xca},					/*8状态超时10ms 就进入9状态 同时执行发送CTS和CML*/
	{9, 0xe4, 11, 0},					/*9状态长超时5s 就进入11状态*/
	{9, 9, 8, 0},							/*9状态超时250ms 就进入8状态*/
	{9, 0xb4, 12, 0},					/*9状态收到BRO  就进入12状态*/
	{12, 12, 12, 0xc5},				/*12状态超时250ms  就进入12状态*/
	{12, 0xe5, 14, 0},				/*12状态长超时5s  就进入14状态*/
	{12, 0xba, 13, 0},				/*12状态判断ready  就进入13状态*/
	{13, 13, 15, 0xc5},				/*13状态超时10ms 就进入15状态 执行发送CRO*/
	
	{15, 0xeb, 13, 0},				/*15状态没有收到BCS BCL 就进入13状态*/
	{15, 0xec, 16, 0},				/*15状态收到BCS BCL 就进入16状态*/
	{15, 0xe6, 17, 0},				/*15状态长超时，就进入17状态*/
	{17, 17, 17, 0},					/*17状态超时，就进入17状态*/
	
	{16, 16, 18, 0xc6},				/*16状态超时10ms  就进入18状态 执行发送CCS */
	{18, 0xEC, 19, 0},				/*18状态成功接收就进入19状态 */
	{18, 0xEB, 21, 0},				/*18状态接收超时就进入21状态 */

	
	{19, 0xb8, 20, 0},					/*19状态收到BST 就进入20状态 */
//	{19, MODULECLOSE, 20, 0}, /*19状态停止状态 就进入20状态 */
	{19, 19, 16, 0},					/*19状态超时250ms 就进入16状态 */
	{20, 20, 22, 0xc7},				/*20状态超时10ms 就进入22状态 */
	{22, 22, 20, 0},				  /*22状态超时250ms 就进入20状态 */
	{22, 0xe9, 24, 0},				/*20状态长超时5s 就进入24状态 */
	
	{23, 0xb9, 24, 0},				/*23状态收到BSD 就进入24状态 */
	{23, 23, 20, 0},					/*23状态超时250ms 就进入20状态 */
	{23, 0xea, 25, 0},				/*20状态长超时5s 就进入25状态 */
	{24, 24, 26, 0xc8},				/*24状态超时10ms 就进入26状态 执行发送CSD*/
	
};
#endif
/****************************************************
变量名：stateTimeItem  状态迁移项
描述：记录了状态所有可能的时间戳 通过这个表可以判断
状态是不是超时，经历的次数是不是太多，规定时间内有没有
完成某一动作等等，是迁移表的时间属性。
****************************************************/
typedef struct 
{
	uint32_t firstTime;		//状态最开始时间
	uint32_t latestTime;	//状态最近一次时间
	uint32_t duration;		//状态最近一次持续时间
	uint32_t times;			//状态总共次数
}stateTimeItem;

/****************************************************
变量名：stateTimeTable  状态时间表
描述：由n个stateTimeItem组成，描述了所有的状态
的时间
****************************************************/
stateTimeItem stateTimeTable[STATE_SIZE];

/****************************************************
变量名：conditionsQueue  条件队列
描述：在某个state活动的条件下，所有产生的条件，之所以
用一个队列，是因为条件的产生频率有可能大于迁移更新的频率
为了防止丢掉了条件，所以采用了队列
在发生了成功的迁移之后，要把队列中剩余的条件清零，防止
影响下一个状态，发生逻辑错误
****************************************************/
conditionQueue triggerQueue;

/****************************************************
函数名：tableInit
形参：无
返回值：无
函数功能：在此函数中，初始化状态机的各项变量，回到
状态机最开始的状态 
****************************************************/
void tableInit()
{
	uint8_t counter;
	stateMachineSwitcher = 0; //关闭状态机
	timeBase = 0;			  //时间轴清零
	activeState = 0;		  //初始状态
	for(counter = 0; counter < STATE_SIZE; counter++)
	{
		stateTimeTable[counter].firstTime = 0;
		stateTimeTable[counter].latestTime = 0;
		stateTimeTable[counter].duration = 0;
		stateTimeTable[counter].times = 0;
	}
	triggerQueue.size = 0;	  //清空条件队列
	
	/*状态机所要控制的条件和动作也要进入初始状态*/
	GPIO_ResetBits(GPIOE, GPIO_Pin_3);
	GPIO_ResetBits(GPIOE, GPIO_Pin_4);
}
/****************************************************
变量名：messageTimeItem 报文时间控制表
描述：通过对报文记录时间，来实现对并且逻辑的表达，也就
是说，并且的含义就是同时！，而通过表格可以很方便表达同
时。 start决定哪几个数据同时进行检验，一旦start有效的
项目中，全部times都为1（或大于1），那么表明全部接收到了
然后判断是否超时的情况。同时，检测超时情况如果发生，那
么说明之前的times不满足要求，才会检测到超时的。
只有在表达报文接收之间的逻辑的时候，才会采用此表
如果全部采用此表，那么报文的超时判断就可以提取出来，不
放在状态机里面判断，结构更清晰，也是不错，以后完善
****************************************************/
typedef struct 
{
	uint8_t  start;						//开始标志 0：停止（清0数据） 1：开始
	uint32_t accumulatedTime;	//累积时间
	uint32_t overFlowTime;		//溢出时间
	uint32_t times;						//收到次数
}messageTimeItem;

messageTimeItem messageTimeTable[3] = 
{
	{0 , 0, 50, 0},			/*BCL报文超时控制*/
	{0 , 0, 250, 0},		/*BCS报文超时控制*/
	{0 , 0, 250, 0},		/*BSM报文超时控制*/
};

/****************************************************
函数名：stateMachineOpen
形参：无
返回值：无
函数功能：在此函数中，初始化状态机的各项变量，回到
状态机最开始的状态 
****************************************************/
void stateMachineOpen()
{
	tableInit();
	stateMachineSwitcher = 1; //打开状态机
}

/****************************************************
函数名：stateMachineClose
形参：无
返回值：无
函数功能：在此函数中，初始化状态机的各项变量，回到
状态机最开始的状态 
****************************************************/
void stateMachineClose()
{
	stateMachineSwitcher = 0; //关闭状态机
	tableInit();
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
void stateTransitionUpdate()
{
	uint8_t tCounter = 0;
	uint8_t cCounter = 0;
	
	if(stateMachineSwitcher == 0)	//说明状态机处于关闭状态
		return;
		
	//首先查找条件队列 如果有 继续 如果没有 return
	if(triggerQueue.size == 0)
		return; 
		
	//对表格进行遍历 寻找与activeState相同的条目
	//也就是说 查找activeState的触发条件都是什么
	for(tCounter = 0; tCounter < STATE_TRANSITION_SIZE; tCounter++)
	{
		if(stateTransitionTable[tCounter].sourceState == activeState)
		{	//找到源状态==activeState的迁移项
			for(cCounter = 0 ; cCounter < triggerQueue.size; cCounter++)
			{	
				//和条件队列里的条件一一对比
				if(stateTransitionTable[tCounter].triggerCondition == \
					triggerQueue.queue[cCounter])
				{
					//执行触发后的动作....
					usart1SendByte(0xd1);
					//首先更改活动状态
					activeState = stateTransitionTable[tCounter].targetState;
					//将条件队列清零
					triggerQueue.size = 0;
					//状态改变标志位
					stateChanged = 1;
					//执行某些动作....
					if(stateTransitionTable[tCounter].action != 0) /*确实有有效的动作编码*/
					{
							transitionMessage.destinationAddress = 4;
							transitionMessage.cmd = 2;
							transitionMessage.parameter_1 = stateTransitionTable[tCounter].action;
							messagePost(transitionMessage);
					}
				}
				else	/*迁移条件没有被满足*/
				{
					//通知系统无用的条件数量，用于调试
				}
				if(stateChanged == 1)
					break;
			}
		}
		if(stateChanged == 1)
				break;
	}
	
	/*经过遍历之后没有针对activeState的条件被满足，也就是说
	当下这些在队列里的条件都不是针对activeState的，那么这些
	条件对于状态机来说无意义，应该被舍弃*/
	/*有没有都要清零*/
	triggerQueue.size = 0;
	
}

/****************************************************
函数名：stateTimeUpdate（1ms消息驱动）
形参：无
返回值：无
函数功能：在此函数中，每1ms更新一次timeBase
判断活动的状态是否改变了stateChanged变量
如果变成了新的状态，那么清零新状态的duration
如果变成了新的状态，那么次数times++
只要firstTime为0，那么就记录为timeBase
正在活动的状态的duration++（记录）
正在活动的状态latestTime记录为timeBase
时间表是迁移表的时间属性：
1.定义了每一种状态的详细时间属性
****************************************************/
void stateTimeUpdate()
{
	uint8_t counter;			/*普通变量*/
	
	if(stateMachineSwitcher == 0)	//说明状态机处于关闭状态
		return;

	timeBase++;					//每1ms加一
	if(stateChanged)    //状态表发生了迁移,状态改变了
	{
		stateTimeTable[activeState].duration = 0;
		stateTimeTable[activeState].times++;

		//如果此状态的firstTime为0 
		if(stateTimeTable[activeState].firstTime == 0)
		{
			//打上时间戳
			stateTimeTable[activeState].firstTime = timeBase;
		}
		stateChanged = 0;	//将标志位清零 对于时间控制来说已经用完
	}
	
	stateTimeTable[activeState].duration++; //状态持续周期
	stateTimeTable[activeState].latestTime = timeBase; //更新最新时间
	
	/*以上是状态表的时间控制，下面是报文表的时间控制*/
	/*在timeUpdate函数中 表格只是用来记录时间，而不进行时间的判断*/
	for(counter = 0 ; counter < 3; counter ++)
	{
			if(messageTimeTable[counter].start == 1)
			{
					messageTimeTable[counter].accumulatedTime++;
			}
			else
			{
					messageTimeTable[counter].accumulatedTime = 0;	/*每1ms都清零*/
					messageTimeTable[counter].times = 0;
			}
	}
	
}
/****************************************************
变量名：timeOutTable 
描述：停留在每一种状态的timeOut时间 此处根据STATE_SIZE初始化值
			定义数组的长度
****************************************************/
/*const uint32_t timeOutTable[STATE_SIZE] = 
{
 10,10,250,10,10,250,10,10,10,10,
 10,10,250,10,10,250,10,10,10,10,
 10,10,250,10,10,250,10,10,10,10
};*/
const uint32_t timeOutTable[STATE_SIZE] = 
{
 1000,1000,1000,1000,1000,1000,1000,10,10,10,
 10,10,250,10,10,250,10,10,10,10,
 10,10,250,10,10,250,10,10,10,10
};
/****************************************************
变量名：conditionTable 
描述：根据PE3 PE4 监测点1 监测点4的值 映射到 条件编码
****************************************************/
const uint8_t conditionTable[14][5] = 
{//PE3 PE4 1  4 code
	{0, 0, 1,  1 ,  0xe1},
	{0, 0, 1,  3 ,  0xe2},
	{0, 0, 1,  0 ,  0},
	{0, 0, 1,  2 ,  0xe3},
	{0, 0, 0,  1 ,  0xe4},
	{0, 0, 0,  2 ,  0xe4},
	{0, 0, 0,  3 ,  0xe4},
	{1, 1, 1,  5 ,  0xe6},
	{1, 0, 0,  4 ,  0xe7},
	{1, 0, 0,  5 ,  0xe7},
	{1, 0, 0,  6 ,  0xe7},
	{1, 0, 0,  0 ,  0},
	{1, 0, 1,  5 ,  0xe9},
	{1, 1, 0,  4 ,  0xea}
};
/****************************************************
函数名：AdcValueToState 根据PE3的值和ADC平均值来确定CP CC目前的状态

形参：无
返回值：无
函数功能：在此函数中 按照PWM和电平两套标准确定CP的状态
电平: 12V   1    9V   2     6V    3    其他: 0
PWM:  12V   4    9V   5     6V    6
****************************************************/
uint8_t AdcValueToCPstate()
{
		uint8_t PE3;
		uint16_t adcAVGValue;
		//首先判断PWM还是电平
		PE3 = GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_3);
		adcAVGValue = adc3Value();
		//根据各自标准判断是12V 9V 6V 还是其他
		if(PE3 == 0)	
		{
				/*目前处于电平输出状态*/
				if(adcAVGValue>3000 && adcAVGValue<3430)
						return 1;
				else if(adcAVGValue>2500 && adcAVGValue<2900)
						return 2;
				else if(adcAVGValue>1500 && adcAVGValue<2000)
						return 3;
				else
						return 0;
		}
		else
		{
				/*目前处于PWM输出状态*/
				if(adcAVGValue>2700 && adcAVGValue<3000)
						return 4;
				else if(adcAVGValue>2350 && adcAVGValue<2500)
						return 5;
				else if(adcAVGValue>1400 && adcAVGValue<1700)
						return 6;
				else
						return 0;
		}
}
uint8_t AdcValueToCCstate()
{
		uint16_t adcAVGValue;
		//首先判断PWM还是电平	
		adcAVGValue = adc3Value();
	
		if(adcAVGValue<2900 && adcAVGValue >600)
				return 1;
		else
				return 0;
		
		
		//根据各自标准判断是12V 9V 6V 还是其他
		if(adcAVGValue>1800 && adcAVGValue<2500)
			return 0;
		else if(adcAVGValue>0 && adcAVGValue< 200)
			return 1;
		else
			return 3;
}
/****************************************************
函数名：conditionUpdate 条件收集（10ms消息驱动）
形参：无
返回值：无
函数功能：在此函数中，每1ms产生一个条件 这是定的
时间也是可以产生迁移条件的，在此函数中也要把时间考虑
因此可能产生两个条件 或者更多
总结就是2个 :逻辑迁移和时间迁移！
****************************************************/
void conditionUpdate()
{
	uint8_t cCounter;		//条件用变量
	uint8_t tCounter;		//时间用变量
	uint8_t startFlag;	//有效标志
	uint8_t resultFlag;
	uint8_t timeOutFlag;
	uint8_t PE3,PE4,M1,M4;
	
	if(stateMachineSwitcher == 0)	//说明状态机处于关闭状态
		return;
	
	/*提取实时条件*/
	PE3 = GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_3);
	PE4 = GPIO_ReadOutputDataBit(GPIOE, GPIO_Pin_4);
	M1 = AdcValueToCCstate();
	M4 = AdcValueToCPstate();
	//扫描逻辑条件
	for(cCounter = 0; cCounter < 14; cCounter++)
	{
		if(PE3 == conditionTable[cCounter][0] && \
		   PE4 == conditionTable[cCounter][1] && \
		    M1 == conditionTable[cCounter][2] && \
		    M4 == conditionTable[cCounter][3] )
		{
			//将触发条件写入
			triggerQueue.queue[triggerQueue.size] = conditionTable[cCounter][4];
			triggerQueue.size++;
			break; //找到一个条件就退出 因为多个条件无意义
		}
	}
	
	/*扫描时间条件（针对activeState的）*/
	/*时间条件可以使duration持续时间太长了，也可以是不连接状态之间
	的时间太长了，更可以是firsttime到某另一状态的最新时间，使用起
	来比较灵活，除了持续时间duration，其他情况由于用数据结构表示的
	话，会比较复杂，所以因各自程序的差异，这里自由实现。duration还
	是用表格表示。*/
	
	/*首先检查durition的情况*/
	if(stateTimeTable[activeState].duration >= timeOutTable[activeState])
	{
			//产生超时的条件编码...
			ConditionInsert(&triggerQueue, activeState); /*activeState=条件编码*/
	}
	
	//再继续检查各种跨状态超时的特殊情况...

	if(activeState == 1)
	{
			if((stateTimeTable[1].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF1); /*条件编码从0xe1开始*/
			}
	}
	if(activeState == 2)
	{
			if((stateTimeTable[2].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF2); 
			}
	}
	if(activeState == 3)
	{
			if((stateTimeTable[3].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF3); 
			}
	}
	if(activeState == 4)
	{
			if((stateTimeTable[4].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF4); 
			}
	}
	if(activeState == 5)
	{
			if((stateTimeTable[5].latestTime-stateTimeTable[1].firstTime)> 600000)
			{
					ConditionInsert(&triggerQueue, 0xF5); 
			}
	}
	
	/*每个状态的次数总限制*/
	if(activeState == 2)
	{
			if(stateTimeTable[2].times > 10)
			{
					ConditionInsert(&triggerQueue, 0xF6); 
			}
	}
	//..

#if 0	
	/*这里收集can接收报文的情况*/
	while(cantriggerQueue.size--)
	{
			ConditionInsert(&triggerQueue, cantriggerQueue.queue[cantriggerQueue.size-1]);
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
									ConditionInsert(&triggerQueue, 0xEB);
								
									/*关闭此次计时*/
									memset(messageTimeTable,0,sizeof(messageTimeTable));
							}
					}
			}
			else		/*成功接受了*/
			{
					/*通知状态机 发送成功接收信号*/
					ConditionInsert(&triggerQueue, 0xEC);
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
void ConditionInsert(conditionQueue * pC ,uint8_t conditionCode)
{
		pC->queue[pC->size] = conditionCode; 
		pC->size++;
}
/****************************************************
函数名：messageRecvUpdate 供外部调用API 就是告诉哪一种报文受到了
				就清0计时器。记一下次数
形参：counter 指的是哪一种报文 BCL BCS 或者其他
返回值：无
函数功能：在此函数中 代表收到了相关报文后 更新messageTimeTable
的操作，一般在外部调用！
调用以后 首先times肯定要加一
然后把时间清零，重新开始计时
****************************************************/
void messageRecvUpdate(uint8_t counter)
{
		messageTimeTable[counter].accumulatedTime = 0;
		messageTimeTable[counter].times++;
}

/****************************************************
函数名：messageTimeTableSet 供外部调用API 就是说哪一种报文进入
或关系的运算。并初始化数据。解除计算关系是在超时后自动进行的。
形参：无
返回值：无
函数功能：在此函数中 代表收到了相关报文后 更新messageTimeTable
的操作，一般在外部调用！
调用以后 首先times肯定要加一
然后把时间清零，重新开始计时
****************************************************/
void messageTimeTableSet(uint8_t counter)
{
		messageTimeTable[counter].start = 1;
		/*顺带把其他数据清0一遍*/
		messageTimeTable[counter].accumulatedTime = 0;
		messageTimeTable[counter].times = 0;
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
void ActionExecutor(struct message actionMessage)		/*动作类型千差万别 cmd就是编码*/
{
		/*调用PGN.c中的发送函数 过滤*/
		//ActionHandle(actionMessage); /*canTask只识别自己认识的编码 对于不认识的编码忽略*/
		switch(actionMessage.parameter_1)
		{
			case 0xb1:
				GPIO_SetBits(GPIOE,GPIO_Pin_3);
				GPIO_ResetBits(GPIOE,GPIO_Pin_4);
				break;
			case 0xb2:
				GPIO_SetBits(GPIOE,GPIO_Pin_3);
				GPIO_SetBits(GPIOE,GPIO_Pin_4);
				break;
			case 0xb3:
				GPIO_ResetBits(GPIOE,GPIO_Pin_3);
				GPIO_ResetBits(GPIOE,GPIO_Pin_4);
				break;
			case 0xb4:
				GPIO_ResetBits(GPIOE,GPIO_Pin_3);
				GPIO_SetBits(GPIOE,GPIO_Pin_4);
				break;
			default:break;
		}
		
}
/****************************************************
函数名：GetActiveState 返回当前激活状态 API供外部调用
形参：无
返回值：无
函数功能：
****************************************************/
uint8_t GetActiveState(void)
{
		return activeState;
}

/**
* @brief  GetSuperState()获得状态机的状态 1:关机 2：准备充电 3：正在充电 4:充电完成
						
  * @param  
  * @param  
  * @retval None
  */
uint8_t GetSuperState(void)
{
		if(stateMachineSwitcher == 0)
			return 1;
		else
		{
			if(activeState == 7)
				return 4;
			else if(activeState == 6)
				return 3;
			else 
				return 2;
		}
}