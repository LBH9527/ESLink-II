#include "eslink.h"
#include "errno.h"
#include "isp_prog_intf.h" 
#include "sflash_port.h"  
#include "isp_port.h" 
#include "./ES_ISP.h"
#include "./target_info.h"

static void isp_init(es_target_cfg *target);
static error_t isp_prog_init(void); //进模式
static error_t isp_prog_uninit(void); //退出模式

static error_t isp_prog_erase_chip (uint8_t para);
static error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t isp_prog_read_chipid(uint8_t *buf);
static error_t isp_prog_read_chip_chksum(uint8_t *buf);
static error_t isp_prog_encrypt_chip(void);
static error_t isp_chipid_check(void);
static error_t isp_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_addr );
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t isp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_target_program_config_all(uint32_t *failed_addr);
static error_t isp_target_program_all(  uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr);
static error_t isp_target_verify_all( uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data);

static error_t read_save_info1(uint8_t *data);
static error_t judge_write_info1(uint8_t *data);

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

    isp_prog_program_config,
    isp_prog_read_config,
    isp_prog_verify_config,
    isp_prog_program_flash,
    isp_prog_read_flash,
    isp_prog_verify_flash,
    isp_target_program_config_all ,
    isp_target_program_all,
    isp_target_verify_all,
    0,
};

#define ISP_PRG_MINI_SIZE  1024 

static const es_target_cfg *isp_target_dev;   

