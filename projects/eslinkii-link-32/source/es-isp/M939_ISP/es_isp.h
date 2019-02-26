#ifndef _ES_ISP_H_
#define _ES_ISP_H_

#include "target_program_config.h"

//1.芯片信息根据芯片手动调整的数据信息     //根据IDE Space 修改
#define CHIP_CHIPID                 0xC40            
#define CHIP_CHECKSUM_ADDR          0x7C0   

//2.rtc info的信息需要根据芯片的xml文件来确认 
#define CHIP_INFO_PART1_ADDR        0x400
#define CHIP_INFO_PART1_SIZE        14              //字

#define CHIP_INFO_PART2_ADDR        0x7C0
#define CHIP_INFO_PART2_SIZE        24              //字

//RTC INFO 地址
#define CHIP_RTC_INFO_ADDR          0x1000    
#define CHIP_RTC_INFO_SIZE          16              //16个字长
#define CHIP_RTC_INFO_OFFSET        38              //rtcinfo 在配置字中的偏移量

#if ESLINK_RTC_ENABLE    
    #define CHIP_CONFIG_WORD_SIZE      \
                (CHIP_INFO_PART1_SIZE + CHIP_INFO_PART2_SIZE + CHIP_RTC_INFO_SIZE)      //54个字长
#else
    #define CHIP_CONFIG_WORD_SIZE      \
                                    (CHIP_INFO_PART1_SIZE + CHIP_INFO_PART2_SIZE)      //38个字长
#endif   

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


/*******************************************************************************
							函数声明
*******************************************************************************/  
//isp复位
void isp_reset(void);
//读芯片ID
uint8_t isp_id_check(void);
//解锁并判断是否解锁成功
uint8_t isp_unlock_check(void);
//isp 模式设置
uint8_t isp_mode_set(void);
//isp 加密字加载并判断
uint8_t isp_encrypt_check(void);   
uint8_t isp_mode_check(void);   
uint8_t isp_erase_chip(void);  
uint8_t isp_read_config(uint32_t addr, uint32_t *data, uint32_t size); 
uint8_t isp_program_config(uint32_t addr, uint32_t *data, uint32_t size,uint32_t *failed_offset); 
uint8_t isp_program_code(uint32_t addr, uint32_t *data, uint32_t size,uint32_t *failed_offset)  ; 
uint8_t isp_read_code(uint32_t addr, uint32_t *data, uint32_t size); 

uint8_t rtc_info_erase(void);

#endif
