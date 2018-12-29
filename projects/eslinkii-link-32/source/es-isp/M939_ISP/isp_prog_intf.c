//M939配置字地址不连续。需要在上位机下发的地址进行重新组装。
#include "eslink.h"
#include "errno.h"
#include "isp_prog_intf.h"
//#include "es_target_set.h"  
#include "sflash_port.h"

//一次编程支持的长度，根据RAM大小可以修改
#define ISP_PRG_MINI_SIZE  1024   

static void isp_init(es_target_cfg *target);
static error_t isp_prog_init(void); //进模式
static error_t isp_prog_uninit(void); //退出模式

static error_t isp_prog_erase_chip (uint8_t para);
static error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t isp_prog_read_chipid(uint32_t *buf);
static error_t isp_prog_read_checksum(uint32_t *buf);
static error_t isp_prog_encrypt_chip(void);
static error_t isp_chipid_check(void);
static error_t isp_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_addr );
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t isp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_target_program_config_all(uint32_t *failed_addr);
static error_t isp_target_program_all(uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr);
static error_t isp_target_verify_all(uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data);

struct  es_prog_ops isp_prog_intf = {
    isp_init,
    isp_prog_init,
    isp_prog_uninit,
    isp_prog_erase_chip,
    isp_prog_check_empty,
    isp_prog_read_chipid,
//    isp_chipid_check,
    isp_prog_read_checksum,
    isp_prog_encrypt_chip,

    isp_prog_program_config,
    isp_prog_read_config,
    isp_prog_verify_config,
    isp_prog_program_flash,
    isp_prog_read_flash,
    isp_prog_verify_flash,
    isp_target_program_config_all ,
    isp_target_program_all,
    isp_target_verify_all,
};


static const es_target_cfg *isp_target_dev;   

void isp_init(es_target_cfg *target)
{
     isp_target_dev = target;
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
        eslink_set_target_hold_reset();        
        status = isp_entry_mode();
        if(ERROR_SUCCESS != status)
            return status; 
    }
    return  ERROR_SUCCESS;
}
//isp退出编程模式
static error_t isp_prog_uninit(void)
{
    isp_out_mode();
    if(isp_mode_check() != TRUE)
        return  ERROR_SUCCESS;
//        isp_set_target_reset(0);
//        PORT_ISP_OFF();  
    return ERROR_OUT_ISP_MODE;
}   

