//M939配置字地址不连续。需要在上位机下发的地址进行重新组装。

#include "string.h"
#include "stdbool.h"
#include "eslink.h"
#include "errno.h"
#include "isp.h"
#include "isp_prog_intf.h"
#include "es_target_set.h"  

#define M939_CONFIG_WORD_SIZE       38      //38个字长

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


static const es_target_cfg *isp_target_dev;   

void isp_init(es_target_cfg *target)
{
     isp_target_dev = target;
}
void isp_set_target_reset(uint8_t asserted)
{
    (asserted) ? PIN_RST_OUT(0) : PIN_RST_OUT(1);
}

//进入isp模式
static error_t isp_entry_mode(void)
{
    //复位
    isp_reset();
    //读取ID
    if(isp_id_check() != TRUE)
        return ERROR_IN_ISP_MODE;
    //解锁
    if(isp_unlock_check() != TRUE)
        return ERROR_ISP_UNLOCK;
    //进ISP模式
    if( isp_mode_set() != TRUE)
        return ERROR_IN_ISP_MODE;
    //加载加密字
    if( isp_encrypt_check() != TRUE)
        return ERROR_IN_ISP_MODE;
    
    return ERROR_SUCCESS; 
}

//退出isp模式
static error_t isp_out_mode(void)
{      
    isp_reset(); 
    return ERROR_SUCCESS; 
}
//判断是否检测到芯片 
error_t isp_chip_check(void)
{
    //复位
    isp_reset();
    //读取ID
    if(isp_id_check() != TRUE)
        return ERROR_IN_ISP_MODE;
    return ERROR_SUCCESS; 
}
//编程初始化，进模式
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
        
        status = isp_entry_mode();
        if(ERROR_SUCCESS != status)
            return status; 
    }
    return  ERROR_SUCCESS;
}
//isp联机编程后，退出模式
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
    {
          //测试模式，不判断ID
//         return  ERROR_CHIP_ID_NOT_MATCH;
    }
        
    return ERROR_SUCCESS; 

}
//读芯片校验和
static error_t isp_prog_read_chip_chksum(uint32_t *buf)
{
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
     
    if(isp_erase_chip() != TRUE)
		 status = ERROR_ISP_ERASE;
    return status;
}
/*
 * 查空。 
 */ 
