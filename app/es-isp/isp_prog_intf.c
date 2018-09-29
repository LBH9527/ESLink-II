#include "string.h"
#include "stdbool.h"
#include "eslink.h"
#include "errno.h"
#include "isp_def.h"
#include "isp_prog_intf.h"
#include "M620_ISP.c"
/*******************************************************************************
							函数声明
*******************************************************************************/
static void isp_init(es_target_cfg *target);
static error_t isp_prog_erase_chip (uint8_t *para);
static error_t isp_prog_check_empty(uint32_t *failedAddress, uint32_t *failedData) ;
static error_t isp_prog_read_chipid(uint32_t *buf);
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size);

struct  es_prog_ops isp_prog_intf = {
    isp_init,
    
    isp_prog_erase_chip,
    isp_prog_check_empty,
//    isp_prog_read_chipid,
};

#define ISP_PRG_MINI_SIZE  1024 

//isp操作错误地址和错误数据
 typedef struct {
    uint32_t addr;          //错误地址
    uint32_t data;          //错误数据
}isp_process_error_t;

static const es_target_cfg *isp_target_dev;   

void isp_init(es_target_cfg *target)
{
     isp_target_dev = target;
//     isp_flash_intf.target_erase_chip =  M620_erase_chip;
//     isp_flash_intf.target_program_code =  isp_program_code;
//     isp_flash_intf.target_program_info =  isp_program_config;
//     isp_flash_intf.target_read_code =  isp_read_code;
//     isp_flash_intf.target_read_info =  isp_read_config;
}
void isp_set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_RST_OUT(0) : PIN_RST_OUT(1);
}

static error_t isp_prog_init()
{
    isp_set_target_reset(0);
    isp_unlock();
}

/*******************************************************************************
*函数名：
* 描述 ：读目标芯片ID
* 输入 ：data：数据。
* 输出 ：
*******************************************************************************/
static error_t isp_prog_read_chipid(uint32_t *buf)
{    
     uint8_t ret;
    	
    ret = isp_read_code(isp_target_dev->chipid_addr, buf, 1);
    if(ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS; 
}

/*
 * 擦除
 */
static error_t isp_prog_erase_chip (uint8_t * para) 
{
    error_t status = ERROR_SUCCESS;
       
    if(isp_erase_chip() != TRUE)
		 status = ERROR_ISP_ERASE;
    return status;
}
/*
 * 查空
 */
static error_t isp_prog_check_empty(uint32_t *failedAddress, uint32_t *failedData)                             
{
    uint32_t fail_addr;
    uint32_t fail_data;
	uint8_t i;
	uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t read_buf[ISP_PRG_MINI_SIZE/4]; 
	uint32_t copy_size; 
    failedAddress = &fail_addr ;
    failedData =  &fail_data;	
    
	code_addr =  isp_target_dev->code_start;
	code_size =  isp_target_dev->code_size;
	while(true)
	{
		copy_size = MIN(code_size, sizeof(read_buf) );
	    isp_read_code(code_addr, read_buf, copy_size);
		for(i = 0; i<copy_size; i++)
		{
			if(read_buf[i] != 0xFFFFFFFF)
			{              
			     fail_addr = code_addr + i*4  ;
                 fail_data = read_buf[i] ; 
				 return ERROR_ISP_CHECK_EMPTY;
			} 				
		} 
        // Update variables
        code_addr  += copy_size;
        code_size  -= copy_size;
        
        // Check for end
        if (code_size <= 0) {
            break;
        } 
	}      
    return ERROR_SUCCESS;      
} 
/*******************************************************************************
*函数名：
* 描述 ：
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size)
{
    error_t ret = ERROR_SUCCESS;

    uint32_t size_in_words;    
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
    ret = isp_program_code( addr, (uint32_t*)data, size_in_words);
    return ERROR_SUCCESS;     
}
/*
 *配置字编程
 * ERROR_ISP_PROG_CONFIG
 */
error_t isp_prog_programe_config(uint32_t addr, const uint8_t *data, uint32_t size)
{

    error_t result = ERROR_SUCCESS;         
//    M620_config_program();
	        
//    result = config_programe(&(buf[0].B32));
    
    return result;        
}

/*
 *读配置字
 * return :   ERROR_SUCCESS   
 */
error_t isp_prog_read_config(uint32_t addr,  uint32_t *buf, uint32_t size)
{
    uint8_t ret;
        
    ret = isp_read_config(addr, buf, size) ;
    if( ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS;        
}

/*
 * 芯片验证
 *  比较spi flash中的用户hex和目标芯片中的数据，在ESLINk中只比较校验和
 */

//error_t isp_flash_verify_program(uint32_t addr, const uint8_t *data, uint32_t size)
error_t isp_prog_flash_verify(uint32_t sum)
{
    uint8_t status;
    uint32_t i;
    uint32_t size = isp_target_dev->code_size;
    uint32_t addr = isp_target_dev->code_start;

    uint32_t checksum = 0;
    uint32_t read_size = 0;
    uint32_t read_buf[ISP_PRG_MINI_SIZE/4]; 
    
    //读目标芯片中的数据，与spi flash中的数据比较是否一致，不一致返回错误地址和错误值。
    //如果数据全相同，在比较校验和与上位机下发的校验和是否一致。
    
    while(true)
    {     
        read_size = MIN(size, sizeof(read_buf));

        status = isp_read_code( addr, read_buf, read_size ) ;
        if(status != TRUE)
            return  ERROR_ISP_READ;    
             
        for(i=0; i<read_size; i++)
        {
            checksum += (read_buf[i] & 0xFF) ;
            checksum += (read_buf[i] >> 8) & 0xFF;
            checksum += (read_buf[i] >> 16) & 0xFF;
            checksum += (read_buf[i] >> 24) & 0xFF;
        }
        addr += read_size;
        size -= read_size; 
        if( size <= 0)
            break;       
    }   
    //读spi flash中的用户hex数据计算校验和，与保存在spi flash 中 SF_HEX_CHECKSUM_ADDR 
    //确认spi flash中的数据正确。
//    es_ext_flash_read(SF_HEX_CHECKSUM_ADDR, 4, sf_checksum.B08);	
    if( sum != (checksum & 0x0000ffff)){
		return ERROR_ISP_VERIFY;			
	}
       	
    //读出spi flash中的芯片配置信息数据
//    isp_read_config();
    return status;      
}

/*
 * 芯片加密
 */
error_t isp_prog_encrypt_chip(void)
{
    uint32_t value;    
    uint8_t ret;
    
    value = isp_target_dev->encrypt_value;
    ret = isp_program_config(isp_target_dev->encrypt_addr, &(isp_target_dev->encrypt_value), 1);
    if(ret != TRUE)
        return ERROR_ISP_ENCRYPT;    
    return ERROR_SUCCESS;    
}











