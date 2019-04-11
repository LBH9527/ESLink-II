/*
********************************************************************************
*
*  模块名称 : 目标芯片编程功能配置
*  文件名称 : target_program_config.h
*  版    本 : V1.0
*  说    明 : 头文件
*   2019年3月14日  9527    M938    脱机检测次数为3
*
********************************************************************************
*/
#ifndef _TARGET_CONFIG_H
#define _TARGET_CONFIG_H

/****************************联机编程设置**************************************/
/*ISP */
//默认使能
/* SWD */   
#define  ESLINK_SWD_ENABLE          0                  //SWD功能使能
/* BOOTISP */
#define  ESLINK_BOOTISP_ENABLE      0                  //BOOTISP功能使能
/* RTC */
#define  ESLINK_RTC_ENABLE          1                //RTC功能使能

//默认烧录接口
#define ESLINK_ONLINE_DEFAULT_INTF      (PRG_INTF_ISP)  //联机默认烧录接口
//#define ESLINK_ONLINE_DEFAULT_INTF      (PRG_INTF_SWD)  //联机默认烧录接口
//#define ESLINK_ONLINE_DEFAULT_INTF      (PRG_INTF_BOOTISP)  //联机默认烧录接口
/****************************脱机编程配置******************************/
//脱机检测次数
#define PROG_MODE_CHECK_TIME        5                   //脱机检测次数

//#define M610
//#define M620
//#define M621
#define M939    
//#define M938    


#ifdef M610
    #include "./es-isp/M610_ISP/ES_ISP.h"
#elif defined M620
    #include "./es-isp/M620_ISP/ES_ISP.h"
#elif defined M621
    #include "./es-isp/M621_ISP/ES_ISP.h"
#elif defined M939
    #include "./es-isp/M939_ISP/ES_ISP.h"
    #include "./es-isp/M939_ISP/target_info.h"
#elif defined M938
    #include "./es-isp/M938_ISP/ES_ISP.h"
#endif



#endif

