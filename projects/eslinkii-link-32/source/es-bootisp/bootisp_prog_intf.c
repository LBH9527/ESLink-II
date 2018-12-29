#include "string.h"
#include "stdbool.h"
#include "eslink.h"
#include "errno.h"

#include "bootisp_prog_intf.h"
#include "bootisp_host.h"
#include "bootisp_target_config.h"

typedef enum {
    STATE_CLOSED,
    STATE_OPEN,
    STATE_ERROR
} state_t;
static state_t state = STATE_CLOSED;
static uint32_t last_addr;
__attribute__((aligned(4)))
static uint8_t buf[BOOTSIP_DATA_MAX_LEN];


//复位方式有多种
//1、硬件复位
__attribute__((weak)) void bootisp_set_target_reset(uint8_t asserted)
{
//    (asserted) ? PIN_nRESET_OUT(0) : PIN_nRESET_OUT(1);
}
//2、RESET=SUPPOT
//go oxffffffff

//3、RESET ！= SUPPOT



error_t bootisp_init(void)
{       
    if(bootisp_start() != 0)
        return  ERROR_BOOTISP_START;
    state =  STATE_CLOSED;
    last_addr = 0;
    return ERROR_SUCCESS;           
}

error_t bootisp_uninit(void)
{
//    if (STATE_CLOSED == state) 
//    {
////        util_assert(0);
//        return ERROR_INTERNAL;
//    } 
    state =  STATE_CLOSED;    
}
//offline 操作函数
//擦除
#define FULL_ERASE_CMD      0xFFFF
#define FULL_ERASE_CMD_H    0xFF
#define FULL_ERASE_CMD_L    0xFF

//擦除
error_t bootisp_erase_chip(uint8_t erase_mode)
{  
    error_t status = ERROR_SUCCESS;
    uint8_t data[4];        
    
    if( erase_mode  == FULL_ERASE)
    {
        //全擦+info页擦+全芯片软复位+同步  
        //全擦
        data[0] = FULL_ERASE_CMD_H;
        data[1] = FULL_ERASE_CMD_L;        
        if(extended_erase(data , 2) != 0)
        {
            bootisp_uninit();
            return ERROR_BOOTISP_WRITE; 

        }            
        //info页擦（除RDP页）
        data[0] = 0x00;
        data[1] = 0x00;     //页数
        data[2] = 0x01;
        data[3] = 0x01;
        if(extended_erase( data, 4) != 0)
        {
            bootisp_uninit();
            return ERROR_BOOTISP_WRITE; 
        }
        //复位
//        bootisp_set_target_reset();
        //同步
        bootisp_start();
        
    }
    else if( erase_mode  == PAGE_EARSE)
    {
        //TODO:页擦，擦除hex所在的页。需要判断目标芯片hex所在区域。
        
    }
    
    state = STATE_OPEN;
    return status;
  
    
}
//编程
error_t bootisp_flash_program(uint32_t addr,  uint8_t *data, uint32_t size)
{
    uint32_t copy_size;
    error_t status = ERROR_SUCCESS;
    
    //编程前默认先擦除
    if (state != STATE_OPEN) {
//        util_assert(0);
//        return ERROR_INTERNAL;
    }
    while (true) 
    {
        copy_size = MIN(size, BOOTSIP_DATA_MAX_LEN );
//        memcpy(buf, data, copy_size);         
        if(write_memory(addr, data, copy_size) != 0)  
        {
            state = STATE_ERROR;
            return ERROR_BOOTISP_WRITE; 
        }
                     
        // Update variables
        addr += copy_size;
        data += copy_size;
        size -= copy_size;
        // Check for end
        if (size <= 0) {
            break;
        }             
    } 
//    last_addr = addr;    
    return status;
//    write_memory_cmd();    
}
//查空
//只判断flash区域
error_t bootisp_check_empty(void)
{       
    if(bootisp_start() != 0)
        return  ERROR_BOOTISP_START;
//    check_empty(bootisp_target_dev.flash_start,);      
    if(check_empty(0x00000000,0x0003FFFF) != 0)
        return  ERROR_BOOTISP_CHECK_EMPTY;
    if(check_empty(0x00040400,0x000007FF) != 0)
        return  ERROR_BOOTISP_CHECK_EMPTY;
    
    return ERROR_SUCCESS;       
}
//校验
//读出
error_t bootisp_verify(void)
{
    error_t status = ERROR_SUCCESS;
    
    return status;
    
    
}
