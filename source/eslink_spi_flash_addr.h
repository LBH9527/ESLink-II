//spi flash 地址配置
#ifndef __ES_FLASH_CONFIG_H__
#define __ES_FLASH_CONFIG_H__

//spi flash分为两部分，一部分用做文件系统，主要是保存脱机工程数据。一部分需要人为划分
//主要用做联机编程时的数据保存。
/**
 *
 * This all Backup Area Flash storage index. All used flash area configure is under here.
 * |----------------------------|   Storage Size
 * | Environment variables area |   ENV area size @see ENV_AREA_SIZE
 * |      1.system section      |   ENV_SYSTEM_SIZE
 * |      2:data section        |   ENV_AREA_SIZE - ENV_SYSTEM_SIZE
 * |----------------------------|
 * |      Saved log area        |   Log area size @see LOG_AREA_SIZE
 * |----------------------------|
 * |(IAP)Downloaded application |   IAP already downloaded application, unfixed size
 * |----------------------------|
 *
 **/

//******************************************************************************
//ES_Link信息配置
/*---------------------------------芯片内部flash--------------------------------*/
#define FirmwareVersionsArea                0x8007ff0  //256 byte 固件版本号
#define DefaultPowerStatusArea              0x800bd00  //256 byte 默认电源状态

#define TimingInformationArea               0x800ff00  //512b  芯片信息 
#define TimingArea                          0x8010000	//app地址

#define FIRMWARE_VERSION_ADDR               0x8007ff0  //固件版本号    
#define FIRMWARE_VERSION_LEN                4           //固件信息长度.(byte)
#define CHIP_TIMING_INFO_ADDR               0x800ff00  //512b  芯片信息 
 

/*---------------------------------外部spi—flash-------------------------------*/
//SPI FLASH信息    
//MX25L64  总容量 = 64M bit = 8M bytes      地址范围： 0 --- 7F FFFF            
//写入数据时，按页大小写入，且首地址为扇区（sector）起始地址。
//用户hex数据和长度.hex 长度由目标芯片容量确定 

//脱机/联机编程操作相关数据地址
#define SF_USER_HEX_ADDR            0x000000        //用户HEX地址 

#define SF_USER_HEX_CHECKOUT_ADDR   0x1FD000        //用户HEX校验和地址

#define SF_TIMING_INFO_ADDR         0x3FF000        //时序信息地址
#define SF_TIMING_INFO_SIZE         0x3FF000        //时序信息地址

#define SF_TIMING_ADDR              0x3FF000        //时序地址
#define SF_TIMING_SIZE              0x3FF000        //时序长度
#define SF_TIMING_CHECKOUT_ADDR     0x1FD000        //用户HEX校验和地址

#define SF_CONFIG_INFO_ADDR         0x3FF000        //配置字地址  
#define SF_CONFIG_INFO_SIZE         0x1000          //配置字长度

#define SF_CHIP_INFO_ADDR           0x1FE000        //chip information addr in spi flash   

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


//for offline test  
#define OFFLINE_INFO_ADDR               0x200000


#endif
