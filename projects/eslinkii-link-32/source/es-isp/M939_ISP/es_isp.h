#ifndef __ES_ISP_H
#define __ES_ISP_H

/*******************************************************************************
							函数声明
*******************************************************************************/  
////isp复位
//void isp_reset(void);
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