//读目标芯片ID
static error_t isp_prog_read_chipid(uint32_t *buf)
{  
    if(isp_read_config(isp_target_dev->chipid_addr, buf, 1) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS; 
}
 
//判断芯片chipid是否正确  
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
static error_t isp_prog_read_checksum(uint32_t *buf)
{
    if(isp_read_config(CHIP_CHECKSUM_ADDR, buf, 1) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS;     
}
//flash编程
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
    //编程时校验   
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
//读flash
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
//flash校验
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
 * 配置字编程
 * 注意：配置字默认从地址0开始编程，且能一次编程结束
 */
static error_t isp_prog_program_config(uint32_t addr, uint8_t *buf, uint32_t size, uint32_t *failed_addr )
{   
    uint8_t ret ;  

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
         *failed_addr = prog_addr + offset*4 ; 
         return ERROR_ISP_PROG_CFG_WORD;
    } 
    
    buf += prog_size*4; 
    prog_addr  =  addr + 0x7C0;     //info1的偏移地址
    prog_size = 24;     //24个字
    
    ret = isp_program_config(prog_addr, (uint32_t*)buf, prog_size, &offset);
    if(ret != TRUE)
    {
         *failed_addr = prog_addr + offset*4 ; 
         return ERROR_ISP_PROG_CFG_WORD;
    } 

#if ESLINK_RTC_ENABLE  
    buf += prog_size*4; 
    prog_addr  =  addr + 0x1000;     //info1的偏移地址
    prog_size = 16;     //16个字
    
    ret = isp_program_config(prog_addr, (uint32_t*)buf, prog_size, &offset);
    if(ret != TRUE)
    {
         *failed_addr = prog_addr + offset*4 ; 
         return ERROR_ISP_PROG_CFG_WORD;
    } 
#endif    
    return ERROR_SUCCESS;        
}

//读配置字  
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
    read_size = 24;     //24个字
    
    ret = isp_read_config(read_addr, (uint32_t*)buf, read_size) ;
    if( ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD; 

#if ESLINK_RTC_ENABLE  
    buf += read_size*4;    
    read_addr  =  addr + 0x1000;     //rtc info的偏移地址
    read_size = 16;     //16个字
    
    ret = isp_read_config(read_addr, (uint32_t*)buf, read_size) ;
    if( ret != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
#endif
    return ERROR_SUCCESS;        
}
//配置字校验  
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
            if( (*buf++ != ((rd_buf[i]>>0)&0xFF))  ||                        
                (*buf++ != ((rd_buf[i]>>8)&0xFF))  ||
                (*buf++ != ((rd_buf[i]>>16)&0xFF)) ||
                (*buf++ != ((rd_buf[i]>>24)&0xFF)) )
            {
                *failed_addr = read_addr + i*4 ;  
                *failed_data = rd_buf[i];
                return  ERROR_ISP_CFG_WORD_VERIFY;  
            } 
        } 
        read_addr += verify_size;
        read_size -= verify_size;
    }
    
    read_addr  =  addr + 0x7C0;     //info1的偏移地址
    read_size = 24;     //24个字
    while (read_size > 0) 
    {          
        verify_size = MIN(read_size, sizeof(rd_buf));
        ret = isp_read_config(read_addr, rd_buf, verify_size); 
        if( ret != TRUE)
            return ERROR_ISP_READ_CFG_WORD;
        for(i=0; i< verify_size; i++)
        {
            if( (*buf++ != ((rd_buf[i]>>0)&0xFF))  ||                        
                (*buf++ != ((rd_buf[i]>>8)&0xFF))  ||
                (*buf++ != ((rd_buf[i]>>16)&0xFF)) ||
                (*buf++ != ((rd_buf[i]>>24)&0xFF)) )
            {
                *failed_addr = read_addr + i*4 ;  
                *failed_data = rd_buf[i];
                return  ERROR_ISP_CFG_WORD_VERIFY;  
            } 
        } 
        read_addr += verify_size;
        read_size -= verify_size;
    }

#if ESLINK_RTC_ENABLE  
    read_addr  =  addr + 0x1000;     //rtc info的偏移地址
    read_size = 16;     //16个字
    while (read_size > 0) 
    {          
        verify_size = MIN(read_size, sizeof(rd_buf));
        ret = isp_read_config(read_addr, rd_buf, verify_size); 
        if( ret != TRUE)
            return ERROR_ISP_READ_CFG_WORD;
        for(i=0; i< verify_size; i++)
        {
            if( (*buf++ != ((rd_buf[i]>>0)&0xFF))  ||                        
                (*buf++ != ((rd_buf[i]>>8)&0xFF))  ||
                (*buf++ != ((rd_buf[i]>>16)&0xFF)) ||
                (*buf++ != ((rd_buf[i]>>24)&0xFF)) )
            {
                *failed_addr = read_addr + i*4 ;  
                *failed_data = rd_buf[i];
                return  ERROR_ISP_CFG_WORD_VERIFY;  
            } 
        } 
        read_addr += verify_size;
        read_size -= verify_size;
    }      
#endif
    return ERROR_SUCCESS;       
}

// 芯片加密 
static error_t isp_prog_encrypt_chip(void)
{ 
    error_t ret;
   
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    ret = isp_program_config(isp_target_dev->encrypt_addr, (uint32_t *)&isp_target_dev->encrypt_value, 1, NULL);
    if(ret != TRUE)
        return ERROR_ISP_ENCRYPT;    
    return ERROR_SUCCESS;    
}

//擦除
static error_t isp_prog_erase_chip (uint8_t  para) 
{
    error_t ret = ERROR_SUCCESS;
//    uint8_t erase_mode;    
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret;   
        
    if(isp_erase_chip() != TRUE)
		 return ERROR_ISP_ERASE;
         
    return ERROR_SUCCESS;
}


