#ifndef _ES_ISP_H_
#define _ES_ISP_H_  

#define CHIP_INFO_ADDR          0x00001800  
#define CHIP_LV1_ADDR           0x00001C00

#define CHIP_CHECKSUM_ADDR      0x00001C40
#define CHIP_USERID_ADDR        0x00001C44

//配置字地址信息，保留未用的数据未下发。此信息需要根据XML文件修改。

#define CHIP_INFO_PART1_ADDR         CHIP_INFO_ADDR
#define CHIP_INFO_PART1_SIZE         48              //字节

#define CHIP_INFO_PART2_ADDR         CHIP_LV1_ADDR
#define CHIP_INFO_PART2_SIZE         8             //字节

#define CHIP_INFO_PART3_ADDR         (CHIP_LV1_ADDR+8)
#define CHIP_INFO_PART3_SIZE         8              //字节

#define CHIP_INFO_PART4_ADDR         (CHIP_CHECKSUM_ADDR)
#define CHIP_INFO_PART4_SIZE         8              //字节

#define CHIP_INFO_PART_SIZE          4
#define CHIP_INFO_SIZE              ( CHIP_INFO_PART1_SIZE + CHIP_INFO_PART2_SIZE   \
                                        + CHIP_INFO_PART3_SIZE + CHIP_INFO_PART4_SIZE)
/*******************************************************************************
							函数声明
*******************************************************************************/  
uint8_t isp_unlock_check(void);  
uint8_t isp_mode_check(void);
uint8_t isp_erase_code(uint32_t addr, uint32_t size); 
uint8_t isp_erase_info6(void);
uint8_t isp_read_config(uint32_t addr, uint8_t *data, uint32_t size);
uint8_t isp_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_offset);
uint8_t isp_program_code(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_offset) ;
uint8_t isp_read_code(uint32_t addr, uint8_t *data, uint32_t size) ;


#endif
