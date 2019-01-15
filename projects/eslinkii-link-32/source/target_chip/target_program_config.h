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
#define  ESLINK_SWD_ENABLE      1                   //SWD功能使能
/* BOOTISP */

/* RTC */
#define  ESLINK_RTC_ENABLE      0                   //RTC功能使能


bool config_get_auto_rst(void) ;
bool config_get_automation_allowed(void);
bool config_isp_verify_program(void); 

#endif