static error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failed_data)                             
{
	uint32_t i;
	uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	
    
    uint32_t read_buf[ISP_PRG_MINI_SIZE/4]; 
	uint32_t copy_size; 
    
    //flash查空
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
    //配置字查空
	cfg_word_addr =  isp_target_dev->config_word_start + 0x400;
	cfg_word_size =  14;     //字长度
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
    
	cfg_word_addr =  isp_target_dev->config_word_start + 0x7C0;
	cfg_word_size =  M939_CONFIG_WORD_SIZE - 14;     //字长度
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
    uint32_t offset;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
    
    ret = isp_program_code( addr, (uint32_t*)data, size_in_words,&offset);
    if(ret != TRUE)
    {
         *failed_addr = addr + offset*4 ; 
         return ERROR_ISP_PROG;
    }
        
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
static error_t isp_prog_programe_config(uint32_t addr, uint8_t *buf, uint32_t size, uint32_t *failed_addr )
{   
    uint8_t ret ;  
//    uint32_t i;
//    uint32_t rd_buf[64];
//    uint32_t verify_size;
    uint32_t size_in_words;  
    uint32_t offset;    
    uint32_t prog_addr;
    uint32_t prog_size;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
	if( size_in_words != M939_CONFIG_WORD_SIZE)
        return ERROR_ISP_PROG_CFG_WORD;
    prog_addr  =  addr + 0x400;     //info1的偏移地址
    prog_size = 14;     //14个字
    ret = isp_program_config(prog_addr, (uint32_t*)buf, prog_size, &offset);
    if(ret != TRUE)
    {
         *failed_addr = addr + offset*4 ; 
         return ERROR_ISP_PROG_CFG_WORD;
    } 
    
    buf += prog_size*4; 
    prog_addr  =  addr + 0x7C0;     //info1的偏移地址
    prog_size = M939_CONFIG_WORD_SIZE- 14;     //28个字
    
    ret = isp_program_config(prog_addr, (uint32_t*)buf, prog_size, &offset);
    if(ret != TRUE)
    {
         *failed_addr = addr + offset*4 ; 
         return ERROR_ISP_PROG_CFG_WORD;
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
    uint32_t read_addr;
    uint32_t read_size;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4; 
    if( size_in_words != M939_CONFIG_WORD_SIZE)
        return ERROR_ISP_PROG_CFG_WORD;

    //配置字在inf1区，保留未用的数据未下发。此信息需要跟软件组沟通
    read_addr  =  addr + 0x400;     //info1的偏移地址
    read_size = 14;     //14个字
    ret = isp_read_config(read_addr, (uint32_t*)buf, read_size) ;
    if( ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;  
        
    buf += read_size*4;    
    read_addr  =  addr + 0x7C0;     //info1的偏移地址
    read_size = M939_CONFIG_WORD_SIZE - 14;     //24个字
    
    ret = isp_read_config(read_addr, (uint32_t*)buf, read_size) ;
    if( ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS;        
}
/*
 * 配置字校验
 * return :   ERROR_SUCCESS   
 */
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *buf, uint32_t size,uint32_t *failed_addr, uint32_t *failed_data)
{
    uint8_t ret ; 
    uint32_t i;
    uint32_t rd_buf[ISP_PRG_MINI_SIZE/4];
    uint32_t verify_size;
    uint32_t size_in_words;    
    
    uint32_t read_addr;
    uint32_t read_size;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
    if( size_in_words != M939_CONFIG_WORD_SIZE)
        return ERROR_ISP_PROG_CFG_WORD;
    
    //配置字在inf1区，保留未用的数据未下发。此信息需要跟软件组沟通
    read_addr  =  addr + 0x400;     //info1的偏移地址
    read_size = 14;     //14个字    
    while (read_size > 0) 
    {          
        verify_size = MIN(read_size, sizeof(rd_buf));
        ret = isp_read_config(read_addr, rd_buf, verify_size); 
        if( ret != TRUE)
            return ERROR_ISP_READ_CFG_WORD;
        for(i=0; i< verify_size; i++)
        {
            if( (buf[i*4]   != ((rd_buf[i]>>0)&0xFF))  ||                        
                (buf[i*4+1] != ((rd_buf[i]>>8)&0xFF))  ||
                (buf[i*4+2] != ((rd_buf[i]>>16)&0xFF)) ||
                (buf[i*4+3] != ((rd_buf[i]>>24)&0xFF)) )
            {
                *failed_addr = read_addr + i*4 ;  
                *failed_data = rd_buf[i];
                return  ERROR_ISP_CFG_WORD_VERIFY;  
            } 
        } 
        read_addr += verify_size;
        size_in_words -= verify_size;
    }
    
    read_addr  =  addr + 0x7C0;     //info1的偏移地址
    read_size = M939_CONFIG_WORD_SIZE - 14;     //28个字
    while (read_size > 0) 
    {          
        verify_size = MIN(read_size, sizeof(rd_buf));
        ret = isp_read_config(read_addr, rd_buf, verify_size); 
        if( ret != TRUE)
            return ERROR_ISP_READ_CFG_WORD;
        for(i=0; i< verify_size; i++)
        {
            if( (buf[i*4]   != ((rd_buf[i]>>0)&0xFF))  ||                        
                (buf[i*4+1] != ((rd_buf[i]>>8)&0xFF))  ||
                (buf[i*4+2] != ((rd_buf[i]>>16)&0xFF)) ||
                (buf [i*4+3] != ((rd_buf[i]>>24)&0xFF)) )
            {
                *failed_addr = read_addr + i*4 ;  
                *failed_data = rd_buf[i];
                return  ERROR_ISP_CFG_WORD_VERIFY;  
            } 
        } 
        read_addr += verify_size;
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
   
    ret = isp_program_config(isp_target_dev->encrypt_addr, (uint32_t *)&isp_target_dev->encrypt_value, 1, NULL);
    if(ret != TRUE)
        return ERROR_ISP_ENCRYPT;    
    return ERROR_SUCCESS;    
}