void isp_init(es_target_cfg *target)
{
    isp_target_dev = target;
    isp_prog_intf.cb = online_file_read;
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

static error_t isp_prog_init(void)
{
     error_t status;
     
    if(isp_mode_check() != TRUE)    //判断是否在isp模式
    {
        PORT_ISP_SETUP();
        eslink_set_target_hold_reset(20);        
        
        status = isp_entry_mode();
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
*函数名：isp_prog_read_chipid
* 描述 ：读目标芯片ID
* 输入 ：data：数据。
* 输出 ：
*******************************************************************************/
static error_t isp_prog_read_chipid(uint8_t *buf)
{    
    if(isp_read_config(isp_target_dev->chipid_addr, (uint32_t*)buf, 1) != TRUE)
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
static error_t isp_prog_read_chip_chksum(uint8_t *buf)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t info1_data[CHIP_INFO1_SIZE] = {0};    
  
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //info1 读取    
    ret = read_save_info1(info1_data);
    if(ERROR_SUCCESS != ret)
        return ret;
        
    if(isp_read_config(CHIP_CHECKSUM_ADDR, (uint32_t*)buf, 1) != TRUE)
    {
        ret=  ERROR_ISP_READ_CFG_WORD;
        goto __fail; 
    }          
    ret = judge_write_info1(info1_data);
    return  ret;     
__fail:
    judge_write_info1(info1_data);
    return  ret;    
}

/*
 * 擦除
 */
static error_t isp_prog_erase_chip (uint8_t para) 
{
    error_t ret = ERROR_SUCCESS;
    uint8_t info1_data[CHIP_INFO1_SIZE] = {0};    
  
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //info1 读取    
    ret = read_save_info1(info1_data);
    if(ERROR_SUCCESS != ret)
        return ret;
    
    if(isp_erase_chip() != TRUE)
    {
        ret=  ERROR_ISP_ERASE;
        goto __fail; 
    }
    
    ret = judge_write_info1(info1_data);
    return  ret;     
__fail:
    judge_write_info1(info1_data);
    return  ret; 
}
/*
 * 查空。 
 */ 
static error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failed_data)                             
{
    error_t ret = ERROR_SUCCESS;
    uint32_t i;
    uint32_t code_addr;	
    uint32_t code_size;	
    uint32_t cfg_word_addr;	
    uint32_t cfg_word_size;	
    
    uint32_t read_buf[ISP_PRG_MINI_SIZE/4]; 
    uint32_t copy_size; 
       
    uint8_t info1_data[CHIP_INFO1_SIZE] = {0};    
  
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //info1 读取    
    ret = read_save_info1(info1_data);
    if(ERROR_SUCCESS != ret)
        return ret;
        
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
                if(failed_addr)
                    *failed_addr = code_addr + i*4  ;
                if( failed_data)
                    *failed_data = read_buf[i] ; 
                ret=  ERROR_ISP_FLASH_CHECK_EMPTY;
                goto __fail; 
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
                if(failed_addr)
                    *failed_addr = cfg_word_addr + i*4  ;
                if(failed_data)
                    *failed_data = read_buf[i] ; 
                ret=  ERROR_ISP_FLASH_CHECK_EMPTY;
                goto __fail; 
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
    ret = judge_write_info1(info1_data);
    return  ret;     
__fail:
    judge_write_info1(info1_data);
    return  ret;   
} 
/*******************************************************************************
*函数名：flash编程
* 描述 ：
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr)
{
    uint32_t size_in_words; 
    uint32_t offset;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
    
    if(isp_program_code( addr, (uint32_t*)data, size_in_words,&offset) != TRUE)
    {
        if( failed_addr)
            *failed_addr =  addr + offset * 4;
        return ERROR_ISP_PROG;
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
    error_t ret = ERROR_SUCCESS;
    uint32_t size_in_words; 
    uint8_t info1_data[CHIP_INFO1_SIZE] = {0};    
  
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //info1 读取    
    ret = read_save_info1(info1_data);
    if(ERROR_SUCCESS != ret)
        return ret;
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;    
    if(isp_read_code(addr, (uint32_t*)data, size_in_words) != TRUE)
    {
        ret=  ERROR_ISP_READ;
        goto __fail;      
    }

    ret = judge_write_info1(info1_data);
    return  ret;     
__fail:
    judge_write_info1(info1_data);
    return  ret;    

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
            return ERROR_ISP_READ;
        for(i=0; i< verify_size; i++)
        {
            if( (data[i*4]   != ((rd_buf[i]>>0)&0xFF))  ||                        
                (data[i*4+1] != ((rd_buf[i]>>8)&0xFF))  ||
                (data[i*4+2] != ((rd_buf[i]>>16)&0xFF)) ||
                (data[i*4+3] != ((rd_buf[i]>>24)&0xFF)) )
            {
                if( failed_addr)
                    *failed_addr = addr + i*4 ;
                if( failed_data)    
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
static error_t isp_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr )
{   
    uint32_t size_in_words;
    uint32_t offset;    
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
    
    if(isp_program_config(addr, (uint32_t*)data, size_in_words, &offset) != TRUE)
    {
        if(failed_addr)
            *failed_addr =  addr + offset * 4;
        return ERROR_ISP_PROG_CFG_WORD;          
    } 
    return  ERROR_SUCCESS;         
}

/*
 *读配置字
 * return :   ERROR_SUCCESS   
 */
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size)
{
    error_t ret = ERROR_SUCCESS ;  
    uint32_t size_in_words; 
    uint8_t info1_data[CHIP_INFO1_SIZE] = {0};    
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //info1 读取    
    ret = read_save_info1(info1_data);
    if(ERROR_SUCCESS != ret)
        return ret;
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;    
    if(isp_read_config(addr, (uint32_t*)buf, size_in_words) != TRUE)
    {
        ret = ERROR_ISP_READ_CFG_WORD;
        goto __fail; 
    }

    ret = judge_write_info1(info1_data);
    return  ret;     
__fail:
    judge_write_info1(info1_data);
    return  ret;       
}
/*
 * 配置字校验
 * return :   ERROR_SUCCESS   
 */
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size,uint32_t *failed_addr, uint32_t *failed_data)
{ 
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
        if(isp_read_config(addr, rd_buf, verify_size) != TRUE)
        {
            return ERROR_ISP_READ_CFG_WORD;
        }
        for(i=0; i< verify_size; i++)
        {
            if( (data[i*4]   != ((rd_buf[i]>>0)&0xFF))  ||                        
                (data[i*4+1] != ((rd_buf[i]>>8)&0xFF))  ||
                (data[i*4+2] != ((rd_buf[i]>>16)&0xFF)) ||
                (data[i*4+3] != ((rd_buf[i]>>24)&0xFF)) )
            {
                if( failed_addr)
                    *failed_addr = addr + i*4 ;
                if( failed_data)
                    *failed_data = rd_buf[i];
                return ERROR_ISP_CFG_WORD_VERIFY; 
            } 
        } 
        addr += verify_size;
        size_in_words -= verify_size;
    }
    
    return  ERROR_SUCCESS;     
}


