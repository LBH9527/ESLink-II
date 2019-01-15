#ifndef _ES_ISP_H_
#define _ES_ISP_H_

#include "target_program_config.h"
//根据芯片手动调整的数据信息

#define CHIP_CHECKSUM_ADDR      0x7C0            //根据IDE Space 修改

#if ESLINK_RTC_ENABLE    
    #define M939_CONFIG_WORD_SIZE       54      //54个字长
#else
    #define M939_CONFIG_WORD_SIZE       38      //38个字长
#endif

#define M939_RTC_INFO_ADDR          0x1000
#define M939_RTC_INFO_OFFSET        38      //rtcinfo在配置字中的偏移量
#define M939_RTC_INFO_SIZE          16      //16个字长
#define RTC_DEBUG   0                       //是否在配置字编程时 编程RTC_INFO,内部使用

//RTC默认配置字
#define M939_RTC_CONFIG_DEFAULT_H       0xFFFFFFFF 
#define M939_RTC_CONFIG_DEFAULT_L       0xF0FE0F01
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
