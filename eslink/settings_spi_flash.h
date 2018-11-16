#ifndef __SETTINGS_SPI_FLASH__
#define __SETTINGS_SPI_FLASH__
#include "errno.h"
//联机数据类型  
typedef enum 
{
    USER_HEX,                       //用户HEX   
    CFG_WORD,                       //用户配置字
    CFG_WORD_CHECKSUM  ,            //配置字校验和
    HEX_CHECKSUM  ,                 //用户HEX校验和
    OFL_PROG_INFO ,                  //脱机编程信息
} oline_data_type_t;   
/*---------------------------------外部spi—flash-------------------------------*/
//SPI FLASH信息    
//MX25L64  总容量 = 64M bit = 8M bytes      地址范围： 0 --- 7F FFFF            
//0----- 1F FFFF = 2M 保存当前联机方案信息

//脱机/联机编程操作相关数据地址
//0x00 0000 ---- 1E FFFF     1984K=(2M - 64K)     保存用户HEX
#define SF_USER_HEX_ADDR                0x000000    
   
//0x1F 0000 ---- 1F CFFF     52K        保存配置字
#define SF_CONFIG_WORD_ADDR             0x1F0000   

//0x1F D000 ---- 1F DFFF     4K         用户HEX校验和
#define SF_USER_HEX_CHECKSUM_ADDR       0x1FD000          

//0x1F E000 ---- 1F EFFF     4K         保存配置字校验和
#define SF_CONFIG_WORD_CHECKSUM_ADDR       0x1FE000   
     
//0x1F F000 ---- 1F FFFF     4K         脱机信息
#define SF_OFFLINE_INFO                 0x1FF000 

//1F FFFF ----- 7F FFFF = 6M 采用文件系统 保存脱机方案数据      

// fatfs 文件系统占用的扇区数
#define SF_FATFS_SECTOR_COUNT           1536        //1536*4096/1024/1024=6(MB)
//fatfs 文件系统在spi flash中的偏移地址
#define SF_FATFS_OFFSET                 512         //512*4096 = 2M














//Debug 操作相关数据地址
///////////////////flash中的常量定义/////////////////////////////
#define SHIXU_CODE_Fth                  0x2800                      //固件程序(Application program)  0x2800~0xA000 30K

#define VOLTAGE_SET_INFO				0xF400						 

#define SCHEDULE_CHECKSUM_Fth           0xF602                       //时序代码校验和
#define RESET_LEVEL_Fth  		        0xF607                       //复位电平00：低电平复位；01：高电平复位
#define BURNER_SCHEDULE_No              0xF610
#define MEM_BYTE_CNT_Fth                0xF612

#define CHIP_INFO_Fth                   0x3FE000                       //0.5k//芯片信息

#define ICD_CHIP_INFO_LEN               512
//#define SF_ICD_CHIP_INFO_START                   0x3FE000                       //0.5k//芯片信息
//#define SF_ICD_CHIP_INFO_SIZE                    512                              //0.5k//芯片信息


error_t online_file_erase(oline_data_type_t type, uint32_t size );
error_t online_file_write(oline_data_type_t type, uint32_t addr, const uint8_t *buf, uint32_t size );
error_t online_file_read(oline_data_type_t type, uint32_t addr, uint8_t *buf, uint32_t size );


#endif
