//M939 info偏移地址为0x40000
#include "string.h"
#include "stdbool.h"
#include "eslink.h"
#include "errno.h"
#include "sflash_port.h"
#include "es_isp.h"

#include "bootisp_prog_intf.h"
#include "bootisp_host.h"
#include "bootisp_target_config.h"

//一次编程支持的长度，根据RAM大小可以修改.长度需要为2^n
#define BOOTISP_PRG_SIZE  1024  

static void bootisp_init(es_target_cfg *target);
static error_t bootisp_prog_init(void); //进模式
static error_t bootisp_prog_uninit(void); //退出模式

static error_t bootisp_prog_erase_chip (uint8_t para);
static error_t bootisp_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t bootisp_prog_read_chipid(uint8_t *buf);
static error_t bootisp_prog_read_checksum(uint8_t *buf);
static error_t bootisp_prog_encrypt_chip(void);
static error_t bootisp_chipid_check(void);
static error_t bootisp_prog_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_addr );
static error_t bootisp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t bootisp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t bootisp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t bootisp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t bootisp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t bootisp_target_program_config_all(uint32_t *failed_addr);
static error_t bootisp_target_program_all(  uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr);
static error_t bootisp_target_verify_all( uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data);

struct  es_prog_ops bootisp_prog_intf = {
    bootisp_init,
    bootisp_prog_init,
    bootisp_prog_uninit,
    bootisp_prog_erase_chip,
    bootisp_prog_check_empty,
    bootisp_prog_read_chipid,
//    bootisp_chipid_check,
    bootisp_prog_read_checksum,
    bootisp_prog_encrypt_chip,

    bootisp_prog_program_config,
    bootisp_prog_read_config,
    bootisp_prog_verify_config,
    bootisp_prog_program_flash,
    bootisp_prog_read_flash,
    bootisp_prog_verify_flash,
    bootisp_target_program_config_all ,
    bootisp_target_program_all,
    bootisp_target_verify_all,
    0
};  
static const es_target_cfg *target_dev;  

static void bootisp_init(es_target_cfg *target)
{         
    target_dev = target;
    bootisp_prog_intf.cb = online_file_read;
    uart_initialize();
//    if(bootisp_start() != 0)
     
}
static error_t bootisp_prog_init(void) //进模式
{
    if(bootisp_start() != TRUE)
        return ERROR_BOOTISP_START;
    return ERROR_SUCCESS; 
}
static error_t bootisp_prog_uninit(void) //退出模式
{
    return ERROR_SUCCESS; 
}

