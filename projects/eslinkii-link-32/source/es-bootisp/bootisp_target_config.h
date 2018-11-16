#ifndef _BOOTISP_TARGET_CONFIG_H_
#define _BOOTISP_TARGET_CONFIG_H_


#include "es_common.h"
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
//写入的最大数据块长度
#define BOOTSIP_DATA_MAX_LEN        0x80 //packet_size


/******************************************************************************/
//bootisp数据定义   
//校验类型
#define BOOTISP_CHECKSUM_CRC32          0
#define BOOTISP_CHECKSUM_CRC32_ECC      1
#define BOOTISP_CHECKSUM_CUSUM16        2
#define BOOTISP_CHECKSUM_CUSUM32        3



//info属性
#define INFO_ATTR_NONE          0   //无属性
#define INFO_ATTR_RDP           1   //info区包含全加密字
#define INFO_ATTR_CPROP         2   //info区包含私有代码保护配置字  
//芯片信息区
typedef struct {
    const uint32_t start;
    const uint32_t size;
    const uint16_t index;
    const uint16_t attr; 
}chip_info_t;



//info区最大数量，暂定为4
#define CHIP_INFO_LEN_MAX       4    
//目标芯片信息
typedef struct  {
    uint32_t flash_start;         
    uint32_t flash_size;
    uint8_t reset_suppot;
    uint8_t ack ;
    uint8_t start ;
    uint8_t checksum;
    uint8_t packet_size;

    const chip_info_t* chip_info; 
    int chip_info_length;
    

//    uint32_t timing_id;             /*时序号*/   
//    uint32_t timing_version;        /*时序版本*/
//    uint32_t timing_checksum;       //时序校验和
//    
//    uint32_t code_start;            //主程序区起始地址
//    uint32_t code_size;             //主程序区容量
//    uint32_t config_start;          //配置字信息区起始地址
//    uint32_t config_size;           //配置字信息区容量
//    uint32_t encrypt_addr;           //加密字地址
//    uint32_t encrypt_value;          //加密字值
//    uint32_t chipid_addr;           //芯片ID地址
//    uint32_t chipid_value;          //芯片ID值
    
} bootisp_target_cfg_t;

extern bootisp_target_cfg_t   bootisp_target_dev  ;
#endif

