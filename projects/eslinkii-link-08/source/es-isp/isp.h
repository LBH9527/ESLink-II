#ifndef _ISP_H_
#define _ISP_H_


/*******************************************************************************
							函数声明
*******************************************************************************/
uint8_t isp_entry_isp_mode(void) ;
uint8_t isp_out_isp_mode(void);
uint8_t isp_id_check(void);   //读取ID
uint8_t isp_mode_check(void);      //isp模式检测  
uint8_t isp_erase_chip(void);  
uint8_t isp_program_code(uint16_t addr, uint16_t *data, uint16_t size, uint16_t *failed_addr);
uint8_t isp_program_config(uint16_t addr, uint16_t *data, uint32_t size,uint16_t *failed_addr);
uint8_t isp_read_config(uint16_t addr, uint16_t *data, uint32_t size);
uint8_t isp_read_code(uint16_t addr, uint16_t *data, uint32_t size) ;
  
#endif