/*
 * 芯片加密
 */
static error_t isp_prog_encrypt_chip(void)
{ 
    error_t ret = ERROR_SUCCESS ;  
    uint32_t  cfg_word0;
    uint8_t info1_data[CHIP_INFO1_SIZE] = {0};    
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //info1 读取    
    ret = read_save_info1(info1_data);
    if(ERROR_SUCCESS != ret)
        return ret;
    //加密前确认芯片option中调试位是否使能？若使能提示不能加密       
    if(isp_read_config(CHIP_CFG_WORD0_ADDR, &cfg_word0, 1) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    if( cfg_word0 & CHIP_CFG_DEBUG_Msk)
        return ERROR_ISP_ENCRYPT;
        
    if(isp_program_config(isp_target_dev->encrypt_addr, (uint32_t *)&isp_target_dev->encrypt_value, 1, NULL) != TRUE)
    {
        ret = ERROR_ISP_ENCRYPT;
        goto __fail;    
    }
    ret = judge_write_info1(info1_data);
    return  ret;     
__fail:
    judge_write_info1(info1_data);
    return  ret;   
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
    uint8_t read_buf[ISP_PRG_MINI_SIZE] = {0x00};
    uint8_t info1_data[CHIP_INFO1_SIZE] = {0};   
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //info1 读取    
    ret = read_save_info1(info1_data);
    if(ERROR_SUCCESS != ret)
        return ret;       
    cfg_word_addr =  isp_target_dev->config_word_start;
	cfg_word_size =  isp_target_dev->config_word_size;
    read_addr =  0;

    while(true)
    {
        copy_size = MIN(cfg_word_size, sizeof(read_buf) );
        
        ret = isp_prog_intf.cb(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            goto __fail;     
        ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
        if(ERROR_SUCCESS != ret)
            goto __fail;             
        // Update variables
        cfg_word_addr  += copy_size;
        cfg_word_size  -= copy_size;
        read_addr += copy_size;
        // Check for end
        if (cfg_word_size <= 0) 
            break;       
    }
    
    ret = judge_write_info1(info1_data);
    return ret; 
__fail:
    judge_write_info1(info1_data);
    return  ret;    
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
    uint8_t read_buf[ISP_PRG_MINI_SIZE];
    uint8_t info1_data[CHIP_INFO1_SIZE] = {0};   
       
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //info1 读取    
    ret = read_save_info1(info1_data);
    if(ERROR_SUCCESS != ret)
        return ret;   
        
    code_addr =  isp_target_dev->code_start;
    code_size =  isp_target_dev->code_size;
    read_addr =  0; 
        
    while(true)
    {
        copy_size = MIN(code_size, sizeof(read_buf) ); 
        ret = isp_prog_intf.cb(USER_HEX, read_addr, read_buf , copy_size);
        
        if(ERROR_SUCCESS != ret)
            goto __fail; 
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
                goto __fail; 
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
        ret = isp_prog_intf.cb(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            goto __fail;      		
        ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
        if( ret !=  ERROR_SUCCESS)
            goto __fail;             
        // Update variables
        cfg_word_addr  += copy_size;
        cfg_word_size  -= copy_size;
        read_addr += copy_size;
        // Check for end
        if (cfg_word_size <= 0) 
            break;       
    }
    ret = judge_write_info1(info1_data);
    return ret; 
__fail:
    judge_write_info1(info1_data);
    return  ret;     
}

/*******************************************************************************
*	函 数 名: isp_target_verify_all
*	功能说明: 芯片校验
*	形    参: sn_enable：是否已编程序列号 sn：序列号代码 
*             failed_addr：错误地址   
*	返 回 值: 错误类型
*******************************************************************************/
static error_t  isp_target_verify_all( uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data)
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
    uint8_t sf_buf[ISP_PRG_MINI_SIZE];  

    uint8_t info1_data[CHIP_INFO1_SIZE] = {0};    
  
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //info1 读取    
    ret = read_save_info1(info1_data);
    if(ERROR_SUCCESS != ret)
        return ret;
        
    code_addr =  isp_target_dev->code_start;
    code_size =  isp_target_dev->code_size;
    sf_addr = 0;
    while(true)
    {
        verify_size = MIN(code_size, sizeof(sf_buf) );
        ret = isp_prog_intf.cb(USER_HEX, sf_addr, sf_buf , verify_size);   
        if( ret !=  ERROR_SUCCESS)
            goto __fail; 
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        if( sn_enable == ENABLE)
        {              
            serial_number_intercept_write(sn, code_addr, sf_buf, verify_size);	//填入序列号 
        }                       
        ret = isp_prog_verify_flash(code_addr, sf_buf, verify_size,failed_addr,failed_data);                        
        if( ret !=  ERROR_SUCCESS)
            goto __fail; 
        // Update variables
        code_addr  += verify_size;
        code_size  -= verify_size;
        sf_addr += verify_size;
        // Check for end
        if (code_size <= 0) 
            break;       
    }  
    isp_prog_intf.cb(HEX_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if((sf_checksum&0x0000ffff) != (checksum&0x0000ffff))
    {
        ret = ERROR_USER_HEX_CHECKSUM;
        goto __fail; 
    }         
    
    cfg_word_addr =  isp_target_dev->config_word_start;
    cfg_word_size =  isp_target_dev->config_word_size;
    sf_addr =  0;
    checksum = 0;
    while(true)
    {
        verify_size = MIN(cfg_word_size, sizeof(sf_buf) );          
        ret = isp_prog_intf.cb(CFG_WORD, sf_addr, sf_buf , verify_size);
        if( ret !=  ERROR_SUCCESS)
            goto __fail;  
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        
        ret = isp_prog_verify_config(cfg_word_addr, sf_buf, verify_size,failed_addr,failed_data); 
        if( ret !=  ERROR_SUCCESS)
            goto __fail; 
        // Update variables
        cfg_word_addr  += verify_size;
        cfg_word_size  -= verify_size;
        sf_addr += verify_size;
        // Check for end
        if (cfg_word_size <= 0) 
            break;       
    }
    isp_prog_intf.cb(CFG_WORD_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if(sf_checksum != (checksum&0x0000ffff))
    {
        ret = ERROR_CFG_WORD_CHECKSUM;
        goto __fail;  
    } 
    ret = judge_write_info1(info1_data);
    return  ret;     
__fail:
    judge_write_info1(info1_data);
    return  ret; 
}


// 保存info1  放在各操作之前 
static error_t read_save_info1(uint8_t *data)
{
    error_t ret;
    uint32_t i = 0;	
    for(i=0;i<3;i++)
    {
        if(isp_read_config(CHIP_INFO1_ADDR, (uint32_t*)data, CHIP_INFO1_SIZE/4) == TRUE)
        {
            ret = isp_prog_verify_config(CHIP_INFO1_ADDR,data,CHIP_INFO1_SIZE, NULL, NULL); 
            if(ERROR_SUCCESS == ret)
            {
                if( (data[251] == (isp_target_dev->chipid_value & 0xFF000000) >> 24)&&
                    (data[250] == (isp_target_dev->chipid_value & 0x00FF0000) >> 16)&&
                    (data[249] == (isp_target_dev->chipid_value & 0x0000FF00) >> 8)&&
                    (data[248] ==(isp_target_dev->chipid_value & 0x000000FF))   ) 
                break; 
            }          
        }
    }
    if(i>=3) 
        return ERROR_ISP_CFG_WORD_VERIFY;
    return ret;
}

// 判chipID 若不对，写回info1  在各个操作结束后执行,防止误差chipid
static error_t judge_write_info1(uint8_t *data)
{
    error_t ret;
    uint32_t i = 0;

    ret = isp_chipid_check();
    
    if(ERROR_SUCCESS == ret)
        return ERROR_SUCCESS;

    for(i=0;i<10;i++)
    {
        es_delay_ms(90);
        //ret = isp_prog_init() ;
        if(isp_erase_info1() != TRUE)
            continue;
        ret = isp_prog_program_config(CHIP_INFO1_ADDR, data, CHIP_INFO1_SIZE, NULL);
        if(ERROR_SUCCESS != ret) 
            continue;
        ret = isp_prog_verify_config(CHIP_INFO1_ADDR, data, CHIP_INFO1_SIZE, NULL, NULL);
        if(ERROR_SUCCESS == ret) 
            break;
    }
    if(i>=10) 
        return ret;
    else
        return ERROR_SUCCESS;       
}










