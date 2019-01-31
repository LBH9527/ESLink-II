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
#ifndef _TARGET_PROGRAM_CONFIG_H
#define _TARGET_PROGRAM_CONFIG_H


/* ISP */

/* SWD */   
#define  ESLINK_SWD_ENABLE          0                  //SWD功能使能
/* BOOTISP */
#define  ESLINK_BOOTISP_ENABLE      1                  //BOOTISP功能使能
/* RTC */
#define  ESLINK_RTC_ENABLE          0                  //RTC功能使能

//#define ESLINK_ONLINE_DEFAULT_INTF      (PRG_INTF_ISP)  //联机默认烧录接口
//#define ESLINK_ONLINE_DEFAULT_INTF      (PRG_INTF_SWD)  //联机默认烧录接口
#define ESLINK_ONLINE_DEFAULT_INTF      (PRG_INTF_BOOTISP)  //联机默认烧录接口
#endif

