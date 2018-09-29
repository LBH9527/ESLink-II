
#ifndef _ERRNO_H
#define _ERRNO_H

#ifdef __cplusplus
}
#endif

typedef enum {
    /* Shared errors */
    ERROR_SUCCESS = 0,
    //--------------------------------------------------------------------------    
    //EEPROM
    ERROR_EE_INIT   = 1,                //"eeprom initialize failed.",
    ERROR_EE_WRITE  = 2,                //"eeprom write failed.",
    ERROR_EE_READ   = 3,                //"eeprom read failed.",
    ERROR_IAP_READ  = 4,                //"In application programming read failed.",
    ERROR_IAP_WRITE = 5,                //"In application programming write failed.",  
    ERROR_SPI_FLASH_ERASE = 6,
    ERROR_SPI_FLASH_WRITE = 7,          //"spi flash write failed.",
    ERROR_SPI_FLASH_READ = 8,           //"spi flash read failed.",
     /* FileSystem interface */
    ERROR_FS_INIT       =  9,              //"file system initialize failed.",
    ERROR_FS_MOUNT      =  10,
    ERROR_FS_OPEN       =  11,              //"创建失败file system open failed.",
    ERROR_FS_FIND       = 12,              //查找文件失败
    ERROR_FS_READ       = 13,              //"file system read failed.",
    ERROR_FS_WRITE      = 14,             //"file system write failed.",     

    ERROR_OUT_OF_BOUNDS,        //读写长度越界
    //电源 
    //command  id
    ERROR_FRAME_HEAD,           //帧头错误
    ERROR_DEVICE_TYPE,          //设备型号错误
    ERROR_FUN_CODE,             //功能码不支持
    ERROR_CMD_PARITY,           //命令校验错误
    ERROR_BUSY,                 //设备忙
    
    //--------------------------------------------------------------------------     
    
    //联机错误     
    ERROR_TIMING_CHECKSUM = 30,          //时序校验和错误    
    ERROR_USER_HEX_CHECKSUM = 31,        //用户代码校验和错误
    ERROR_CFG_WORD_CHECKSUM = 32,        //配置字校验和错误
    
    
    //"ISP FAILED to unlock the target",
    ERROR_ISP_UNLOCK = 40,               //芯片解锁失败
    //"ISP erase command FAILURE", 
    ERROR_ISP_ERASE = 41,                //ISP擦除失败
    //"ISP check empty command FAILURE", 
    ERROR_ISP_CHECK_EMPTY = 42,          //ISP查空失败 
    //"ISP program command FAILURE",     
    ERROR_ISP_PROG = 43,                 //ISP code编程失败
    //"ISP read command FAILURE", 
    ERROR_ISP_READ = 45,                 //ISP读code失败
    ERROR_ISP_PROG_CFG_WORD = 46,        //ISP配置字编程失败
    ERROR_ISP_READ_CFG_WORD = 47,        //ISP配置字读失败
    //"ISP verify command FAILURE",  
    ERROR_ISP_VERIFY = 48,               //ISP校验失败 
    //"ISP encrypt command FAILURE",  
    ERROR_ISP_ENCRYPT = 49,              //ISP加密失败
    
    //swd
    ERROR_SWD_ERASE = 60,                //SWD擦除失败    
    ERROR_SWD_CHECK_EMPTY = 61,          //SWD查空失败
    ERROR_SWD_PROG = 62,                 //swd编程失败
    ERROR_SWD_PROG_CFG_WORD = 63,        //swd配置字编程失败
    ERROR_SWD_VERIFY = 64,               //swd校验失败    
    
    /* swd target flash errors */ 
    ERROR_RESET,            //"SWD FAILED to reset/halt the target MCU",  
    ERROR_ALGO_DL,          //"SWD FAILED to download the flash programming algorithms to the target MCU",
    ERROR_ALGO_DATA_SEQ,    //"SWD FAILED to download the flash data contents to be programmed",
    ERROR_INIT,             //"SWD FAILED to initialize the target MCU",
    ERROR_SECURITY_BITS,    //"SWD ABORTED programming. Image is trying to set security bits",
    ERROR_UNLOCK,           //"SWD FAILED to unlock the target for programming",
    ERROR_ERASE_SECTOR,     //"Flash algorithm erase sector command FAILURE",
    ERROR_ERASE_ALL,        //"Flash algorithm erase all command FAILURE",
    ERROR_WRITE,            //"Flash algorithm write command FAILURE", 
    
    //bootisp
    ERROR_BOOTISP_START = 80,            //启动失败
    ERROR_BOOTISP_WRITE = 81,            //bootisp写失败
    ERROR_BOOTISP_ERASE = 82,            //BOOTISP擦除失败
    ERROR_BOOTISP_CHECK_EMPTY = 83,      //bootisp查空失败     
    ERROR_BOOTISP_PROG = 84,             //bootisp编程失败
    ERROR_BOOTISP_PROG_CFG_WORD = 85,    //bootisp编程失败
    
    //脱机
    ERROR_PROG_INTF = 100,                //烧录接口设置错误  
    ERROR_OFL_TIMING_CHECKSUM = 101,      //时序校验和错误    
    ERROR_OFL_USER_HEX_CHECKSUM = 102,        //用户代码校验和错误
    ERROR_OFL_CFG_WORD_CHECKSUM = 103,        //配置字校验和错误

    

    
    
    ERROR_ESLINK_RUN_OK             = 0xF4,
//    CONFIGVERIFYFAIL           = 0xAD,
//    ENCRYPFAIL                 = 0xAE,
//    ERROR_HEX_CHECKSUM_FAIL            = 0xAF,
//    CONFIGCHECKSUMFAIL         = 0xB0,
//    SWDOPERATIONFAIL           = 0xB2,
//    //  CONFIGRDFAIL                 0xB4
//    SERIALNUMCHECKSUMFAIL      = 0xB5,
//    POWERSETFAIL               = 0xC1,
//    POWERCONFLICT              = 0xC2,

//    TIMINGINFORDLFAIL          = 0xC4,
//          ERROR_IAP_UPDT,
//    ERROR_IAP_WRITE,
//    ERROR_IAP_ERASE_SECTOR,
//    ERROR_IAP_ERASE_ALL,
//    ERROR_IAP_OUT_OF_BOUNDS,
//    ERROR_IAP_UPDT_NOT_SUPPORTED,
//    ERROR_IAP_UPDT_INCOMPLETE,
//    ERROR_IAP_NO_INTERCEPT,
    ERROR_COUNT,
} error_t;

const char *error_get_string(error_t error);

typedef unsigned char error_type_t;

#define ERROR_TYPE_INTERNAL 0x1
#define ERROR_TYPE_TRANSIENT 0x2
#define ERROR_TYPE_USER 0x4
#define ERROR_TYPE_TARGET 0x8
#define ERROR_TYPE_INTERFACE 0x10
// If you add another error type:
// 1. update error_type_names, used by read_file_fail_txt()
// 2. update ERROR_TYPE_MASK
// 3. make sure that error type bits still fit inside of error_type_t
#define ERROR_TYPE_MASK 0x1F

error_type_t error_get_type(error_t error);

#ifdef __cplusplus
}
#endif

#endif
