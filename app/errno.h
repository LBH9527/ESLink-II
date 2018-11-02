
#ifndef _ERRNO_H
#define _ERRNO_H

#ifdef __cplusplus
}
#endif

typedef enum {
    /* Shared errors */
    ERROR_SUCCESS = 0,
    //--------------------------------------------------------------------------   
    ERROR_FRAME_HEAD        = 0x01,           //帧头错误
    ERROR_DEVICE_TYPE       = 0x02,          //设备型号错误
    ERROR_FUN_CODE          = 0x03,             //功能码不支持
    ERROR_CMD_PARITY        = 0x04,           //命令校验错误
    ERROR_BUSY              = 0x05 ,                 //设备忙      
    //EEPROM
    ERROR_EE_WRITE          = 0x06 ,                //"eeprom write failed.",
    ERROR_EE_READ           = 0x07  ,                //"eeprom read failed.",

    ERROR_SPI_FLASH_ERASE   = 0x08,
    ERROR_SPI_FLASH_WRITE   = 0x09,          //"spi flash write failed.",
    ERROR_SPI_FLASH_READ    = 0x0A,           //"spi flash read failed.",
     /* FileSystem interface */
    ERROR_FS_INIT           = 0x0B,              //"file system initialize failed.",
    ERROR_FS_MOUNT          = 0x0C,
    ERROR_FS_OPEN           = 0x0D,              //"创建失败file system open failed.",
    ERROR_FS_FIND           = 0x0E,              //查找文件失败
    ERROR_FS_READ           = 0x0F,              //"file system read failed.",
    ERROR_FS_WRITE          = 0x10,             //"file system write failed.",     
    //内部flash
    ERROR_IAP_READ          = 0x11,                //"In application programming read failed.",
    ERROR_IAP_WRITE         = 0x12,                //"In application programming write failed.", 
    //
    ERROR_OUT_OF_BOUNDS = 0x13,                 //读写长度越界， 
    ERROR_CHIP_ID_NOT_MATCH = 0x20,             //ID不匹配
    ERROR_PROM_VOLTAGE      = 0x21,             //电压设置错误
    
    //--------------------------------------------------------------------------  
    //联机错误     
    ERROR_TIMING_CHECKSUM   = 0x30,             //时序校验和错误    
    ERROR_USER_HEX_CHECKSUM = 0x31,             //用户代码校验和错误
    ERROR_CFG_WORD_CHECKSUM = 0x32,             //配置字校验和错误
        //BOOT
    ERROR_BOOT_UPDATE_TIMING = 0X33,           //boot更新时序失败
    
    ERROR_IN_ISP_MODE       = 0x40,             //进模式失败 
    //"ISP FAILED to unlock the target",
    ERROR_ISP_UNLOCK        = 0x41,             //芯片解锁失败
    //"ISP erase command FAILURE", 
    ERROR_ISP_ERASE         = 0x42,             //ISP擦除失败
    //"ISP check empty command FAILURE", 
    ERROR_ISP_FLASH_CHECK_EMPTY   = 0x43,       //ISP FLASH查空失败 
    ERROR_ISP_CFG_WORD_CHECK_EMPTY   = 0x44,    //ISP 配置字查空失败 
    //"ISP program command FAILURE",     
    ERROR_ISP_PROG          = 0x45,              //ISP code编程失败
    //"ISP read command FAILURE", 
    ERROR_ISP_READ          = 0x46,             //ISP读code失败
    ERROR_ISP_PROG_CFG_WORD = 0x47,             //ISP配置字编程失败
    ERROR_ISP_READ_CFG_WORD = 0x48,             //ISP配置字读失败
    //"ISP verify command FAILURE",  
    ERROR_ISP_VERIFY        = 0x49,               //ISP code校验失败 
    ERROR_ISP_CFG_WORD_VERIFY = 0x4A,             //isp 配置字校验失败
    //"ISP encrypt command FAILURE",  
    ERROR_ISP_ENCRYPT       = 0x4B,              //ISP加密失败
    
    //swd
    ERROR_SWD_ERASE         = 0x50,                //SWD擦除失败    
    ERROR_SWD_CHECK_EMPTY   = 0x51,                 //SWD查空失败
    ERROR_SWD_PROG          = 0x52,                 //swd编程失败
    ERROR_SWD_PROG_CFG_WORD = 0x53,             //swd配置字编程失败
    ERROR_SWD_VERIFY        = 0x54,               //swd校验失败    
    
    /* swd target flash errors */ 
    ERROR_RESET             = 0x67,            //"SWD FAILED to reset/halt the target MCU",  
    ERROR_ALGO_DL           = 0x68,             //"SWD FAILED to download the flash programming algorithms to the target MCU",
    ERROR_ALGO_DATA_SEQ     = 0x69,             //"SWD FAILED to download the flash data contents to be programmed",
    ERROR_INIT              = 0x6A,             //"SWD FAILED to initialize the target MCU",
    ERROR_SECURITY_BITS     = 0x6B,             //"SWD ABORTED programming. Image is trying to set security bits",
    ERROR_UNLOCK            = 0x6C,           //"SWD FAILED to unlock the target for programming",
    ERROR_ERASE_SECTOR      = 0x6D,             //"Flash algorithm erase sector command FAILURE",
    ERROR_ERASE_ALL         = 0x6E,             //"Flash algorithm erase all command FAILURE",
    ERROR_WRITE             = 0x6F,            //"Flash algorithm write command FAILURE", 
    
    //bootisp
    ERROR_BOOTISP_START     = 0x70,            //启动失败
    ERROR_BOOTISP_WRITE     = 0x71,            //bootisp写失败
    ERROR_BOOTISP_ERASE     = 0x72,            //BOOTISP擦除失败
    ERROR_BOOTISP_CHECK_EMPTY = 0x73,           //bootisp查空失败     
    ERROR_BOOTISP_PROG      = 0x74,             //bootisp编程失败
    ERROR_BOOTISP_PROG_CFG_WORD = 0x75,         //bootisp编程失败
    
    //脱机
    ERROR_PROG_INTF = 0x80,                     //烧录接口设置错误  
    ERROR_OFL_TIMING_CHECKSUM = 0x81,           //时序校验和错误    
    ERROR_OFL_USER_HEX_CHECKSUM = 0x82,        //用户代码校验和错误
    ERROR_OFL_CFG_WORD_CHECKSUM = 0x83,        //配置字校验和错误
    ERROR_OFL_EXIT = 0X84,                      //脱机工程已存在  the file is existing.
    ERROR_OFL_DELETE = 0x85,                    //工程删除失败
    //------------------------------
    ERROR_OFL_NUM_OVERSTEP = 0x85,          //脱机文件超过支持的最大文件数
    ERROR_OFL_DATA_FORMAT = 0x86,           //脱机文件数据格式错误，此脱机方案数据出错
    ERROR_OFL_DECODE = 0x87,                //脱机文件解析失败
    //RTC
    //DEBUG
    HEAD_ERROR	=	0xA0 ,
 DEV_ADD_ERROR = 					0xA1,
 FUN_ERROR = 					0xA2   ,
 CHECKSUM_ERROR	=				0xA3    ,
 BUSY_ERROR = 					0xA4     ,
 CHIP_INFO_ERROR =					0xA5 ,
 SCHEDULE_CHECKSUM_ERROR  =        0xA6 ,
 SEND_DATE_ADDR_ERROR  =          0xA7,
 SEND_DATE_LENTH_ERROR  =         0xA8,
 UNLOCK_ERROR   =                 0xA9 ,
 BLANK_ERROR   =                  0xAA ,
 PGM_ERROR    =                   0xAB  ,
 OPTION_PGM_ERROR  =              0xAC   ,
 OPTION_VERIFY_ERROR  =           0xAD    ,            
 ENCYPT_ERROR    =                0xAE      ,
 USER_CODE_CHECKSUM_ERROR   =     0xB0       ,
 WRITE_EEPROM_ERROR     =         0xB1        ,
 CHIP_CONTROL_ERROR 	=		    0xB2      ,
 ERASE_ERROR     	=		    0xB3           ,

 OPTION_CODE_CHECKSUM_ERROR   =   0xAF,
 MEMORY_CODE_CHECKSUM_ERROR   =   0xB0,
   
    ERROR_ESLINK_RUN_OK             = 0xF4,

//    ERROR_COUNT,
} error_t;

//const char *error_get_string(error_t error);

//typedef unsigned char error_type_t;

//#define ERROR_TYPE_INTERNAL 0x1
//#define ERROR_TYPE_TRANSIENT 0x2
//#define ERROR_TYPE_USER 0x4
//#define ERROR_TYPE_TARGET 0x8
//#define ERROR_TYPE_INTERFACE 0x10
//// If you add another error type:
//// 1. update error_type_names, used by read_file_fail_txt()
//// 2. update ERROR_TYPE_MASK
//// 3. make sure that error type bits still fit inside of error_type_t
//#define ERROR_TYPE_MASK 0x1F

//error_type_t error_get_type(error_t error);

#ifdef __cplusplus
}
#endif

#endif
