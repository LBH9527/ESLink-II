#ifndef __TARGET_INFO_H
#define __TARGET_INFO_H
#include "target_config.h"

struct info_part_map
{
    uint32_t addr;
    uint32_t size;
};  

//1.芯片信息根据芯片手动调整的数据信息               //根据IDE Space 修改
#define CHIP_CHIPID                 0xC40
#define CHIP_CHECKSUM_ADDR          0x7C0
#define CHIP_CHECKSUMN_ADDR         0x7C8

#define CHIP_CFG_GBRDP_ADDR         0x800           //加密字地址
#define CHIP_CFG_GBRDP_OFFSET       120             //加密字在配置字中的偏移
//2.CHIP INFO的信息需要根据芯片的xml文件来确认 

#define CHIP_INFO_PART1_ADDR        0x400
#define CHIP_INFO_PART1_SIZE        56    //14---->56         

#define CHIP_INFO_PART2_ADDR        0x7C0
#define CHIP_INFO_PART2_SIZE        96    //24--->96        

//RTC INFO 地址
#define CHIP_RTC_INFO_ADDR          0x1000    
#define CHIP_RTC_INFO_SIZE          64    //16---->64               //16个字长
#define CHIP_RTC_INFO_OFFSET        152   //38---->152              //rtcinfo 在配置字中的偏移量


static const struct info_part_map info_part_map[] =
{
    {CHIP_INFO_PART1_ADDR, CHIP_INFO_PART1_SIZE},
    {CHIP_INFO_PART2_ADDR, CHIP_INFO_PART2_SIZE},
};  
 

//3.rtc编程相关数据
#define RTC_DEBUG   0                   //是否在配置字编程时 编程RTC_INFO,内部使用
//RTC默认配置字
#define M939_RTC_CONFIG_DEFAULT_H       0xFFFFFFFF 
#define M939_RTC_CONFIG_DEFAULT_L       0xF0FE0F01

//4.SWD\UARTBoot编程信息，根据IDE SPEC确认  
#define CHIP_INFO_FLASH_OFFSET          0x40000           //INFO区在flash中的绝对地址

#define CHIP_INFO1_OFFSET                (0x400)
#define CHIP_INFO1_ADDR                  (CHIP_INFO_FLASH_OFFSET + CHIP_INFO1_OFFSET)
#define CHIP_INFO1_SIZE                  (0x400)

#endif
