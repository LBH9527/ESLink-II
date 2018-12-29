#ifndef _ES_ISP_H_
#define _ES_ISP_H_

#include "target_program_config.h"
//根据芯片手动调整的数据信息
#define M939

#ifdef M939
#define CHIP_CHECKSUM_ADDR      0x248            //根据IDE Space 修改
#endif

#if ESLINK_RTC_ENABLE    
    #define M939_CONFIG_WORD_SIZE       54      //54个字长
#else
    #define M939_CONFIG_WORD_SIZE       38      //38个字长
#endif

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
