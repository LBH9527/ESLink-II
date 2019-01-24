#ifndef _BOOTISP_TARGET_CONFIG_H_
#define _BOOTISP_TARGET_CONFIG_H_


#include "es_common.h"
//写入的最大数据块长度
#define BOOTSIP_DATA_SIZE               0x80 //packet_size

#define BOOTISP_DEFAULT_BAUDRATE        115200              //默认波特率
//115200波特率下的时间
#define BOOTISP_ERASE_TIME              10                  //全擦时间10
#define BOOTISP_CHECK_EMPTY_TIME        50                  //查空时间 
#define BOOTISP_READ_PAGE_TIME          2                   //读一页时间(0x80个字)
#define BOOTISP_WRITE_PAGE_TIME         20                  //写一页时间(0x80个字)

/******************************************************************************/
//bootisp配置,根据bootisp中的939.map文件修改。
#define BOOTISP_ACK         0x79        //ACK                     
#define BOOTISP_NACK        0x1F
//bootisp 命令码
#define CMD_START           0xFF  	    //satrt /* 启动命令，用于波特率检测*/
#define CMD_RD_MEMORY	    0x91		/* 读应用程序 */
#define CMD_GO	            0xA1		/* 跳转到内部flash或SRAM内的应用程序代码 */
#define CMD_WR_MEMORY       0xB1		/* 写数据*/
#define CMD_ERASE           0xC4  	    /* 擦除一个到全部falsh */
#define CMD_GET_CRC         0xD0		/* 读取若干字节的CRC32 */
#define CMD_CHECK_EMPTY     0xD1		/* 查空 */



/******************************************************************************/
//bootisp数据定义   
//校验类型
#define BOOTISP_CHECKSUM_CRC32          0
#define BOOTISP_CHECKSUM_CRC32_ECC      1
#define BOOTISP_CHECKSUM_CUSUM16        2
#define BOOTISP_CHECKSUM_CUSUM32        3    

/******************************************************************************/
//info区信息。参考《BootROM应用笔记》
//info区最大数量，暂定为4
#define CHIP_INFO_LEN_MAX       4   
//info区属性
#define INFO_ATTR_NONE          0   //无属性
#define INFO_ATTR_RDP           1   //info区包含全加密字
#define INFO_ATTR_CPROP         2   //info区包含私有代码保护配置字  

//芯片info区信息
typedef struct 
{
    const uint32_t start;       //info起始地址   
    const uint32_t size;        //info区大小
    const uint32_t index;       //info区索引值
    const uint32_t attr;        //info区属性
}chip_info_t;  
 
//目标芯片信息
typedef struct  {
    uint32_t flash_start;       //flash起始地址     
    uint32_t flash_size;        //flash大小
    uint8_t reset_suppot;       //是否支持
//    uint8_t ack ;               //应答
    uint8_t start ;
    uint8_t checksum;
    uint8_t packet_size;

    const chip_info_t* chip_info; 
    int chip_info_length;
    
} bootisp_target_cfg_t;

extern bootisp_target_cfg_t   bootisp_target_dev  ;
#endif

