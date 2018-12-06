#include "string.h"
#include "stdbool.h"
#include "eslink.h"
#include "errno.h"
#include "isp.h"
#include "isp_prog_intf.h"
#include "es_target_set.h"  



static void isp_init(es_target_cfg *target);
static error_t isp_prog_init(void); //进模式
static error_t isp_prog_uninit(void); //退出模式

static error_t isp_prog_erase_chip (uint8_t *para);
static error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t isp_prog_read_chipid(uint32_t *buf);
static error_t isp_prog_read_chip_chksum(uint32_t *buf);
static error_t isp_prog_encrypt_chip(void);
static error_t isp_chipid_check(void);
static error_t isp_prog_programe_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_addr );
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t isp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);

struct  es_prog_ops isp_prog_intf = {
    isp_init,
    isp_prog_init,
    isp_prog_uninit,
    isp_prog_erase_chip,
    isp_prog_check_empty,
    isp_prog_read_chipid,
    isp_chipid_check,
    isp_prog_read_chip_chksum,
    isp_prog_encrypt_chip,

    isp_prog_programe_config,
    isp_prog_read_config,
    isp_prog_verify_config,
    isp_prog_program_flash,
    isp_prog_read_flash,
    isp_prog_verify_flash,
};

#define ISP_PRG_MINI_SIZE  1024 

static const es_target_cfg *isp_target_dev;   

void isp_init(es_target_cfg *target)
{
     isp_target_dev = target;
}
void isp_set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_RST_OUT(0) : PIN_RST_OUT(1);
}


static error_t isp_prog_init(void)
{
     error_t status;
     
    if(isp_mode_check() != TRUE)    //判断是否在isp模式
    {
        PORT_ISP_SETUP();
        isp_set_target_reset(1);
        es_set_trget_power(TRGET_POWER_DISABLE);
        es_delay_ms(10);
        es_set_trget_power(TRGET_POWER_ENABLE);
//        isp_set_target_reset(0);
//        es_delay_ms(10);         
        
        status = (error_t)isp_entry_mode();
        if(ERROR_SUCCESS != status)
            return status; 
    }
    return  ERROR_SUCCESS;
}
static error_t isp_prog_uninit(void)
{
    isp_out_mode();
    if(isp_mode_check() != TRUE)
        return  ERROR_SUCCESS;
//        isp_set_target_reset(0);
//        PORT_ISP_OFF();

    return ERROR_OUT_ISP_MODE;
}   

