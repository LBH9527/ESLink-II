#ifndef _BOOTISP_TARGET_CONFIG_H_
#define _BOOTISP_TARGET_CONFIG_H_   

#include "es_common.h"

#define BOOTISP_ACK                     0x79        //ACK                     
#define BOOTISP_NACK                    0x1F
//bootisp 命令码
#define CMD_START                       0xFF  	    //satrt /* 启动命令，用于波特率检测*/
#define CMD_RD_MEMORY	                0x91		/* 读应用程序 */
#define CMD_GO	                        0xA1		/* 跳转到内部flash或SRAM内的应用程序代码 */
#define CMD_WR_MEMORY                   0xB1		/* 写数据*/
#define CMD_ERASE                       0xC4  	    /* 擦除一个到全部falsh */
#define CMD_GET_CRC                     0xD0		/* 读取若干字节的CRC32 */
#define CMD_CHECK_EMPTY                 0xD1		/* 查空 */    
#define CHIP_INFO1_INDEX		        0x0101	    //info1 索引值

/******************************************************************************/
//写入的最大数据块长度
#define BOOTSIP_DATA_SIZE               0x80                //packet_size   
#define BOOTISP_DEFAULT_BAUDRATE        115200              //默认波特率 
#define BOOTISP_DEFAULT_TIME            100                  //默认等待时间 
#define BOOTISP_WRITE_PAGE_TIME         200                  //写一页时间  

/******************************************************************************/
////info区信息。参考《BootROM应用笔记》
////info区最大数量，暂定为4
//#define CHIP_INFO_LEN_MAX       4   
////info区属性
//#define INFO_ATTR_NONE          0   //无属性
//#define INFO_ATTR_RDP           1   //info区包含全加密字
//#define INFO_ATTR_CPROP         2   //info区包含私有代码保护配置字  

////芯片info区信息
//typedef struct 
//{
//    const uint32_t start;       //info起始地址   
//    const uint32_t size;        //info区大小
//    const uint32_t index;       //info区索引值
//    const uint32_t attr;        //info区属性
//}chip_info_t;  
 

#endif

