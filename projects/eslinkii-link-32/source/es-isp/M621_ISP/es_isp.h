#ifndef _ES_ISP_H_
#define _ES_ISP_H_  
/*******************************************************************************
              函数声明
*******************************************************************************/  
uint8_t isp_unlock_check(void);  
uint8_t isp_mode_check(void);
uint8_t isp_erase_code(uint32_t addr, uint32_t size); 
uint8_t isp_erase_info7(void);
uint8_t isp_read_config(uint32_t addr, uint8_t *data, uint32_t size);
uint8_t isp_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_offset);
uint8_t isp_program_code(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_offset) ;
uint8_t isp_read_code(uint32_t addr, uint8_t *data, uint32_t size) ;


#endif
