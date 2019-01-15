#ifndef _ISP_H_
#define _ISP_H_  

#define CHIP_CHECKSUM_ADDR      0x23            //根据IDE Space 修改   
#define CHIP_INFO1_ADDR         0x00100100    
#define CHIP_INFO1_SIZE         256   
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
  
uint8_t isp_mode_check(void);   
uint8_t isp_erase_chip(void); 
uint8_t isp_erase_info1(void); 
uint8_t isp_read_config(uint32_t addr, uint32_t *data, uint32_t size); 
uint8_t isp_program_config(uint32_t addr, uint32_t *data, uint32_t size,uint32_t *failed_offset); 
uint8_t isp_program_code(uint32_t addr, uint32_t *data, uint32_t size,uint32_t *failed_offset)  ; 
uint8_t isp_read_code(uint32_t addr, uint32_t *data, uint32_t size); 

#endif