/*******************************************************************************
*函数名：
* 描述 ：读目标芯片ID
* 输入 ：data：数据。
* 输出 ：
*******************************************************************************/
static error_t isp_prog_read_chipid(uint32_t *buf)
{    
//    error_t status = ERROR_SUCCESS;
    
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status;

    if(isp_read_config(isp_target_dev->chipid_addr, buf, 1) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS; 
}
//判断chip id
static error_t isp_chipid_check(void)
{
    uint32_t chipid = 0;
   
    if(isp_read_config(isp_target_dev->chipid_addr, &chipid, 1) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    if(chipid != isp_target_dev->chipid_value)    
        return  ERROR_CHIP_ID_NOT_MATCH;
    return ERROR_SUCCESS; 

}
//读芯片校验和
static error_t isp_prog_read_chip_chksum(uint32_t *buf)
{
//    error_t status = ERROR_SUCCESS;
    
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status;
    if(isp_read_config(CHIP_CHECKSUM_ADDR, buf, 1) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS;     
}

/*
 * 擦除
 */
static error_t isp_prog_erase_chip (uint8_t * para) 
{
    error_t status = ERROR_SUCCESS;
    
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status;  
  
    if(isp_erase_chip() != TRUE)
		 status = ERROR_ISP_ERASE;
    return status;
}
/*
 * 查空。 
 */ 
static error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failed_data)                             
{
//    error_t status = ERROR_SUCCESS;
	uint32_t i;
	uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	
    
    uint32_t read_buf[ISP_PRG_MINI_SIZE/4]; 
	uint32_t copy_size; 
    
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status; 
    
	code_addr =  isp_target_dev->code_start;
	code_size =  isp_target_dev->code_size / 4; //字长度
	while(true)
	{
		copy_size = MIN(code_size, sizeof(read_buf)/4 );      
	    isp_read_code(code_addr, read_buf, copy_size);
		for(i = 0; i<copy_size; i++)
		{
			if(read_buf[i] != 0xFFFFFFFF)
			{              
			    *failed_addr = code_addr + i*4  ;
                *failed_data = read_buf[i] ; 
				return ERROR_ISP_FLASH_CHECK_EMPTY;
			} 				
		} 
        // Update variables
        code_addr  += copy_size*4;
        code_size  -= copy_size*4;
        
        // Check for end
        if (code_size <= 0) {
            break;
        } 
	}  
	cfg_word_addr =  isp_target_dev->config_word_start;
	cfg_word_size =  isp_target_dev->config_word_size/4;     //字长度
    while(true)
	{
		copy_size = MIN(cfg_word_size, sizeof(read_buf)/4 );
	    isp_read_config(cfg_word_addr, read_buf, copy_size);
		for(i = 0; i<copy_size; i++)
		{
			if(read_buf[i] != 0xFFFFFFFF)
			{              
			    *failed_addr = cfg_word_addr + i*4  ;
                *failed_data = read_buf[i] ; 
				return ERROR_ISP_CFG_WORD_CHECK_EMPTY;
			} 				
		} 
        // Update variables
        cfg_word_addr  += copy_size;
        cfg_word_size  -= copy_size;
        
        // Check for end
        if (code_size <= 0) {
            break;
        } 
	} 
    return ERROR_SUCCESS;      
} 
/*******************************************************************************
*函数名：flash编程
* 描述 ：
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr)
{
    uint8_t ret ; 
    uint32_t i;
    uint32_t rd_buf[ISP_PRG_MINI_SIZE/4];
    uint32_t verify_size;
    uint32_t size_in_words;    
//    error_t status = ERROR_SUCCESS;
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status; 
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
    
    ret = isp_program_code( addr, (uint32_t*)data, size_in_words,failed_addr);
    if(ret != TRUE)
        return ERROR_ISP_PROG;
    if (config_isp_verify_program())
    {
        while (size_in_words > 0) 
        {          
            verify_size = MIN(size_in_words, sizeof(rd_buf));
            ret = isp_read_code(addr, rd_buf, verify_size); 
            if( ret != TRUE)
                return ERROR_ISP_READ_CFG_WORD;
            for(i=0; i< verify_size; i++)
            {
                if( (data[i*4]   != ((rd_buf[i]>>0)&0xFF))  ||                        
                    (data[i*4+1] != ((rd_buf[i]>>8)&0xFF))  ||
                    (data[i*4+2] != ((rd_buf[i]>>16)&0xFF)) ||
                    (data[i*4+3] != ((rd_buf[i]>>24)&0xFF)) )
                {
                    *failed_addr = addr + i*4 ;  
                    return  ERROR_ISP_VERIFY;  
                } 
            } 
            addr += verify_size;
            size_in_words -= verify_size;
        }  
    }
    return ERROR_SUCCESS;     
}
/*******************************************************************************
*函数名：读flash
* 描述 ：
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size)
{
    uint8_t ret;
    uint32_t size_in_words; 
//    error_t status = ERROR_SUCCESS;
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;    
    ret = isp_read_code(addr, (uint32_t*)data, size_in_words) ;
    if( ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS;  

}
/*
 * flash校验
 * return :   ERROR_SUCCESS   
 */
static error_t isp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
    uint8_t ret ; 
    uint32_t i;
    uint32_t rd_buf[ISP_PRG_MINI_SIZE/4];
    uint32_t verify_size;
    uint32_t size_in_words;    
//    error_t status = ERROR_SUCCESS;
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
    
    while (size_in_words > 0) 
    {          
        verify_size = MIN(size_in_words, sizeof(rd_buf));
        ret = isp_read_code(addr, rd_buf, verify_size); 
        if( ret != TRUE)
            return ERROR_ISP_READ_CFG_WORD;
        for(i=0; i< verify_size; i++)
        {
            if( (data[i*4]   != ((rd_buf[i]>>0)&0xFF))  ||                        
                (data[i*4+1] != ((rd_buf[i]>>8)&0xFF))  ||
                (data[i*4+2] != ((rd_buf[i]>>16)&0xFF)) ||
                (data[i*4+3] != ((rd_buf[i]>>24)&0xFF)) )
            {
                *failed_addr = addr + i*4 ;  
                *failed_data = rd_buf[i];
                return  ERROR_ISP_VERIFY;  
            } 
        } 
        addr += verify_size;
        size_in_words -= verify_size;
    }
    
    return ERROR_SUCCESS;      
}
/*
 *配置字编程
 * ERROR_ISP_PROG_CONFIG
 */
static error_t isp_prog_programe_config(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr )
{   
    uint8_t ret ;  
    uint32_t i;
    uint32_t rd_buf[64];
    uint32_t verify_size;
    uint32_t size_in_words;          
//    error_t status = ERROR_SUCCESS;
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
	        
    ret = isp_program_config(addr, (uint32_t*)data, size_in_words, failed_addr);
    if(ret != TRUE)
        return ERROR_ISP_PROG_CFG_WORD;
    if (config_isp_verify_program())
    {
        while (size_in_words > 0) 
        {          
            verify_size = MIN(size_in_words, sizeof(rd_buf));
            ret = isp_read_config(addr, rd_buf, verify_size); 
            if( ret != TRUE)
                return ERROR_ISP_READ_CFG_WORD;
            for(i=0; i< verify_size; i++)
            {
                if( (data[i*4]   != ((rd_buf[i]>>0)&0xFF))  ||                        
                    (data[i*4+1] != ((rd_buf[i]>>8)&0xFF))  ||
                    (data[i*4+2] != ((rd_buf[i]>>16)&0xFF)) ||
                    (data[i*4+3] != ((rd_buf[i]>>24)&0xFF)) )
                {
                    *failed_addr = addr + i*4 ;  
                    return  ERROR_ISP_PROG;  
                } 
            } 
            addr += verify_size;
            size_in_words -= verify_size;
        }     
    }

    return ERROR_SUCCESS;        
}

/*
 *读配置字
 * return :   ERROR_SUCCESS   
 */
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size)
{
    uint8_t ret;
    uint32_t size_in_words; 
//    error_t status = ERROR_SUCCESS;
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status;    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;    
    ret = isp_read_config(addr, (uint32_t*)buf, size_in_words) ;
    if( ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS;        
}
/*
 * 配置字校验
 * return :   ERROR_SUCCESS   
 */
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size,uint32_t *failed_addr, uint32_t *failed_data)
{
    uint8_t ret ; 
    uint32_t i;
    uint32_t rd_buf[ISP_PRG_MINI_SIZE/4];
    uint32_t verify_size;
    uint32_t size_in_words;    
//    error_t status = ERROR_SUCCESS;
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
    
    while (size_in_words > 0) 
    {          
        verify_size = MIN(size_in_words, sizeof(rd_buf));
        ret = isp_read_config(addr, rd_buf, verify_size); 
        if( ret != TRUE)
            return ERROR_ISP_READ_CFG_WORD;
        for(i=0; i< verify_size; i++)
        {
            if( (data[i*4]   != ((rd_buf[i]>>0)&0xFF))  ||                        
                (data[i*4+1] != ((rd_buf[i]>>8)&0xFF))  ||
                (data[i*4+2] != ((rd_buf[i]>>16)&0xFF)) ||
                (data[i*4+3] != ((rd_buf[i]>>24)&0xFF)) )
            {
                *failed_addr = addr + i*4 ;  
                *failed_data = rd_buf[i];
                return  ERROR_ISP_CFG_WORD_VERIFY;  
            } 
        } 
        addr += verify_size;
        size_in_words -= verify_size;
    }
    
    return ERROR_SUCCESS;       
}


/*
 * 芯片加密
 */
static error_t isp_prog_encrypt_chip(void)
{ 
    uint8_t ret;
//    error_t status = ERROR_SUCCESS;
//    status = isp_prog_init();
//    if(ERROR_SUCCESS != status)
//        return  status;
    
    ret = isp_program_config(isp_target_dev->encrypt_addr, (uint32_t *)&isp_target_dev->encrypt_value, 1, NULL);
    if(ret != TRUE)
        return ERROR_ISP_ENCRYPT;    
    return ERROR_SUCCESS;    
}











