//M939 info偏移地址为0x40000
#include "eslink.h"
#include "errno.h"
#include "sflash_port.h"  
#include "program_port.h"
#include "uartboot_prog_intf.h"
#include "uartboot_host.h"
#include "uartboot_target_config.h"
#include "target_config.h"

//一次编程支持的长度，根据RAM大小可以修改.长度需要为2^n
#define BOOTISP_PRG_SIZE  1024  

static void uartboot_init(es_target_cfg *target);
static error_t uartboot_prog_init(void); //进模式
static error_t uartboot_prog_uninit(void); //退出模式

static error_t uartboot_prog_erase_chip (uint8_t para);
static error_t uartboot_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t uartboot_prog_read_chipid(uint8_t *buf);
static error_t uartboot_prog_read_checksum(uint8_t *buf);
static error_t uartboot_prog_encrypt_chip(void);
static error_t uartboot_chipid_check(void);
static error_t uartboot_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_addr );
static error_t uartboot_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t uartboot_prog_verify_config(uint32_t addr,  uint8_t *buf, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t uartboot_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t uartboot_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t uartboot_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t uartboot_target_program_config_all(uint32_t *failed_addr);
static error_t uartboot_target_program_all(  uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr);
static error_t uartboot_target_verify_all( uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data);

struct  es_prog_ops uartboot_prog_intf = {
    uartboot_init,
    uartboot_prog_init,
    uartboot_prog_uninit,
    uartboot_prog_erase_chip,
    uartboot_prog_check_empty,
    uartboot_prog_read_chipid,
    uartboot_chipid_check,
    uartboot_prog_read_checksum,
    uartboot_prog_encrypt_chip,

    uartboot_prog_program_config,
    uartboot_prog_read_config,
    uartboot_prog_verify_config,
    uartboot_prog_program_flash,
    uartboot_prog_read_flash,
    uartboot_prog_verify_flash,
    uartboot_target_program_config_all ,
    uartboot_target_program_all,
    uartboot_target_verify_all,
    0
};  
static const es_target_cfg *target_dev;  

static void uartboot_init(es_target_cfg *target)
{         
    target_dev = target;
    uartboot_prog_intf.cb = online_file_read;
    uart_initialize();
//    if(uartboot_start() != 0)
     
}
static error_t uartboot_prog_init(void) //进模式
{
    if(uartboot_start() != TRUE)
        return ERROR_UARTBOOT_START;
    return ERROR_SUCCESS; 
}
static error_t uartboot_set_target_reset(void)  
{
  uint32_t addr;
  uint32_t data;
  
  addr = 0x40080000;
  data = 0x55AA6996;
    if(uartboot_write_memory(addr, (uint8_t*)&data, 4) != TRUE)
    return  ERROR_UARTBOOT_WRITE;
  
  addr = 0x40080824;
  data = 0x00000001;
    if(uartboot_write_memory(addr, (uint8_t*)&data, 4) != TRUE)
    return  ERROR_UARTBOOT_WRITE;
  return ERROR_SUCCESS; 
}

static error_t uartboot_prog_uninit(void) //退出模式
{
    return ERROR_SUCCESS; 
}

//擦除
#define FULL_ERASE_CMD      0xFFFF
#define FULL_ERASE_CMD_H    0xFF
#define FULL_ERASE_CMD_L    0xFF
/*******************************************************************************
*  函 数 名: uartboot_prog_erase_chip
*  功能说明: 擦除.芯片加密后
*  形    参:  
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_erase_chip (uint8_t para)
{
    uint8_t data[4]; 
  error_t ret;
  
  ret = uartboot_chipid_check();         
    if( (ERROR_SUCCESS != ret) && (ERROR_LV2_ENCRYPT != ret))
        return ret;  
        
    //flash 擦除 
    data[0] = FULL_ERASE_CMD_H;
    data[1] = FULL_ERASE_CMD_L;        
    if(uartboot_extended_erase(data , 2) != TRUE)
    {
        return ERROR_UARTBOOT_ERASE;   
    }            
    //info页擦（除RDP页）
    data[0] = 0x00;
    data[1] = 0x00;     //页数
    data[2] = (CHIP_INFO1_INDEX & 0xff00) >> 8;
    data[3] = CHIP_INFO1_INDEX & 0x00ff;
    if(uartboot_extended_erase( data, 4) != TRUE)
    {
        return ERROR_UARTBOOT_ERASE; 
    }
    
    return ERROR_SUCCESS; 
}
/*******************************************************************************
*  函 数 名: uartboot_prog_check_empty
*  功能说明: 查空
*  形    参: failed_addr：错误地址  failedData:错误hsuju
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_check_empty(uint32_t *failed_addr, uint32_t *failed_data) 
{    
    error_t ret;
    
    ret = uartboot_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    if(uartboot_check_empty(target_dev->code_start , target_dev->code_size) != TRUE)
  {
      if(failed_addr)
            *failed_addr = 0xFFFFFFFF ; 
    if(failed_data)
            *failed_data = 0xFFFFFFFF ; 
    return ERROR_UARTBOOT_CHECK_EMPTY;
  }
        
  if(uartboot_check_empty(CHIP_INFO1_ADDR , CHIP_INFO1_SIZE) != TRUE)
  {
    if(failed_addr)
            *failed_addr = 0xFFFFFFFF ; 
    if(failed_data)
            *failed_data = 0xFFFFFFFF ; 
    return ERROR_UARTBOOT_CHECK_EMPTY;  
  }         
    return ERROR_SUCCESS; 
}
/*******************************************************************************
*  函 数 名: uartboot_prog_read_chipid
*  功能说明: 读chipid
*  形    参:  
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_prog_read_chipid(uint8_t *buf)
{
    if(uartboot_read_memory(CHIP_INFO_FLASH_OFFSET + target_dev->chipid_addr, buf, 4) != TRUE)
        return ERROR_UARTBOOT_READ;
    return ERROR_SUCCESS; 
}
/*******************************************************************************
*  函 数 名: uartboot_chipid_check
*  功能说明: id检测
*  形    参:  
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_chipid_check(void)
{
    uint32_t chipid = 0;
    uint32_t reg_data ;
    
    if (uartboot_read_memory(CHIP_INFO_FLASH_OFFSET + target_dev->chipid_addr, (uint8_t*)&chipid, 4) != TRUE)  
    {
        return ERROR_UARTBOOT_READ;
    }
    if(chipid != target_dev->chipid_value)    
    {
        if(uartboot_read_memory(CHIP_INFO_FLASH_OFFSET+ CHIP_CFG_GBRDP_ADDR, (uint8_t*)&reg_data, 4) != TRUE)
             return ERROR_ISP_READ_CFG_WORD;
        if((chipid == 0x00000000) && ( reg_data == 0x00000000))
             return  ERROR_LV2_ENCRYPT;
        return  ERROR_CHIP_ID_NOT_MATCH;
    }
        
    return ERROR_SUCCESS; 
}
/*******************************************************************************
*  函 数 名:  uartboot_prog_read_checksum
*  功能说明:  读校验和
*  形    参:  
*  返 回 值:  错误类型
*******************************************************************************/
static error_t uartboot_prog_read_checksum(uint8_t *buf)
{
  error_t ret;
  
  ret = uartboot_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret;  
    
    if(uartboot_read_memory(CHIP_INFO_FLASH_OFFSET + CHIP_CHECKSUM_ADDR, buf, 4) != TRUE)
        return ERROR_UARTBOOT_READ;
    return ERROR_SUCCESS; 
}
/*******************************************************************************
*  函 数 名:  uartboot_prog_encrypt_chip
*  功能说明:  加密
*  形    参:  
*  返 回 值:  错误类型
*******************************************************************************/
static error_t uartboot_prog_encrypt_chip(void)
{
    error_t ret = ERROR_SUCCESS;
    uint8_t status;
  
    ret = uartboot_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 

    status = uartboot_write_memory(target_dev->encrypt_addr, (uint8_t *)&target_dev->encrypt_value, 4);
    if(status != TRUE)
        return ERROR_UARTBOOT_ENCRYPT;  
       
    return ERROR_SUCCESS;   
}
/*******************************************************************************
*  函 数 名: uartboot_prog_program_config
*  功能说明: 配置字编程
*  形    参:  
*  返 回 值: 
*******************************************************************************/
static error_t uartboot_prog_program_config(uint32_t addr, uint8_t *buf, uint32_t size,uint32_t *failed_addr )
{
    uint8_t ret = ERROR_SUCCESS ;    
    uint32_t prog_addr;
    uint32_t prog_size;       

    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
            
    prog_addr = CHIP_INFO_FLASH_OFFSET + CHIP_INFO_PART1_ADDR;     
    prog_size = CHIP_INFO_PART1_SIZE ;     
    ret = uartboot_write_memory(prog_addr, buf, prog_size );
    if(ret != TRUE)
    {
        if(failed_addr)
            *failed_addr = 0xFFFFFFFF ; 
         return ERROR_UARTBOOT_PROG_CFG_WORD;
    } 
    
    buf += prog_size; 
    prog_addr = CHIP_INFO_FLASH_OFFSET + CHIP_INFO_PART2_ADDR;     
    prog_size = CHIP_INFO_PART2_SIZE ;     
    
    ret = uartboot_write_memory(prog_addr, buf, prog_size);
    if(ret != TRUE)
    {
        if(failed_addr)
            *failed_addr = 0xFFFFFFFF; 
        return ERROR_UARTBOOT_PROG_CFG_WORD;
    } 
    
    return ERROR_SUCCESS;      
}

static error_t uartboot_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size)
{
    error_t ret;
    uint32_t read_addr;
    uint32_t read_size;
    
  ret = uartboot_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;

    read_addr = CHIP_INFO_FLASH_OFFSET + CHIP_INFO_PART1_ADDR; 
    read_size = CHIP_INFO_PART1_SIZE ;
    if (uartboot_read_memory(read_addr, buf, read_size) != TRUE) 
        return ERROR_UARTBOOT_READ; 
        
    buf += read_size;    
    read_addr = CHIP_INFO_FLASH_OFFSET + CHIP_INFO_PART2_ADDR;
    read_size = CHIP_INFO_PART2_SIZE ;     
    
    if (uartboot_read_memory(read_addr, buf, read_size) != TRUE) 
        return ERROR_UARTBOOT_READ;     
  
    return ERROR_SUCCESS;
}