//擦除
#define FULL_ERASE_CMD      0xFFFF
#define FULL_ERASE_CMD_H    0xFF
#define FULL_ERASE_CMD_L    0xFF
static error_t bootisp_prog_erase_chip (uint8_t para)
{
    error_t status = ERROR_SUCCESS;
    uint8_t data[4]; 

    //全擦
    data[0] = FULL_ERASE_CMD_H;
    data[1] = FULL_ERASE_CMD_L;        
    if(bootisp_extended_erase(data , 2) != TRUE)
    {
        return ERROR_BOOTISP_WRITE;   
    }            
//    //info页擦（除RDP页）
//    data[0] = 0x00;
//    data[1] = 0x00;     //页数
//    data[2] = 0x01;
//    data[3] = 0x01;
//    if(bootisp_extended_erase( data, 4) != 0)
//    {
//        return ERROR_BOOTISP_WRITE; 
//    }
    //复位
//        bootisp_set_target_reset();
    //同步
//    bootisp_start();
        

    
    return ERROR_SUCCESS; 
}
static error_t bootisp_prog_check_empty(uint32_t *failed_addr, uint32_t *failedData) 
{    
    if(bootisp_check_empty(target_dev->code_start , target_dev->code_size) != TRUE)
        return ERROR_BOOTISP_CHECK_EMPTY;
    return ERROR_SUCCESS; 
}
static error_t bootisp_prog_read_chipid(uint8_t *buf)
{
    if(bootisp_read_memory(CHIP_INFO_OFFSET + target_dev->chipid_addr, buf, 4) != TRUE)
        return ERROR_BOOTISP_READ;
    return ERROR_SUCCESS; 
}
static error_t bootisp_prog_read_checksum(uint8_t *buf)
{
    if(bootisp_read_memory(CHIP_INFO_OFFSET + CHIP_CHECKSUM_ADDR, buf, 4) != TRUE)
        return ERROR_BOOTISP_READ;
    return ERROR_SUCCESS; 
}
static error_t bootisp_prog_encrypt_chip(void)
{
       return ERROR_SUCCESS; 
}
static error_t bootisp_chipid_check(void)
{
      return ERROR_SUCCESS; 
}
static error_t bootisp_prog_program_config(uint32_t addr, uint8_t *buf, uint32_t size,uint32_t *failed_addr )
{
    uint8_t ret = ERROR_SUCCESS ;    
    uint32_t prog_addr;
    uint32_t prog_size;       

    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
            
    prog_addr  =  CHIP_INFO_OFFSET + 0x400;     //info1的偏移地址
    prog_size = 56;     
    ret = bootisp_write_memory(prog_addr, buf, prog_size );
    if(ret != TRUE)
    {
        if(failed_addr)
            *failed_addr = 0xFFFFFFFF ; 
         return ERROR_BOOTISP_PROG_CFG_WORD;
    } 
    
    buf += prog_size; 
    prog_addr  =  addr + 0x7C0;     //info1的偏移地址
    prog_size = 96;     
    
    ret = bootisp_write_memory(prog_addr, buf, prog_size);
    if(ret != TRUE)
    {
        if(failed_addr)
            *failed_addr = 0xFFFFFFFF; 
        return ERROR_BOOTISP_PROG_CFG_WORD;
    } 
     return ERROR_SUCCESS;      
}
static error_t bootisp_prog_read_config(uint32_t addr,  uint8_t *buf, uint32_t size)
{
    uint8_t ret;
    uint32_t read_addr;
    uint32_t read_size;
    
//    ret = swd_chipid_check();
//    if(ERROR_SUCCESS != ret)
//        return ret; 
        
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;

    //配置字在inf1区，保留未用的数据未下发。此信息需要根据XML文件修改。
    read_addr  =  CHIP_INFO_OFFSET + 0x400;     //info1的偏移地址
    read_size = 56;     //14个字
    if (bootisp_read_memory(read_addr, buf, read_size) != TRUE) 
        return ERROR_SWD_READ; 
        
    buf += read_size;    
    read_addr  =  CHIP_INFO_OFFSET + 0x7C0;     //info1的偏移地址
    read_size = 96;     //24个字
    
    if (bootisp_read_memory(read_addr, buf, read_size) != TRUE) 
        return ERROR_SWD_READ;     
  
    return ERROR_SUCCESS;
}
static error_t bootisp_prog_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{

      return ERROR_SUCCESS; 
}
static error_t bootisp_prog_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr)
{
    uint8_t ret ; 
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    
    ret = bootisp_write_memory( addr, data, size);
    if(ret != TRUE)
    {
        if(failed_addr)
            *failed_addr = 0xFFFFFFFF ; 
        return ERROR_BOOTISP_WRITE;
    }
    return ERROR_SUCCESS; 
}
static error_t bootisp_prog_read_flash(uint32_t addr, uint8_t *data, uint32_t size)
{
    if(bootisp_read_memory(addr, data, size) != TRUE)
        return ERROR_BOOTISP_READ;
    return ERROR_SUCCESS; 
}
static error_t bootisp_prog_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
     return ERROR_SUCCESS; 
}
static error_t bootisp_target_program_config_all(uint32_t *failed_addr)
{
    error_t ret = ERROR_SUCCESS;
        
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	 
    
    uint32_t copy_size;
    uint32_t read_addr;
    uint8_t read_buf[BOOTISP_PRG_SIZE] = {0x00};
    
//    ret = isp_chipid_check();
//    if(ERROR_SUCCESS != ret)
//        return ret; 
        
    cfg_word_addr =  target_dev->config_word_start;
	cfg_word_size =  target_dev->config_word_size;
    read_addr =  0;

    while(true)
    {
        copy_size = MIN(cfg_word_size, sizeof(read_buf) );
        
        ret = bootisp_prog_intf.cb(CFG_WORD, read_addr, read_buf , copy_size);
        if(ERROR_SUCCESS != ret)
            return ret;     
        ret = bootisp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
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
static error_t bootisp_target_program_all(  uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr)
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
       
    ret = bootisp_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  target_dev->code_start;
	code_size =  target_dev->code_size;
    read_addr =  0; 
        
    while(true)
    {
        copy_size = MIN(code_size, sizeof(read_buf) ); 
        ret = bootisp_prog_intf.cb(USER_HEX, read_addr, read_buf , copy_size);
        
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
            ret = bootisp_prog_program_flash(code_addr, read_buf, copy_size, failed_addr); 
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
//    cfg_word_addr =  target_dev->config_word_start;
//	cfg_word_size =  target_dev->config_word_size;
//    read_addr =  0;
//    while(true)
//    {
//        copy_size = MIN(cfg_word_size, sizeof(read_buf) );          
//        ret = bootisp_prog_intf.cb(CFG_WORD, read_addr, read_buf , copy_size);
//        if(ERROR_SUCCESS != ret)
//            return ret;     		
//        ret = bootisp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
//        if( ret !=  ERROR_SUCCESS)
//            return ret;             
//        // Update variables
//        cfg_word_addr  += copy_size;
//        cfg_word_size  -= copy_size;
//        read_addr += copy_size;
//        // Check for end
//        if (cfg_word_size <= 0) 
//            break;       
//    }
    return ret;     
}
static error_t bootisp_target_verify_all( uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data)
{
    return ERROR_SUCCESS; 
}





//static uint32_t last_addr;
//__attribute__((aligned(4)))
//static uint8_t buf[BOOTSIP_DATA_MAX_LEN];


//复位方式有多种
//1、硬件复位

//2、RESET=SUPPOT
//go oxffffffff

//3、RESET ！= SUPPOT


//error_t bootisp_uninit(void)
//{
////    if (STATE_CLOSED == state) 
////    {
//////        util_assert(0);
////        return ERROR_INTERNAL;
////    } 
////    state =  STATE_CLOSED;    
//}
////offline 操作函数
////擦除
//#define FULL_ERASE_CMD      0xFFFF
//#define FULL_ERASE_CMD_H    0xFF
//#define FULL_ERASE_CMD_L    0xFF

////擦除
//error_t bootisp_erase_chip(uint8_t erase_mode)
//{  
//    error_t status = ERROR_SUCCESS;
//    uint8_t data[4];        
//    
//    if( erase_mode  == FULL_ERASE)
//    {
//        //全擦+info页擦+全芯片软复位+同步  
//        //全擦
//        data[0] = FULL_ERASE_CMD_H;
//        data[1] = FULL_ERASE_CMD_L;        
//        if(extended_erase(data , 2) != 0)
//        {
//            bootisp_uninit();
//            return ERROR_BOOTISP_WRITE; 

//        }            
//        //info页擦（除RDP页）
//        data[0] = 0x00;
//        data[1] = 0x00;     //页数
//        data[2] = 0x01;
//        data[3] = 0x01;
//        if(extended_erase( data, 4) != 0)
//        {
//            bootisp_uninit();
//            return ERROR_BOOTISP_WRITE; 
//        }
//        //复位
////        bootisp_set_target_reset();
//        //同步
//        bootisp_start();
//        
//    }
//    else if( erase_mode  == PAGE_EARSE)
//    {
//        //TODO:页擦，擦除hex所在的页。需要判断目标芯片hex所在区域。
//        
//    }
//    
//    return status;
//  
//    
//}
////编程
//error_t bootisp_flash_program(uint32_t addr,  uint8_t *data, uint32_t size)
//{
//    uint32_t copy_size;
//    error_t status = ERROR_SUCCESS;
//    

//    while (true) 
//    {
//        copy_size = MIN(size, BOOTSIP_DATA_MAX_LEN );
////        memcpy(buf, data, copy_size);         
//        if(write_memory(addr, data, copy_size) != 0)  
//        {
//            return ERROR_BOOTISP_WRITE; 
//        }
//                     
//        // Update variables
//        addr += copy_size;
//        data += copy_size;
//        size -= copy_size;
//        // Check for end
//        if (size <= 0) {
//            break;
//        }             
//    } 
////    last_addr = addr;    
//    return status;
////    write_memory_cmd();    
//}
////查空
////只判断flash区域
//error_t bootisp_check_empty(void)
//{       
//    if(bootisp_start() != 0)
//        return  ERROR_BOOTISP_START;
////    check_empty(bootisp_target_dev.flash_start,);      
//    if(check_empty(0x00000000,0x0003FFFF) != 0)
//        return  ERROR_BOOTISP_CHECK_EMPTY;
//    if(check_empty(0x00040400,0x000007FF) != 0)
//        return  ERROR_BOOTISP_CHECK_EMPTY;
//    
//    return ERROR_SUCCESS;       
//}
////校验
////读出
//error_t bootisp_verify(void)
//{
//    error_t status = ERROR_SUCCESS;
//    
//    return status;
//    
//    
//}
