/**
  ******************************************************************************
  * @file    pgn.h 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Evaluation board specific configuration file.
  ******************************************************************************
  * @attention
  *
  * 这个文件描述与所有BMS相关的定义 也就是参数表+BMS解析函数
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PGN_H
#define __PGN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"	
#include "message.h"
#include "string.h"
#include "can.h"
/* Exported types ------------------------------------------------------------*/
/*这个是所有BMS交互时 使用的参数表 */
typedef struct
{
	uint32_t   SPN2600;
	uint16_t   SPN2601;
	uint8_t    SPN2560;
	uint32_t   SPN2561;
	uint32_t   SPN2562;
	uint32_t   SPN2565;
	uint8_t   SPN2566;
	uint16_t  SPN2567;
	uint16_t  SPN2568;
	uint32_t  SPN2569;
	uint32_t  SPN2570;
	uint8_t   SPN2571[3];
	uint8_t   SPN2572[3];
	uint8_t   SPN2573;
	uint8_t   SPN2574;
	uint8_t   SPN2575[17];
	uint8_t   SPN2576[8];
	uint16_t  SPN2816;
	uint16_t  SPN2817;
	uint16_t  SPN2818;
	uint16_t  SPN2819;
	uint8_t   SPN2820;
	uint16_t  SPN2821;
	uint16_t  SPN2822;
	uint8_t	  SPN2823[7];
	uint16_t  SPN2824;
	uint16_t  SPN2825;
	uint16_t  SPN2826;
	uint16_t  SPN2827;
	uint8_t   SPN2829;
	uint8_t   SPN2830;
	uint16_t  SPN3072;
	uint16_t  SPN3073;
	uint8_t   SPN3074;
	uint16_t  SPN3075;
	uint16_t  SPN3076;
	uint16_t  SPN3077;
	uint8_t   SPN3078;
	uint16_t  SPN3079;
	uint16_t  SPN3081;
	uint16_t  SPN3082;
	uint16_t  SPN3083;
	uint8_t   SPN3929;
	uint8_t   SPN3085;
	uint8_t   SPN3086;
	uint8_t   SPN3087;
	uint8_t   SPN3088;
	uint8_t   SPN3089;
	uint8_t   SPN3090;
	uint8_t   SPN3091;
	uint8_t   SPN3092;
	uint8_t   SPN3093;
	uint8_t   SPN3094;
	uint8_t   SPN3095;
	uint8_t   SPN3096;
	uint8_t   SPN3101[511];
	uint8_t   SPN3361[128];
	uint8_t   SPN3491[16]; 
	uint8_t   SPN3511;
	uint16_t  SPN3512;
	uint32_t  SPN3513;
	uint8_t   SPN3521;
	uint16_t  SPN3522;
	uint8_t   SPN3523;
	uint8_t   SPN3601;
	uint16_t  SPN3602;
	uint16_t  SPN3603;
	uint8_t   SPN3604;
	uint8_t   SPN3605;
	uint16_t  SPN3611;
	uint16_t  SPN3612;
	uint32_t  SPN3613;
	uint8_t   SPN3901;  
	uint8_t   SPN3902; 
	uint8_t   SPN3903; 
	uint8_t   SPN3904; 
	uint8_t   SPN3905; 
	uint8_t   SPN3906; 
	uint8_t   SPN3907;
	uint8_t   SPN3921;
	uint8_t   SPN3922;
	uint8_t   SPN3923;
	uint8_t   SPN3924;
	uint8_t   SPN3925;
	uint8_t   SPN3926;
	uint8_t   SPN3927;
}pgnGroup;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define CHM 0
#define BHM 1
#define CRM 2
#define BRM 3
#define BCP 4
#define CTS 5
#define CML 6
#define BRO 7
#define CRO 8
#define BCL 9
#define BCS 10
#define CCS 11
#define BSM 12
#define BMV 13
#define BMT 14
#define BSP 15
#define BST 16
#define CST 17
#define BSD 18
#define CSD 19
#define BEM 20
#define CEM 21

/* Exported functions ------------------------------------------------------- */
void NormalMessageParse(CanRxMsg rxMessage);
void LongMessageParse(struct linkStatus * pD);
void FormulaOnPgnGroup(void);
void ActionHandle(struct message txMessage);
#endif /* __PGN_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
