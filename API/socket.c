/***************************************************************************************
 * 描述    ：W5500的端口0工作在服务端模式:《TCP&UDP测试工具》上创建的客户端主动与服务端连接,
 *			 连接成功后，服务端定时给客户端发送字符串"\r\nWelcome To YiXinElec!\r\n",
 *			 同时将接收到客户端发来的数据回发给客户端。
 * 实验平台：用户STM32开发板 + YIXIN_W5500以太网(TCP/IP)模块
 * 硬件连接：  PC5 -> W5500_RST      
 *             PA4 -> W5500_SCS      
 *             PA5 -> W5500_SCK    
 *             PA6 -> W5500_MISO    
 *             PA7 -> W5500_MOSI    
 * 库版本  ：ST_v3.5

 * 淘宝    ：http://yixindianzikeji.taobao.com/
***************************************************************************************/

/*例程网络参数*/
//网关：192.168.1.1
//掩码:	255.255.255.0
//物理地址：0C 29 AB 7C 00 01
//本机IP地址:192.168.1.199
//端口0的端口号：5000
//端口0的目的IP地址：192.168.1.190
//端口0的目的端口号：6000

#include "stm32f10x.h"		
#include "W5500.h"			
#include "string.h"
#include "socket.h"
#include "usart.h"

unsigned int Timer2_Counter=0; //Timer2定时器计数变量(ms)
unsigned int W5500_Send_W5500Delay_Counter=0; //W5500发送延时计数变量(ms)

/*******************************************************************************
* 函数名  : W5500_Initialization
* 描述    : W5500初始货配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_Initialization(void)
{
		W5500_Init();			//初始化W5500 真正的设置参数
		Detect_Gateway();	//检查网关服务器 
		Socket_Init(0);		//指定Socket(0~7)初始化,初始化端口0
}

/*******************************************************************************
* 函数名  : Load_Net_Parameters
* 描述    : 装载网络参数 注意 只是装载 不是设置 设置是在 W5500_Init()进行的 
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、
						目的端口号、端口工作模式（TCP UDP）
*******************************************************************************/
void Load_Net_Parameters(void)
{
	Gateway_IP[0] = 192;//加载网关参数
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 1;
	Gateway_IP[3] = 1;

	Sub_Mask[0]=255;//加载子网掩码
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	Phy_Addr[0]=0x0c;//加载物理地址
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;

	IP_Addr[0]=192;//加载本机IP地址
	IP_Addr[1]=168;
	IP_Addr[2]=1;
	IP_Addr[3]=199;

	S0_Port[0] = 0x13;//加载端口0的端口号5000 
	S0_Port[1] = 0x88;

//	S0_DIP[0]=192;//加载端口0的目的IP地址
//	S0_DIP[1]=168;
//	S0_DIP[2]=1;
//	S0_DIP[3]=190;
//	
//	S0_DPort[0] = 0x17;//加载端口0的目的端口号6000
//	S0_DPort[1] = 0x70;

	S0_Mode=TCP_SERVER;//加载端口0的工作模式,TCP服务端模式
}

/*******************************************************************************
* 函数名  : W5500_Socket_Set
* 描述    : W5500端口初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 分别设置4个端口,根据端口工作模式,将端口置于TCP服务器、TCP客户端或UDP模式.
*			从端口状态字节Socket_State可以判断端口的工作情况
*******************************************************************************/
void W5500_Socket_Set(void)
{
	if(S0_State==0)//端口0初始化配置
	{
		if(S0_Mode==TCP_SERVER)//TCP服务器模式 
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else if(S0_Mode==TCP_CLIENT)//TCP客户端模式 
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else//UDP模式 
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}
}

/*******************************************************************************
* 函数名  : Process_Socket_Data
* 描述    : W5500接收并发送接收到的数据
* 输入    : s:端口号
* 输出    : 无
* 返回值  : 无
* 说明    : 本过程先调用S_rx_process()从W5500的端口接收数据缓冲区读取数据,
*			然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。
*			处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
*			发送数据。
*******************************************************************************/
void Process_Socket_Data(SOCKET s)
{
		unsigned short size;
		size=Read_SOCK_Data_Buffer(s, Rx_Buffer);
		memcpy(Tx_Buffer, Rx_Buffer, size);			
		Write_SOCK_Data_Buffer(s, Tx_Buffer, size);
}

/*******************************************************************************
* 函数名  : main
* 描述    : 主函数，用户程序从main函数开始运行
* 输入    : 无
* 输出    : 无
* 返回值  : int:返回值为一个16位整形数
* 说明    : 无
*******************************************************************************/
int W5500Init(void)
{
	System_Initialization();	//STM32系统初始化 引脚RST和INT的配置
	Load_Net_Parameters();		//装载网络参数	配置IP 和工作模式TCP等等 
	W5500_Hardware_Reset();		//硬件复位W5500
	W5500_Initialization();		//W5500初始化配置 截至目前是可以ping通的
	W5500_Socket_Set();				//W5500端口初始化配置
}


/*******************************************************************************
* 函数名  : main
* 描述    : 主函数，用户程序从main函数开始运行
* 输入    : 无
* 输出    : 无
* 返回值  : int:返回值为一个16位整形数
* 说明    : 无
*******************************************************************************/
#define STATE_WAIT					0
#define STATE_INT_PRO				1
#define STATE_INT_STATUS    2
#define STATE_DATA_PRO			3

static uint8_t wAppState = STATE_WAIT;    // State tracking variable

void W5500Task(void)
{
		switch(wAppState)
		{
			case STATE_WAIT:
				if((Read_W5500_1Byte(PHYCFGR)&LINK)==1)	/*硬件连接上了*/
				{
						if(vW5500TimerGetValue() > 5)
						{
								wAppState = STATE_INT_PRO;
								W5500_Socket_Set();	/*时间比较长 还需要继续拆分*/
						}		
				}
				break;
			case STATE_INT_PRO:
				W5500_Interrupt_Process();		/*W5500采集寄存器状态到程序中*/
				wAppState = STATE_INT_STATUS;
				break;
			case STATE_INT_STATUS:
				if(SIRStatus() == 0)
				{
						wAppState = STATE_DATA_PRO;
				}
				else
				{
						wAppState = STATE_INT_PRO;
				}
				break;
			case STATE_DATA_PRO:
				if((S0_Data & S_RECEIVE) == S_RECEIVE)//如果Socket0接收到数据
				{
						S0_Data&=~S_RECEIVE;				/*接收状态清标志位*/
						Process_Socket_Data(0);			/*W5500接收并发送接收到的数据*/
				}
				vW5500TimerSetValue(0);
				wAppState = STATE_WAIT;
				break;
			default:break;
		}

	
//		if(S0_State == (S_INIT|S_CONN))			/*首先判断目前的连接状态*/
//		{
//				S0_Data&=~S_TRANSMITOK;		/*发送完成状态清除*/
//				memcpy(Tx_Buffer, "\r\nWelcome To YiXinElec!\r\n", 23);	
//				Write_SOCK_Data_Buffer(0, Tx_Buffer, 23);//指定Socket(0~7)发送数据处理,端口0发送23字节数据
//		}
}
/*******************************************************************************
* 函数名  : System_Initialization
* 描述    : STM32系统初始化函数 主要就是引脚的配置 
* 输入    : 无
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void System_Initialization(void)
{
		W5500_GPIO_Configuration();	//W5500 GPIO初始化配置	
}


