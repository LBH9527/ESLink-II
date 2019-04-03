#include "eslink.h"
#include "errno.h"
#include "isp_prog_intf.h" 
#include "sflash_port.h"  
#include "program_port.h"
#include "./es_isp.h"
#include "./target_info.h"

static void isp_init(es_target_cfg *target);
static error_t isp_prog_init(void); //进模式
static error_t isp_prog_uninit(void); //退出模式

static error_t isp_prog_erase_chip (uint8_t para);
static error_t isp_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t isp_prog_read_chipid(uint8_t *buf);
static error_t isp_chipid_check(void);
static error_t isp_prog_read_chip_chksum(uint8_t *buf);
static error_t isp_prog_encrypt_chip(void);

static error_t isp_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_addr );
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t isp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t isp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t isp_target_program_config_all(uint32_t *failed_addr);
static error_t isp_target_program_all(  uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr);
static error_t isp_target_verify_all( uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data);


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
    isp_reset();
    if(isp_id_check() != TRUE)
        return ERROR_IN_ISP_MODE;
    if(isp_unlock_check() != TRUE)
        return ERROR_ISP_UNLOCK;
    if( isp_mode_set() != TRUE)
        return ERROR_IN_ISP_MODE;
    if( is_encrypt_check() != TRUE)
        return ERROR_IN_ISP_MODE;
    
    return ERROR_SUCCESS; 
} 

//退出isp模式
static error_t isp_out_mode(void)
{      
    isp_reset(); 
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
*函数名：
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
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    if(isp_read_config(CHIP_CHECKSUM_ADDR, (uint32_t*)buf, 1) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
        
    return ERROR_SUCCESS;     
}

/*
 * 擦除
 */
static error_t isp_prog_erase_chip (uint8_t para) 
{
    error_t ret = ERROR_SUCCESS;
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    if(isp_erase_chip() != TRUE)
     return  ERROR_ISP_ERASE;
         
    return ret;
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
    const struct info_part_map *part;
    uint32_t item_num;
      
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  isp_target_dev->code_start;
    code_size =  isp_target_dev->code_size / 4; //字长度
    while(true)
    {
      copy_size = MIN(code_size, sizeof(read_buf)/sizeof(read_buf[0]) );
      isp_read_code(code_addr, read_buf, copy_size);
      for(i = 0; i<copy_size; i++)
      {
        if(read_buf[i] != 0xFFFFFFFF)
        {    
          if(failed_addr)
              *failed_addr = code_addr + i*4  ;
          if( failed_data)
              *failed_data = read_buf[i] ; 
          return ERROR_ISP_FLASH_CHECK_EMPTY;
        }         
      } 
          // Update variables
      code_addr  += copy_size*4;
      code_size  -= copy_size;
        
        // Check for end
        if (code_size <= 0) {
            break;
        } 
  }  
    //配置字查空      
    for(item_num=0; item_num<ITEM_NUM(info_part_map); item_num++) 
    {
        part = &info_part_map[item_num]; 
        cfg_word_addr = part->addr;     
        cfg_word_size = part->size/4;   //字长度
        while(true)
        {
            copy_size = MIN(cfg_word_size, sizeof(read_buf)/sizeof(read_buf[0]));
            isp_read_config(cfg_word_addr, read_buf, copy_size);
            for(i = 0; i<copy_size; i++)
            {
                if(read_buf[i] != 0xFFFFFFFF)
                {     
                    if(failed_addr)
                        *failed_addr = cfg_word_addr + i*4  ;
                    if(failed_data)
                        *failed_data = read_buf[i] ; 
                    return ERROR_ISP_CFG_WORD_CHECK_EMPTY;
                }         
            } 
            // Update variables
            cfg_word_addr  += copy_size * 4;
            cfg_word_size  -= copy_size ;
            
            // Check for end
            if (code_size <= 0) {
                break;
            } 
        }     
    }

    return ERROR_SUCCESS;      
} 
/*******************************************************************************
*函数名：isp_prog_program_flash
* 描述 ：flash编程    
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
static error_t isp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr)
{
    uint8_t ret ; 
    uint32_t size_in_words; 
    uint32_t offset;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;
    
    ret = isp_program_code( addr, (uint32_t*)data, size_in_words,&offset);
    if(ret != TRUE)
    {
        if( failed_addr)
            *failed_addr =  addr + offset * 4;
        return ERROR_ISP_PROG;
    }
   
    return ERROR_SUCCESS;     
}
/*******************************************************************************
*函数名：isp_prog_read_flash
* 描述 ：读flash  
* 输入 ：addr：地址。data：数据。size：大小
* 输出 ：
*******************************************************************************/
static error_t isp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t size_in_words; 
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    size_in_words = size/4;    
    if(isp_read_code(addr, (uint32_t*)data, size_in_words) != TRUE)
        return ERROR_ISP_READ;
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
        verify_size = MIN(size_in_words, sizeof(rd_buf)/sizeof(rd_buf[0]));
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
        addr += verify_size*4;
        size_in_words -= verify_size;
    }
    
    return ERROR_SUCCESS;      
}
/*
 *配置字编程
 * ERROR_ISP_PROG_CONFIG
 */
