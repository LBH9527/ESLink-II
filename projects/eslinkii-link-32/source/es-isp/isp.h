#ifndef _ISP_H_
#define _ISP_H_

//#define M939
#define M620 
//#define M610 

#ifdef M620  
#define CHIP_CHECKSUM_ADDR      0x2C            //根据IDE Space 修改
#endif

#ifdef M610
#define CHIP_CHECKSUM_ADDR      0x23            //根据IDE Space 修改
#endif

#ifdef M939
#define CHIP_CHECKSUM_ADDR      0x248            //根据IDE Space 修改
#endif



/*******************************************************************************
							函数声明
*******************************************************************************/
uint8_t isp_entry_isp_mode(void) ;
uint8_t isp_out_isp_mode(void);   
uint8_t isp_erase_chip(void);  
uint8_t isp_read_config(uint32_t addr, uint32_t *data, uint32_t size); 
uint8_t isp_program_config(uint32_t addr, uint32_t *data, uint32_t size,uint32_t *failed_addr); 
uint8_t isp_program_code(uint32_t addr, uint32_t *data, uint32_t size,uint32_t *failed_addr)  ; 
uint8_t isp_read_code(uint32_t addr, uint32_t *data, uint32_t size); 
  
#endif
