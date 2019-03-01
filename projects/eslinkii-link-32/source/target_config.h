/*
********************************************************************************
*
*	模块名称 : 目标芯片编程功能配置
*	文件名称 : target_program_config.h
*	版    本 : V1.0
*	说    明 : 头文件
*
********************************************************************************
*/
#ifndef _TARGET_CONFIG_H
#define _TARGET_CONFIG_H

//目标芯片编程接口
/* ISP */

/* SWD */   
#define  ESLINK_SWD_ENABLE          0                  //SWD功能使能
/* BOOTISP */
#define  ESLINK_BOOTISP_ENABLE      0                  //BOOTISP功能使能
/* RTC */
#define  ESLINK_RTC_ENABLE          0                 //RTC功能使能

//默认烧录接口
#define ESLINK_ONLINE_DEFAULT_INTF      (PRG_INTF_ISP)  //联机默认烧录接口
//#define ESLINK_ONLINE_DEFAULT_INTF      (PRG_INTF_SWD)  //联机默认烧录接口
//#define ESLINK_ONLINE_DEFAULT_INTF      (PRG_INTF_BOOTISP)  //联机默认烧录接口


#define M610    0
#define M620    0
#define M621    0
#define M939    0
#define M938    1


#if M610
#include "./es-isp/M610_ISP/ES_ISP.h"
#endif

#if M620
#include "./es-isp/M620_ISP/ES_ISP.h"
#endif

#if  M621
#include "./es-isp/M621_ISP/ES_ISP.h"
#endif
 
#if M939
#include "./es-isp/M939_ISP/ES_ISP.h"
#include "./es-isp/M939_ISP/target_info.h"
//#include "./es-rtc/M939_RTC"
//#include "./es-uartboot" 
#endif   

#if  M938
#include "./es-isp/M938_ISP/ES_ISP.h"
#endif
 

#endif