static error_t isp_prog_program_config(uint32_t addr, uint8_t *buf, uint32_t size, uint32_t *failed_addr )
{   
    uint8_t ret ;  
    uint32_t offset;    
    const struct info_part_map *part;
    uint32_t i;  
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
 
    for(i=0; i<ITEM_NUM(info_part_map); i++)
    {
        part = &info_part_map[i];     
    
        ret = isp_program_config(part->addr, (uint32_t*)buf, part->size, &offset);
        if(ret != TRUE)
        {
            if(failed_addr)
                *failed_addr = part->addr + offset*4 ; 
             return ERROR_ISP_PROG_CFG_WORD;
        } 
        buf += (part->size) * 4;    
    }    
    return ERROR_SUCCESS;        
}

/*
 *读配置字
 * return :   ERROR_SUCCESS   
 */
static error_t isp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size)
{
    error_t ret = ERROR_SUCCESS;
    const struct info_part_map *part;        
    uint32_t i;  
    
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 

    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;        
 
    for(i=0; i<ITEM_NUM(info_part_map); i++)
    {
        part = &info_part_map[i];     
    
        if( isp_read_config(part->addr, (uint32_t*)buf, part->size) != TRUE)
            return ERROR_ISP_READ_CFG_WORD;  
        buf += (part->size) * 4;    
    }   
    
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
    uint32_t read_addr;
    uint32_t read_size;
    const struct info_part_map *part;        
    uint32_t item_num;
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;  

    for(item_num=0; item_num<ITEM_NUM(info_part_map); item_num++)    
    {
        part = &info_part_map[item_num]; 
        read_addr = part->addr;     
        read_size = part->size/4; 
        while (read_size > 0) 
        {          
            verify_size = MIN(read_size, sizeof(rd_buf)/sizeof(rd_buf[0]));
            ret = isp_read_config(read_addr, rd_buf, verify_size); 
            if( ret != TRUE)
                return ERROR_ISP_READ_CFG_WORD;
            for(i=0; i< verify_size; i++)
            {
                if( (buf[i*4] != ((rd_buf[i]>>0)&0xFF))  ||                        
                    (buf[i*4+1] != ((rd_buf[i]>>8)&0xFF))  ||
                    (buf[i*4+2] != ((rd_buf[i]>>16)&0xFF)) ||
                    (buf[i*4+3] != ((rd_buf[i]>>24)&0xFF)) )
                {
                    if(failed_addr)
                        *failed_addr = read_addr + i*4 ;  
                    if(failed_data)
                        *failed_data = rd_buf[i];
                    return  ERROR_ISP_CFG_WORD_VERIFY;  
                } 
            } 
            read_addr += verify_size*4;
            read_size -= verify_size;
        }  
        buf += part->size ;
    }
    return ERROR_SUCCESS;
}


/*
 * 芯片加密
 */
static error_t isp_prog_encrypt_chip(void)
{ 
    error_t ret = ERROR_SUCCESS;
    uint32_t cfg_word0;
    ret = isp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
    //加密前确认芯片option中调试位是否使能？若使能提示不能加密       
    if(isp_read_config(CHIP_CFG_WORD0_ADDR, &cfg_word0, 1) != TRUE)
        return ERROR_ISP_READ_CFG_WORD;
    if( cfg_word0 & CHIP_CFG_DEBUG_Msk)
        return ERROR_ISP_ENCRYPT;
        
    if(isp_program_config(isp_target_dev->encrypt_addr, (uint32_t *)&isp_target_dev->encrypt_value, 1, NULL) != TRUE)
        return ERROR_ISP_ENCRYPT;    
    return ERROR_SUCCESS;    
}

/*******************************************************************************
*  函 数 名: isp_target_program_config_all
*  功能说明: 芯片配置字编程。
*  形    参: failed_addr：错误地址  
*  返 回 值: 编程错误地址
*******************************************************************************/
static error_t isp_target_program_config_all(uint32_t *failed_addr)
{
    error_t ret = ERROR_SUCCESS;
    
    uint32_t cfg_word_addr;  
    uint32_t cfg_word_size;   
    
    uint32_t copy_size;
    uint32_t read_addr;
    uint8_t read_buf[ISP_PRG_MINI_SIZE] = {0x00};
    
    ret = isp_chipid_check();
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
*  函 数 名: isp_prog_program_flash
*  功能说明: 芯片编程。flash和配置字编程
*  形    参: sn_enable：是否已编程序列号 sn：序列号代码 
*             failed_addr：错误地址   failed_data ：错误数据
*  返 回 值: 错误类型
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
       
    ret = isp_chipid_check();
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
            return ret;
        if(sn_enable == ENABLE)     //序列号代码使能
            serial_number_intercept_write(sn ,code_addr, read_buf, copy_size);  //填入序列号
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
        ret = isp_prog_intf.cb(CFG_WORD, read_addr, read_buf , copy_size);
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
*  函 数 名: isp_target_verify_all
*  功能说明: 芯片验证
*  形    参: sn_enable：是否已编程序列号 sn：序列号代码 
*             failed_addr：错误地址   
*  返 回 值: 错误类型
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
  
    ret = isp_chipid_check();
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
            return ret; 
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        if( sn_enable == ENABLE)
        {              
            serial_number_intercept_write(sn, code_addr, sf_buf, verify_size);  //填入序列号 
        }                       
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
    isp_prog_intf.cb(HEX_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
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
        ret = isp_prog_intf.cb(CFG_WORD, sf_addr, sf_buf , verify_size);
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
    isp_prog_intf.cb(CFG_WORD_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if(sf_checksum != (checksum&0x0000ffff))
    {
        ret = ERROR_CFG_WORD_CHECKSUM;
        return  ret; 
    }           
    return  ret; 
}