/*******************************************************************************
*	函 数 名: isp_prog_check_empty
*	功能说明: 查空
*	形    参: failed_addr：错误地址  failed_data：错误数据
*	返 回 值: 错误类型
*******************************************************************************/
error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failed_data)           
{
    error_t ret = ERROR_SUCCESS;
	uint32_t i;
	uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	
    
    uint32_t read_buf[ISP_PRG_MINI_SIZE/4]; 
	uint32_t copy_size;   
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret;   
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
	cfg_word_size =  24;     //字长度
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
    #if ESLINK_RTC_ENABLE  
    cfg_word_addr =  isp_target_dev->config_word_start + 0x1000;
	cfg_word_size =  16;     //字长度
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
   #endif
    return ERROR_SUCCESS;     
}  

/*******************************************************************************
*	函 数 名: isp_target_program_config_all
*	功能说明: 芯片配置字编程。
*	形    参: failed_addr：错误地址  
*	返 回 值: 编程错误地址
*******************************************************************************/
static error_t isp_target_program_config_all(uint32_t *failed_addr)
{
    error_t ret = ERROR_SUCCESS;
    
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	 
    
    uint32_t copy_size;
    uint32_t read_addr;
    uint8_t read_buf[FLASH_PRG_MIN_SIZE] = {0x00};
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    cfg_word_addr =  isp_target_dev->config_word_start;
	cfg_word_size =  isp_target_dev->config_word_size;
    read_addr =  0;

    while(true)
    {
        copy_size = MIN(cfg_word_size, sizeof(read_buf) );
        
        ret = online_file_read(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;     
        ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
        if(ERROR_SUCCESS != ret)
            return ret;              
        // Update variables
        cfg_word_addr  += copy_size;
        cfg_word_size  -= copy_size;
        read_addr += copy_size;
        // Check for end
        if (cfg_word_size <= 0) 
            break;       
    }
    return ERROR_SUCCESS;     
} 
    
/*******************************************************************************
*	函 数 名: isp_prog_program_flash
*	功能说明: 芯片编程。flash和配置字编程
*	形    参: sn_enable：是否已编程序列号 sn：序列号代码 
*             failed_addr：错误地址   failed_data ：错误数据
*	返 回 值: 错误类型
*******************************************************************************/
static error_t isp_target_program_all(uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr) 
{
    error_t ret = ERROR_SUCCESS;
    uint32_t i;
    
    uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	   
    
    uint32_t copy_size;      
    uint32_t read_addr;
    uint8_t read_buf[FLASH_PRG_MIN_SIZE];
       
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  isp_target_dev->code_start;
	code_size =  isp_target_dev->code_size;
    read_addr =  0; 
        
    while(true)
    {
        copy_size = MIN(code_size, sizeof(read_buf) );    
        
        ret = online_file_read(USER_HEX, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;
        if(sn_enable == ENABLE)     //序列号代码使能
            serial_number_intercept_write(sn ,code_addr, read_buf, copy_size);	//填入序列号
        for(i=0; i<copy_size; i++)
        {
            if(read_buf[i] != 0xFF)
                break;
        }
        if(i < copy_size)      //数据段都为0xFF,不进行编程
        {
            ret = isp_prog_program_flash(code_addr, read_buf, copy_size, failed_addr); 
            if( ret !=  ERROR_SUCCESS)   //编程失败，返回编程失败地址
                return ret;
        }           
        // Update variables
        code_addr  += copy_size;
        code_size  -= copy_size;
        read_addr += copy_size;
        // Check for end
        if (code_size <= 0) 
            break;       
    }
    cfg_word_addr =  isp_target_dev->config_word_start;
	cfg_word_size =  isp_target_dev->config_word_size;
    read_addr =  0;
    while(true)
    {
        copy_size = MIN(cfg_word_size, sizeof(read_buf) );          
        ret = online_file_read(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;     		
        ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
        if( ret !=  ERROR_SUCCESS)
            return ret;             
        // Update variables
        cfg_word_addr  += copy_size;
        cfg_word_size  -= copy_size;
        read_addr += copy_size;
        // Check for end
        if (cfg_word_size <= 0) 
            break;       
    }
    return ret;  
}

/*******************************************************************************
*	函 数 名: isp_target_verify_all
*	功能说明: 芯片验证
*	形    参: sn_enable：是否已编程序列号 sn：序列号代码 
*             failed_addr：错误地址   
*	返 回 值: 错误类型
*******************************************************************************/
static error_t  isp_target_verify_all(uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data)
{       
    error_t ret = ERROR_SUCCESS;

    uint32_t checksum = 0;  
    uint32_t sf_checksum = 0;   //spi保存的校验和   
    
    uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	   
//    uint8_t read_buf[FLASH_PRG_MIN_SIZE];
    
    uint32_t verify_size; 
    uint32_t sf_addr;  
    uint8_t sf_buf[FLASH_PRG_MIN_SIZE];     
  
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  isp_target_dev->code_start;
	code_size =  isp_target_dev->code_size;
    sf_addr = 0;
    while(true)
    {
        verify_size = MIN(code_size, sizeof(sf_buf) );
       
        ret = online_file_read(USER_HEX, sf_addr, sf_buf , verify_size);
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        if( sn_enable == ENABLE)
            serial_number_intercept_write(sn, code_addr, sf_buf, verify_size);	//填入序列号         
        ret = isp_prog_verify_flash(code_addr, sf_buf, verify_size,failed_addr,failed_data);                        
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        // Update variables
        code_addr  += verify_size;
        code_size  -= verify_size;
        sf_addr += verify_size;
        // Check for end
        if (code_size <= 0) 
            break;       
    }  
    online_file_read(HEX_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if((sf_checksum&0x0000ffff) != (checksum&0x0000ffff))
    {
        ret = ERROR_USER_HEX_CHECKSUM;
        return  ret;
    }         
    
    cfg_word_addr =  isp_target_dev->config_word_start;
	cfg_word_size =  isp_target_dev->config_word_size;
    sf_addr =  0;
    checksum = 0;
    while(true)
    {
        verify_size = MIN(cfg_word_size, sizeof(sf_buf) );          
        ret = online_file_read(CFG_WORD, sf_addr, sf_buf , verify_size);
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        
        ret = isp_prog_verify_config(cfg_word_addr, sf_buf, verify_size,failed_addr,failed_data); 
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        // Update variables
        cfg_word_addr  += verify_size;
        cfg_word_size  -= verify_size;
        sf_addr += verify_size;
        // Check for end
        if (cfg_word_size <= 0) 
            break;       
    }
    online_file_read(CFG_WORD_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if(sf_checksum != (checksum&0x0000ffff))
    {
        ret = ERROR_CFG_WORD_CHECKSUM;
        return  ret; 
    }           
    return  ret; 
}

//#if ESLINK_RTC_ENABLE  

///*******************************************************************************
//*	函 数 名:  isp_read_rtc_info
//*	功能说明: 读rtc info区的值
//*	形    参: addr：地址 buf：数据 size：长度
//*	返 回 值: 错误类型
//*******************************************************************************/
//error_t isp_read_rtc_info( uint32_t addr, uint8_t *buf, uint32_t size)
//{
//    uint8_t ret;
//    uint32_t size_in_words; 
//    uint32_t read_addr;
// 
//    read_addr  =  0x1000 + addr;     //rtc info的偏移地址
//    size_in_words = size /4 ;     //16个字
//    
//    ret = isp_read_config(read_addr, (uint32_t*)buf, size_in_words) ;
//    if( ret != TRUE)
//        return ERROR_ISP_READ_CFG_WORD; 
//    return ERROR_SUCCESS;
//}

///*******************************************************************************
//*	函 数 名: isp_write_rtc_info
//*	功能说明: 写rtc info区的值
//*	形    参: addr：地址 buf：数据 size：长度
//*	返 回 值: 错误类型
//*******************************************************************************/
//error_t isp_program_rtc_info( uint32_t addr, uint8_t *buf, uint32_t size)
//{   
//    uint8_t ret ;  
//    uint32_t size_in_words;    
//    uint32_t prog_addr;
//    uint32_t offset;     
//    
//    if(size & 0x03)
//        return ERROR_OUT_OF_BOUNDS;
//    size_in_words = size/4;      
//    prog_addr = addr + 0x1000;
//    ret = isp_program_config(prog_addr, (uint32_t*)buf, size_in_words, &offset);
//    if(ret != TRUE)
//    {
//         return ERROR_ISP_PROG_CFG_WORD;
//    }    
//}

//#endif