static error_t uartboot_prog_verify_config(uint32_t addr,  uint8_t *buf, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
    uint32_t i;
    uint8_t read_buf[BOOTISP_PRG_SIZE];
    uint32_t verify_size;
    
    uint32_t read_addr;
    uint32_t read_size;
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    
    read_addr = CHIP_INFO_FLASH_OFFSET + CHIP_INFO_PART1_ADDR; 
    read_size = CHIP_INFO_PART1_SIZE   ;
    while (read_size > 0) 
    {          
        verify_size = MIN(read_size, sizeof(read_buf));
    if (uartboot_read_memory(read_addr, read_buf, verify_size) != TRUE) 
      return ERROR_UARTBOOT_READ; 
        for(i=0; i< verify_size; i++)
        {
            if( *buf++ != read_buf[i] ) 
            {
                *failed_addr = addr + ROUND_DOWN(i, 4)  ;
                *failed_data |= (read_buf[*failed_addr] << 0) ; 
                *failed_data |= (read_buf[*failed_addr+1] << 8) ; 
                *failed_data |= (read_buf[*failed_addr+2] << 16) ; 
                *failed_data |= (read_buf[*failed_addr+3] << 24) ; 
        return ERROR_UARTBOOT_VERIFY;  
            }  
        } 
        read_addr += verify_size;
        read_size -= verify_size;
    }
    
    read_addr = CHIP_INFO_FLASH_OFFSET + CHIP_INFO_PART2_ADDR;
    read_size = CHIP_INFO_PART2_SIZE ;     
    while (read_size > 0) 
    {          
        verify_size = MIN(read_size, sizeof(read_buf));
    if (uartboot_read_memory(read_addr, read_buf, verify_size) != TRUE) 
      return ERROR_UARTBOOT_READ; 
        for(i=0; i< verify_size; i++)
        {
            if( *buf++ != read_buf[i] ) 
            {
                *failed_addr = addr + ROUND_DOWN(i, 4)  ;
                *failed_data |= (read_buf[*failed_addr] << 0) ; 
                *failed_data |= (read_buf[*failed_addr+1] << 8) ; 
                *failed_data |= (read_buf[*failed_addr+2] << 16) ; 
                *failed_data |= (read_buf[*failed_addr+3] << 24) ; 
        return ERROR_UARTBOOT_VERIFY;  
            }  
        } 
        read_addr += verify_size;
        read_size -= verify_size;
    }
    return ERROR_SUCCESS;  
}
static error_t uartboot_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr)
{    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    
    if(uartboot_write_memory( addr, data, size)!= TRUE)
    {
        if(failed_addr)
            *failed_addr = 0xFFFFFFFF ; 
        return ERROR_UARTBOOT_WRITE;
    }
    return ERROR_SUCCESS; 
}
static error_t uartboot_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size)
{
    error_t ret;
    
    ret = uartboot_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    if(uartboot_read_memory(addr, data, size) != TRUE)
        return ERROR_UARTBOOT_READ;
    return ERROR_SUCCESS; 
}
static error_t uartboot_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
    uint32_t i;
    uint8_t read_buf[BOOTISP_PRG_SIZE];
    uint32_t verify_size; 
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    
    while (size > 0) 
    {          
        verify_size = MIN(size, sizeof(read_buf));
        if (uartboot_read_memory(addr, read_buf, verify_size) != TRUE) 
      return ERROR_UARTBOOT_READ; 
        for(i=0; i< verify_size; i++)
        {
            if( data[i] != read_buf[i] ) 
            {
                *failed_addr = addr + ROUND_DOWN(i, 4)  ;
                *failed_data |= (read_buf[*failed_addr] << 0) ; 
                *failed_data |= (read_buf[*failed_addr+1] << 8) ; 
                *failed_data |= (read_buf[*failed_addr+2] << 16) ; 
                *failed_data |= (read_buf[*failed_addr+3] << 24) ; 
        return ERROR_SWD_VERIFY;              
            }
        } 
        addr += verify_size;
        size -= verify_size;
    }      
    return ERROR_SUCCESS;  
}
/*******************************************************************************
*  函 数 名:  uartboot_target_program_config_all
*  功能说明:  配置字编程
*  形    参:  failed_addr  ： 编程失败地址
*  返 回 值: 
*******************************************************************************/
static error_t uartboot_target_program_config_all(uint32_t *failed_addr)
{
    error_t ret = ERROR_SUCCESS;
        
    uint32_t cfg_word_addr;  
  uint32_t cfg_word_size;   
    
    uint32_t copy_size;
    uint32_t read_addr;
    uint8_t read_buf[BOOTISP_PRG_SIZE] = {0x00};
    
    ret = uartboot_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    cfg_word_addr =  target_dev->config_word_start;
  cfg_word_size =  target_dev->config_word_size;
    read_addr =  0;

    while(true)
    {
        copy_size = MIN(cfg_word_size, sizeof(read_buf) );
        
        ret = uartboot_prog_intf.cb(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;     
        ret = uartboot_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
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
*  函 数 名: uartboot_target_program_all
*  功能说明: 编程  编程256k时间需要3分钟
*  形    参:  
*  返 回 值: 
*******************************************************************************/
static error_t uartboot_target_program_all(  uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr)
{
    error_t ret = ERROR_SUCCESS;
    uint32_t i;
    
    uint32_t code_addr;  
  uint32_t code_size;  
    uint32_t cfg_word_addr;  
  uint32_t cfg_word_size;     
    
    uint32_t copy_size;      
    uint32_t read_addr;
    uint8_t read_buf[BOOTISP_PRG_SIZE];
       
    ret = uartboot_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  target_dev->code_start;
  code_size =  target_dev->code_size;
    read_addr =  0; 
        
    while(true)
    {
        copy_size = MIN(code_size, sizeof(read_buf) ); 
        ret = uartboot_prog_intf.cb(USER_HEX, read_addr, read_buf , copy_size);
        
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
            ret = uartboot_prog_program_flash(code_addr, read_buf, copy_size, failed_addr); 
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
    cfg_word_addr =  target_dev->config_word_start;
  cfg_word_size =  target_dev->config_word_size;
    read_addr =  0;
    while(true)
    {
        copy_size = MIN(cfg_word_size, sizeof(read_buf) );          
        ret = uartboot_prog_intf.cb(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;         
        ret = uartboot_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
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
*  函 数 名: uartboot_target_verify_all
*  功能说明: 芯片数据校验
*  形    参: sn_enable：序列号是否使能  sn：序列号  failed_addr：错误地址 
*             failed_data：错误数据
*  返 回 值: 错误类型
*******************************************************************************/
static error_t uartboot_target_verify_all( uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data)
{
    error_t ret = ERROR_SUCCESS;

    uint32_t checksum = 0;  
    uint32_t sf_checksum = 0;   //spi保存的校验和         
    uint32_t code_addr;  
  uint32_t code_size;  
    uint32_t cfg_word_addr;  
  uint32_t cfg_word_size;       
    uint32_t verify_size; 
    uint32_t sf_addr;  
    uint8_t sf_buf[BOOTISP_PRG_SIZE];     
  
    ret = uartboot_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  target_dev->code_start;
  code_size =  target_dev->code_size;
    sf_addr = 0;
    while(true)
    {
        verify_size = MIN(code_size, sizeof(sf_buf) );
       
        ret = uartboot_prog_intf.cb(USER_HEX, sf_addr, sf_buf , verify_size);
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        if( sn_enable == ENABLE)
            serial_number_intercept_write(sn, code_addr, sf_buf, verify_size);  //填入序列号         
        ret = uartboot_prog_verify_flash(code_addr, sf_buf, verify_size,failed_addr,failed_data);                        
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
    uartboot_prog_intf.cb(HEX_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if((sf_checksum&0x0000ffff) != (checksum&0x0000ffff))
    {
        ret = ERROR_USER_HEX_CHECKSUM;
        return  ret;
    }         
    
    cfg_word_addr =  target_dev->config_word_start;
  cfg_word_size =  target_dev->config_word_size;
    sf_addr =  0;
    checksum = 0;
    while(true)
    {
        verify_size = MIN(cfg_word_size, sizeof(sf_buf) );          
        ret = uartboot_prog_intf.cb(CFG_WORD, sf_addr, sf_buf , verify_size);
        if( ret !=  ERROR_SUCCESS)
            return ret; 
        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
        
        ret = uartboot_prog_verify_config(cfg_word_addr, sf_buf, verify_size,failed_addr,failed_data); 
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
    uartboot_prog_intf.cb(CFG_WORD_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
    if(sf_checksum != (checksum&0x0000ffff))
    {
        ret = ERROR_CFG_WORD_CHECKSUM;
        return  ret; 
    }           
    return  ret;  
}


