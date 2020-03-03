/***************************************************************************************
 * ����    ��W5500�Ķ˿�0�����ڷ����ģʽ:��TCP&UDP���Թ��ߡ��ϴ����Ŀͻ�����������������,
 *			 ���ӳɹ��󣬷���˶�ʱ���ͻ��˷����ַ���"\r\nWelcome To YiXinElec!\r\n",
 *			 ͬʱ�����յ��ͻ��˷��������ݻط����ͻ��ˡ�
 * ʵ��ƽ̨���û�STM32������ + YIXIN_W5500��̫��(TCP/IP)ģ��
 * Ӳ�����ӣ�  PC5 -> W5500_RST      
 *             PA4 -> W5500_SCS      
 *             PA5 -> W5500_SCK    
 *             PA6 -> W5500_MISO    
 *             PA7 -> W5500_MOSI    
 * ��汾  ��ST_v3.5

 * �Ա�    ��http://yixindianzikeji.taobao.com/
***************************************************************************************/

/*�����������*/
//���أ�192.168.1.1
//����:	255.255.255.0
//������ַ��0C 29 AB 7C 00 01
//����IP��ַ:192.168.1.199
//�˿�0�Ķ˿ںţ�5000
//�˿�0��Ŀ��IP��ַ��192.168.1.190
//�˿�0��Ŀ�Ķ˿ںţ�6000

#include "stm32f10x.h"		
#include "W5500.h"			
#include "string.h"
#include "socket.h"
#include "usart.h"

unsigned int Timer2_Counter=0; //Timer2��ʱ����������(ms)
unsigned int W5500_Send_W5500Delay_Counter=0; //W5500������ʱ��������(ms)

/*******************************************************************************
* ������  : W5500_Initialization
* ����    : W5500��ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void W5500_Initialization(void)
{
		W5500_Init();			//��ʼ��W5500 ���������ò���
		Detect_Gateway();	//������ط����� 
		Socket_Init(0);		//ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
}

/*******************************************************************************
* ������  : Load_Net_Parameters
* ����    : װ��������� ע�� ֻ��װ�� �������� �������� W5500_Init()���е� 
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ���ء����롢������ַ������IP��ַ���˿ںš�Ŀ��IP��ַ��
						Ŀ�Ķ˿ںš��˿ڹ���ģʽ��TCP UDP��
*******************************************************************************/
void Load_Net_Parameters(void)
{
	Gateway_IP[0] = 192;//�������ز���
	Gateway_IP[1] = 168;
	Gateway_IP[2] = 1;
	Gateway_IP[3] = 1;

	Sub_Mask[0]=255;//������������
	Sub_Mask[1]=255;
	Sub_Mask[2]=255;
	Sub_Mask[3]=0;

	Phy_Addr[0]=0x0c;//����������ַ
	Phy_Addr[1]=0x29;
	Phy_Addr[2]=0xab;
	Phy_Addr[3]=0x7c;
	Phy_Addr[4]=0x00;
	Phy_Addr[5]=0x01;

	IP_Addr[0]=192;//���ر���IP��ַ
	IP_Addr[1]=168;
	IP_Addr[2]=1;
	IP_Addr[3]=199;

	S0_Port[0] = 0x13;//���ض˿�0�Ķ˿ں�5000 
	S0_Port[1] = 0x88;

//	S0_DIP[0]=192;//���ض˿�0��Ŀ��IP��ַ
//	S0_DIP[1]=168;
//	S0_DIP[2]=1;
//	S0_DIP[3]=190;
//	
//	S0_DPort[0] = 0x17;//���ض˿�0��Ŀ�Ķ˿ں�6000
//	S0_DPort[1] = 0x70;

	S0_Mode=TCP_SERVER;//���ض˿�0�Ĺ���ģʽ,TCP�����ģʽ
}

