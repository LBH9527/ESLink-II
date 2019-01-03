#ifndef __SETTINGS_SPI_FLASH__
#define __SETTINGS_SPI_FLASH__
#include "errno.h"
//联机数据类型  
#define USER_HEX                0x00            //用户HEX   
#define CFG_WORD                0x01            //用户配置字
#define CFG_WORD_CHECKSUM       0x02            //配置字校验和
#define HEX_CHECKSUM            0x03            //用户HEX校验和
#define OFL_PROG_INFO           0x04            //脱机编程信息
#define RTC_HEX                 0x05            //RTC hex
#define RTC_HEX_CHECKSUM        0x06            //RTC HEX校验和


/*---------------------------------外部spi—flash-------------------------------*/
//SPI FLASH信息    
//MX25L64  总容量 = 64M bit = 8M bytes      地址范围： 0 --- 7F FFFF            
//0----- 1F FFFF = 2M 保存当前联机方案信息

//脱机/联机编程操作相关数据地址
//0x00 0000 ---- 0F FFFF     1024K     保存用户HEX
#define SF_USER_HEX_ADDR                0x000000    

//0x10 0000 ---- 1E FFFF                保存RTC hex
#define SF_RTC_HEX_ADDR                 0x100000        
#define SF_RTC_HEX_CHECKSUM_ADDR        0x1EF000 

//0x1F 0000 ---- 1F CFFF     52K        保存配置字
#define SF_CONFIG_WORD_ADDR             0x1F0000   

//0x1F D000 ---- 1F DFFF     4K         用户HEX校验和
#define SF_USER_HEX_CHECKSUM_ADDR       0x1FD000          

//0x1F E000 ---- 1F EFFF     4K         保存配置字校验和
#define SF_CONFIG_WORD_CHECKSUM_ADDR       0x1FE000   
     
//0x1F F000 ---- 1F FFFF     4K         脱机信息
#define SF_OFFLINE_INFO_ADDR                 0x1FF000 

//1F FFFF ----- 7F FFFF = 6M 采用文件系统 保存脱机方案数据      

// fatfs 文件系统占用的扇区数
#define SF_FATFS_SECTOR_COUNT           1536        //1536*4096/1024/1024=6(MB)
//fatfs 文件系统在spi flash中的偏移地址
#define SF_FATFS_OFFSET                 512         //512*4096 = 2M


error_t online_file_erase(uint8_t type, uint32_t size );
error_t online_file_write(uint8_t type, uint32_t addr, const uint8_t *buf, uint32_t size );
error_t online_file_read(uint8_t type, uint32_t addr, uint8_t *buf, uint32_t size );


#endif