/*******************************************************************************
* ������  : W5500_Socket_Set
* ����    : W5500�˿ڳ�ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : �ֱ�����4���˿�,���ݶ˿ڹ���ģʽ,���˿�����TCP��������TCP�ͻ��˻�UDPģʽ.
*			�Ӷ˿�״̬�ֽ�Socket_State�����ж϶˿ڵĹ������
*******************************************************************************/
void W5500_Socket_Set(void)
{
	if(S0_State==0)//�˿�0��ʼ������
	{
		if(S0_Mode==TCP_SERVER)//TCP������ģʽ 
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else if(S0_Mode==TCP_CLIENT)//TCP�ͻ���ģʽ 
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else//UDPģʽ 
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}
}

/*******************************************************************************
* ������  : Process_Socket_Data
* ����    : W5500���ղ����ͽ��յ�������
* ����    : s:�˿ں�
* ���    : ��
* ����ֵ  : ��
* ˵��    : �������ȵ���S_rx_process()��W5500�Ķ˿ڽ������ݻ�������ȡ����,
*			Ȼ�󽫶�ȡ�����ݴ�Rx_Buffer������Temp_Buffer���������д�����
*			������ϣ������ݴ�Temp_Buffer������Tx_Buffer������������S_tx_process()
*			�������ݡ�
*******************************************************************************/
void Process_Socket_Data(SOCKET s)
{
		unsigned short size;
		size=Read_SOCK_Data_Buffer(s, Rx_Buffer);
		memcpy(Tx_Buffer, Rx_Buffer, size);			
		Write_SOCK_Data_Buffer(s, Tx_Buffer, size);
}

/*******************************************************************************
* ������  : main
* ����    : ���������û������main������ʼ����
* ����    : ��
* ���    : ��
* ����ֵ  : int:����ֵΪһ��16λ������
* ˵��    : ��
*******************************************************************************/
int W5500Init(void)
{
	System_Initialization();	//STM32ϵͳ��ʼ�� ����RST��INT������
	Load_Net_Parameters();		//װ���������	����IP �͹���ģʽTCP�ȵ� 
	W5500_Hardware_Reset();		//Ӳ����λW5500
	W5500_Initialization();		//W5500��ʼ������ ����Ŀǰ�ǿ���pingͨ��
	W5500_Socket_Set();				//W5500�˿ڳ�ʼ������
}


/*******************************************************************************
* ������  : main
* ����    : ���������û������main������ʼ����
* ����    : ��
* ���    : ��
* ����ֵ  : int:����ֵΪһ��16λ������
* ˵��    : ��
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
				if((Read_W5500_1Byte(PHYCFGR)&LINK)==1)	/*Ӳ����������*/
				{
						if(vW5500TimerGetValue() > 5)
						{
								wAppState = STATE_INT_PRO;
								W5500_Socket_Set();	/*ʱ��Ƚϳ� ����Ҫ�������*/
						}		
				}
				break;
			case STATE_INT_PRO:
				W5500_Interrupt_Process();		/*W5500�ɼ��Ĵ���״̬��������*/
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
				if((S0_Data & S_RECEIVE) == S_RECEIVE)//���Socket0���յ�����
				{
						S0_Data&=~S_RECEIVE;				/*����״̬���־λ*/
						Process_Socket_Data(0);			/*W5500���ղ����ͽ��յ�������*/
				}
				vW5500TimerSetValue(0);
				wAppState = STATE_WAIT;
				break;
			default:break;
		}

	
//		if(S0_State == (S_INIT|S_CONN))			/*�����ж�Ŀǰ������״̬*/
//		{
//				S0_Data&=~S_TRANSMITOK;		/*�������״̬���*/
//				memcpy(Tx_Buffer, "\r\nWelcome To YiXinElec!\r\n", 23);	
//				Write_SOCK_Data_Buffer(0, Tx_Buffer, 23);//ָ��Socket(0~7)�������ݴ���,�˿�0����23�ֽ�����
//		}
}
/*******************************************************************************
* ������  : System_Initialization
* ����    : STM32ϵͳ��ʼ������ ��Ҫ�������ŵ����� 
* ����    : ��
* ���    : ��
* ����    : �� 
* ˵��    : ��
*******************************************************************************/
void System_Initialization(void)
{
		W5500_GPIO_Configuration();	//W5500 GPIO��ʼ������	
}

