/*
*********************************************************************************************************
*
*	模块名称 : swd编程接口
*	文件名称 : swd_prg_intf.c
*	说    明 : 提供脱机编程的操作接口：查空、擦除、编程、校验。
*               
*               
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2016-08-02	9527	 
*
*
*********************************************************************************************************
*/     
//#include "string.h"
//#include "stdbool.h"



//typedef enum {
//    STATE_CLOSED,
//    STATE_OPEN,
//    STATE_ERROR
//} state_t;

//// Target programming expects buffer
//// passed in to be 4 byte aligned
//__attribute__((aligned(4)))
//static uint8_t buf[1024];
//static bool buf_empty;
//static bool current_sector_valid;
//static bool page_erase_enabled = false;
//static uint32_t current_write_block_addr;
//static uint32_t current_write_block_size;
//static uint32_t current_sector_addr;
//static uint32_t current_sector_size;
//static uint32_t last_addr;
////static const flash_intf_t *intf;
//static state_t state = STATE_CLOSED;

////static bool flash_intf_valid(const flash_intf_t *flash_intf);
//static error_t setup_next_sector(uint32_t addr);


//error_t flash_manager_uninit(void)
//{
//    error_t flash_uninit_error;
//    error_t flash_write_error = ERROR_SUCCESS;
////    flash_manager_printf("flash_manager_uninit()\r\n");

//    if (STATE_CLOSED == state) {
////        util_assert(0);
////        return ERROR_INTERNAL;
//    }

//    // Write out current page
//    if ((STATE_OPEN == state) && (!buf_empty)) {
//        flash_write_error = flash_intf_target->program_page(current_write_block_addr, buf, current_write_block_size);
////        flash_manager_printf("    intf->program_page(addr=0x%x, size=0x%x) ret=%i\r\n",
////                             current_write_block_addr, current_write_block_size, flash_write_error);
//    }

//    // Close flash interface (even if there was an error during program_page)
//    flash_uninit_error = flash_intf_target->uninit();
////    flash_manager_printf("    intf->uninit() ret=%i\r\n", flash_uninit_error);
//    // Reset variables to catch accidental use
//    memset(buf, 0xFF, sizeof(buf));
//    buf_empty = true;
//    current_sector_valid = false;
//    current_write_block_addr = 0;
//    current_write_block_size = 0;
//    current_sector_addr = 0;
//    current_sector_size = 0;
//    last_addr = 0;
//    state = STATE_CLOSED;

//    // Make sure an error from a page write or from an
//    // uninit gets propagated
//    if (flash_uninit_error != ERROR_SUCCESS) {
//        return flash_uninit_error;
//    }

//    if (flash_write_error != ERROR_SUCCESS) {
//        return flash_write_error;
//    }

//    return ERROR_SUCCESS;
//}

//void flash_manager_set_page_erase(bool enabled)
//{
//    page_erase_enabled = enabled;
//}

////
//static error_t setup_next_sector(uint32_t addr)
//{
//    uint32_t min_prog_size;     //最小编程大小
//    uint32_t sector_size;       //扇区大小
//    error_t status;
//    min_prog_size = flash_intf_target->program_page_min_size(addr);
//    sector_size = flash_intf_target->erase_sector_size(addr);

//    if ((min_prog_size <= 0) || (sector_size <= 0)) {
//        // Either of these conditions could cause divide by 0 error
////        util_assert(0);
////        return ERROR_INTERNAL;
//    }

//    // Assert required size and alignment
////    util_assert(sizeof(hex_buf) >= min_prog_size);
////    util_assert(sizeof(hex_buf) % min_prog_size == 0);
////    util_assert(sector_size >= min_prog_size);
////    util_assert(sector_size % min_prog_size == 0);
//    // Setup global variables
//    current_sector_addr = ROUND_DOWN(addr, sector_size);
//    current_sector_size = sector_size;
//    current_write_block_addr = current_sector_addr;
//    current_write_block_size = MIN(sector_size, sizeof(buf));

////    if(page_erase_enabled) {
//        // Erase the current sector
//        status = flash_intf_target->erase_sector(current_sector_addr);
////        flash_manager_printf("    intf->erase_sector(addr=0x%x) ret=%i\r\n", current_sector_addr);
//        if (ERROR_SUCCESS != status) {
//            flash_intf_target->uninit();
//            return status;
//        }         
////    }

//    // Clear out buffer in case block size changed
//    memset(buf, 0xFF, current_write_block_size);
////    flash_manager_printf("    setup_next_sector(addr=0x%x) sect_addr=0x%x, write_addr=0x%x,\r\n",
////                         addr, current_sector_addr, current_write_block_addr);
////    flash_manager_printf("        actual_write_size=0x%x, sector_size=0x%x, min_write=0x%x\r\n",
////                         current_write_block_size, current_sector_size, min_prog_size);
//    return ERROR_SUCCESS;
//}



////------------------------------------------------------------------------------
////static target_cfg_t* swd_target_dev ;


////void ofl_swd_init(target_cfg_t target_device_cfg)
////{
////    target_device = target_device_cfg;
////    
////}

////1、初始化，把算法烧录到目标芯片的RAM中
// error_t swd_prog_init(target_cfg_t *target_dev)
// {
// 
// 
// 
// }
//error_t swd_prg_init( uint32_t addr, const uint8_t *data, uint32_t size)
//{
//    error_t status;
//    // Assert that interface has been properly uninitialized
////    flash_manager_printf("flash_manager_init()\r\n");

//    if (state != STATE_CLOSED) {
////        util_assert(0);
////        return ERROR_INTERNAL;
//    }

//    // Check for a valid flash interface
////    if (!flash_intf_valid(flash_intf_target)) {
////        util_assert(0);
////        return ERROR_INTERNAL;
////    }

//    // Initialize variables
//    memset(buf, 0xFF, sizeof(buf));
//    buf_empty = true;
//    current_sector_valid = false;
//    current_write_block_addr = 0;
//    current_write_block_size = 0;
//    current_sector_addr = 0;
//    current_sector_size = 0;
//    last_addr = 0;

//    // Initialize flash
//    status = flash_intf_target->init();
////    flash_manager_printf("    intf->init ret=%i\r\n", status);

//    return status;
//}

////擦除
//error_t swd_erase_chip(void)
//{
//    error_t status = ERROR_SUCCESS;

//    // Initialize variables
//    memset(buf, 0xFF, sizeof(buf));
//    buf_empty = true;
//    current_sector_valid = false;
//    current_write_block_addr = 0;
//    current_write_block_size = 0;
//    current_sector_addr = 0;
//    current_sector_size = 0;
//    last_addr = 0;

//    // Initialize flash
//    status = flash_intf_target->init();
////    flash_manager_printf("    intf->init ret=%i\r\n", status);

//    if (ERROR_SUCCESS != status) {
//        return status;
//    }

//    status = flash_intf_target->erase_chip();
////        flash_manager_printf("    intf->erase_chip ret=%i\r\n", status);

//    if (ERROR_SUCCESS != status) 
////            flash_intf_target->uninit();
//        return status;
//    return status;
//}

////编程
//error_t swd_prg_program(uint32_t addr, const uint8_t *data, uint32_t size)
//{
//    uint32_t size_left;
//    uint32_t copy_size;
//    uint32_t pos;
//    error_t status = ERROR_SUCCESS;
////    flash_manager_printf("flash_manager_data(addr=0x%x size=0x%x)\r\n", addr, size);

//    // Enforce that addresses are sequential.  Currently flash manager
//    // only supports sequential addresses.  In the future flash manager
//    // could be updated to support this.
//    if (addr < last_addr) {
////        util_assert(0);
//        state = STATE_ERROR;
////        return ERROR_INTERNAL;
//    }

//    // Setup the current sector if it is not setup already
//    if (!current_sector_valid) {
//        status = setup_next_sector(addr);

//        if (ERROR_SUCCESS != status) {
//            state = STATE_ERROR;
//            return status;
//        }
//        current_sector_valid = true;
//    }

//    while (true) 
//    {
//        // flush if necessary
//        if (addr >= current_write_block_addr + current_write_block_size) {
//            // Write out current buffer
//           
////            flash_manager_printf("    intf->program_page(addr=0x%x, size=0x%x) ret=%i\r\n", current_write_block_addr, current_write_block_size, status);

//            if (ERROR_SUCCESS != status) {
//                state = STATE_ERROR;
//                return status;
//            }

//            // Setup for next page
//            memset(buf, 0xFF, current_write_block_size);
//            buf_empty = true;
//            current_write_block_addr += current_write_block_size;
//        }

//        // Check for end
//        if (size <= 0) {
//            break;
//        }

//        // Change sector if necessary
//        if (addr >= current_sector_addr + current_sector_size) {
//            status = setup_next_sector(addr);

//            if (ERROR_SUCCESS != status) {
//                state = STATE_ERROR;
//                return status;
//            }
//        }

//        // write buffer
//        pos = addr - current_write_block_addr;
//        size_left = current_write_block_size - pos;
//        copy_size = MIN(size, size_left);
//        memcpy(buf + pos, data, copy_size);
//        buf_empty = copy_size == 0;
//        // Update variables
//        addr += copy_size;
//        data += copy_size;
//        size -= copy_size;
//    }

//    last_addr = addr;
//    return status;
//}
////查空
////读目标芯片中的数据与下载的数据，比较是否都为0xFF
//error_t swd_prg_check_empty(void)
//{
//    error_t status = ERROR_SUCCESS;
//    
//    return status;
//}


////读目标芯片中的数据与下载的数据，比较是否一致
//error_t swd_prg_verify(void)
//{
//    error_t status = ERROR_SUCCESS;
////                uint8_t rb_buf[16];
////                uint32_t verify_size = MIN(write_size, sizeof(rb_buf));
////                if (!swd_read_memory(addr, rb_buf, verify_size)) {
////                    return ERROR_ALGO_DATA_SEQ;
////                }
////                if (memcmp(buf, rb_buf, verify_size) != 0) {
////                    return ERROR_WRITE;
////                }
////                addr += verify_size;
////                buf += verify_size;
////                size -= verify_size;
////                write_size -= verify_size;   
//    return status;
//}



//error_t swd_test()
//{
//     uint8_t data[256];
//     
//     flash_intf_target->init();
////     flash_intf_target->erase_chip();
//      if (! {
//        return ERROR_ALGO_DATA_SEQ;
//    }
//    return ERROR_SUCCESS; 


//}

#include "eslink.h"
#include "es_common.h"   
#include "swd_prog_intf.h"
#include "swd_host.h"
#include "swd_target_config.h"
#include "swd_target_reset.h"  

#include "sflash_port.h"

//一次编程支持的长度，根据RAM大小可以修改.长度需要为2^n
#define SWD_PRG_MINI_SIZE  1024  

static void swd_prog_init(es_target_cfg *target);
static error_t es_swd_init(void); //进模式
static error_t es_swd_uninit(void); //退出模式

static error_t es_swd_erase_chip (uint8_t para);
static error_t es_swd_check_empty(uint32_t *failed_addr, uint32_t *failedData) ;
static error_t es_swd_read_chipid(uint32_t *buf);
static error_t es_swd_read_checksum(uint32_t *buf);
static error_t es_swd_encrypt_chip(void);
static error_t es_swd_chipid_check(void);
static error_t es_swd_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_addr );
static error_t es_swd_read_config(uint32_t addr,  uint8_t *buf, uint32_t size);
static error_t es_swd_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t es_swd_program_flash(uint32_t addr, uint8_t *data, uint32_t size, uint32_t *failed_addr);
static error_t es_swd_read_flash(uint32_t addr, uint8_t *data, uint32_t size);
static error_t es_swd_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data);
static error_t swd_target_program_config_all(uint32_t *failed_addr);
static error_t swd_target_program_all(uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr);
static error_t swd_target_verify_all(uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data);

struct  es_prog_ops swd_prog_intf = {
    swd_prog_init,
    es_swd_init,
    es_swd_uninit,
    es_swd_erase_chip,
    es_swd_check_empty,
    es_swd_read_chipid,
//    swd_chipid_check,
    es_swd_read_checksum,
    es_swd_encrypt_chip,

    es_swd_program_config,
    es_swd_read_config,
    es_swd_verify_config,
    es_swd_program_flash,
    es_swd_read_flash,
    es_swd_verify_flash,
    swd_target_program_config_all ,
    swd_target_program_all,
    swd_target_verify_all,
};

static const es_target_cfg *target_dev;   
//swd编程初始化
void swd_prog_init(es_target_cfg *target)
{
     target_dev = target;
}

//编程初始化，进模式
//算法文件写入目标芯片的RAM中
static error_t es_swd_init(void)
{
    const program_target_t *const flash = swd_target_device.flash_algo;

    if (0 == target_set_state(RESET_PROGRAM)) {
        return ERROR_RESET;
    }

    // Download flash programming algorithm to target and initialise.
    if (0 == swd_write_memory(flash->algo_start, (uint8_t *)flash->algo_blob, flash->algo_size)) {
        return ERROR_ALGO_DL;
    }

    if (0 == swd_flash_syscall_exec(&flash->sys_call_s, flash->init, swd_target_device.flash_start, 0, 0, 0)) {
        return ERROR_INIT;
    }

    return ERROR_SUCCESS;
}
//isp退出编程模式
static error_t es_swd_uninit(void)
{
    if (config_get_auto_rst()) {
        // Resume the target if configured to do so
        target_set_state(RESET_RUN);
    } else {
        // Leave the target halted until a reset occurs
        target_set_state(RESET_PROGRAM);
    }

    swd_off();
    return ERROR_SUCCESS;
}  
//读芯片chipid
static error_t es_swd_read_chipid(uint32_t *buf)
{  
//    if(swd_read_config(target_dev->chipid_addr, buf, 1) != TRUE)
//        return ERROR_ISP_READ_CFG_WORD;
    return ERROR_SUCCESS; 
}
//判断chipid是否正确
static error_t es_swd_chipid_check(void)
{
    return ERROR_SUCCESS; 
}
//读芯片校验和
static error_t es_swd_read_checksum(uint32_t *buf)
{
    return ERROR_SUCCESS; 
}

//flash编程
static error_t es_swd_program_flash(uint32_t addr, uint8_t *buf, uint32_t size, uint32_t *failed_addr)
{
    const program_target_t *const flash = swd_target_device.flash_algo;

    // check if security bits were set
    if (1 == security_bits_set(addr, (uint8_t *)buf, size)) {
        return ERROR_SECURITY_BITS;
    }

    while (size > 0) {
        uint32_t write_size = MIN(size, flash->program_buffer_size);

        // Write page to buffer
        if (!swd_write_memory(flash->program_buffer, (uint8_t *)buf, write_size)) {
            return ERROR_ALGO_DATA_SEQ;
        }

        // Run flash programming
        if (!swd_flash_syscall_exec(&flash->sys_call_s,
                                    flash->program_page,
                                    addr,
                                    flash->program_buffer_size,
                                    flash->program_buffer,
                                    0)) {
            return ERROR_WRITE;
        }
         //是否需要编程时自动校验
        if (config_get_automation_allowed()) {
            // Verify data flashed if in automation mode
            while (write_size > 0) {
                uint8_t rb_buf[16];
                uint32_t verify_size = MIN(write_size, sizeof(rb_buf));
                if (!swd_read_memory(addr, rb_buf, verify_size)) {
                    return ERROR_ALGO_DATA_SEQ;
                }
                if (memcmp(buf, rb_buf, verify_size) != 0) {
                    return ERROR_WRITE;
                }
                addr += verify_size;
                buf += verify_size;
                size -= verify_size;
                write_size -= verify_size;
            }
        } else {
            addr += write_size;
            buf += write_size;
            size -= write_size;
        }
    }

    return ERROR_SUCCESS;
}
//读flash
static error_t es_swd_read_flash(uint32_t addr, uint8_t *buf, uint32_t size)
{
    if (!swd_read_memory(addr, buf, size)) 
    {
        return ERROR_SWD_READ;
    }
    return ERROR_SUCCESS;
}
//flash校验
static error_t es_swd_verify_flash(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{

    uint32_t i;
    uint8_t read_buf[SWD_PRG_MINI_SIZE];
    uint32_t verify_size;
    uint32_t size_in_words;    
    
    if(size & 0x03)
        return ERROR_OUT_OF_BOUNDS;
    
    while (size_in_words > 0) 
    {          
        verify_size = MIN(size, sizeof(read_buf));
        if (!swd_read_memory(addr, read_buf, verify_size)) 
        {
            return ERROR_SWD_READ;
        }
        for(i=0; i< verify_size; i++)
        {
            if( data[i] != read_buf[i] ) 
            {
                *failed_addr = addr + ROUND_DOWN(i, 4)  ;
                *failed_data |= (read_buf[*failed_addr] << 0) ; 
                *failed_data |= (read_buf[*failed_addr+1] << 8) ; 
                *failed_data |= (read_buf[*failed_addr+2] << 16) ; 
                *failed_data |= (read_buf[*failed_addr+3] << 24) ; 
				return ERROR_ISP_VERIFY;              
            }
        } 
        addr += verify_size;
        size_in_words -= verify_size;
    }      
    return ERROR_SUCCESS;  
}
//配置字编程
static error_t es_swd_program_config(uint32_t addr, uint8_t *data, uint32_t size,uint32_t *failed_addr )
{
     return ERROR_SUCCESS;  
}
//读配置字
static error_t es_swd_read_config(uint32_t addr,  uint8_t *buf, uint32_t size)
{
     return ERROR_SUCCESS; 
}
//配置字校验
static error_t es_swd_verify_config(uint32_t addr,  uint8_t *data, uint32_t size, uint32_t *failed_addr, uint32_t *failed_data)
{
     return ERROR_SUCCESS; 
}


static error_t es_swd_encrypt_chip(void)
{
    return ERROR_SUCCESS; 
}
//擦除
static error_t es_swd_erase_chip (uint8_t para)
{
    error_t status = ERROR_SUCCESS;
    const program_target_t *const flash = swd_target_device.flash_algo;

    if (0 == swd_flash_syscall_exec(&flash->sys_call_s, flash->erase_chip, 0, 0, 0, 0)) {
        return ERROR_ERASE_ALL;
    }

//    // Reset and re-initialize the target after the erase if required
//    if (swd_target_device.erase_reset) {
//        status = target_flash_init();
//    }      

    return status; 
}

/*******************************************************************************
*	函 数 名: es_swd_check_empty
*	功能说明: 查空
*	形    参: failed_addr：错误地址 failed_data：错误数据 
*	返 回 值: 错误类型
*******************************************************************************/
error_t es_swd_check_empty(uint32_t *failed_addr, uint32_t *failed_data)           
{
    error_t status = ERROR_SUCCESS;
	uint32_t i;
	uint32_t code_addr;	
	uint32_t code_size;	
//    uint32_t cfg_word_addr;	
//	uint32_t cfg_word_size;	
    
    uint8_t read_buf[SWD_PRG_MINI_SIZE]; 
	uint32_t copy_size;   
    
    status = es_swd_chipid_check();
    if(ERROR_SUCCESS != status)
        return status;   
        
    //flash查空
	code_addr =  swd_target_device.flash_start ;
	code_size =  swd_target_device.flash_end ; 
	while(true)
	{                 
		copy_size = MIN(code_size, sizeof(read_buf));      
	    swd_read_memory(code_addr, read_buf, copy_size);
		for(i = 0; i<copy_size; i++)
		{
			if(read_buf[i] != 0xFF)
			{              
			    *failed_addr = code_addr + ROUND_DOWN(i, 4)  ;
                *failed_data |= (read_buf[*failed_addr] << 0) ; 
                *failed_data |= (read_buf[*failed_addr+1] << 8) ; 
                *failed_data |= (read_buf[*failed_addr+2] << 16) ; 
                *failed_data |= (read_buf[*failed_addr+3] << 24) ; 
				return ERROR_ISP_FLASH_CHECK_EMPTY;
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

//    //配置字查空
//	cfg_word_addr =  isp_target_dev->config_word_start + 0x400;
//	cfg_word_size =  14;     //字长度
//    while(true)
//	{
//		copy_size = MIN(cfg_word_size, sizeof(read_buf)/4 );
//	    isp_read_config(cfg_word_addr, read_buf, copy_size);
//		for(i = 0; i<copy_size; i++)
//		{
//			if(read_buf[i] != 0xFFFFFFFF)
//			{              
//			    *failed_addr = cfg_word_addr + i*4  ;
//                *failed_data = read_buf[i] ; 
//				return ERROR_ISP_CFG_WORD_CHECK_EMPTY;
//			} 				
//		} 
//        // Update variables
//        cfg_word_addr  += copy_size;
//        cfg_word_size  -= copy_size;
//        
//        // Check for end
//        if (code_size <= 0) {
//            break;
//        } 
//	} 
//    
//	cfg_word_addr =  isp_target_dev->config_word_start + 0x7C0;
//	cfg_word_size =  24;     //字长度
//    while(true)
//	{
//		copy_size = MIN(cfg_word_size, sizeof(read_buf)/4 );
//	    isp_read_config(cfg_word_addr, read_buf, copy_size);
//		for(i = 0; i<copy_size; i++)
//		{
//			if(read_buf[i] != 0xFFFFFFFF)
//			{              
//			    *failed_addr = cfg_word_addr + i*4  ;
//                *failed_data = read_buf[i] ; 
//				return ERROR_ISP_CFG_WORD_CHECK_EMPTY;
//			} 				
//		} 
//        // Update variables
//        cfg_word_addr  += copy_size;
//        cfg_word_size  -= copy_size;
//        
//        // Check for end
//        if (code_size <= 0) {
//            break;
//        } 
//	} 

//    cfg_word_addr =  isp_target_dev->config_word_start + 0x1000;
//	cfg_word_size =  10;     //字长度
//    while(true)
//	{
//		copy_size = MIN(cfg_word_size, sizeof(read_buf)/4 );
//	    isp_read_config(cfg_word_addr, read_buf, copy_size);
//		for(i = 0; i<copy_size; i++)
//		{
//			if(read_buf[i] != 0xFFFFFFFF)
//			{              
//			    *failed_addr = cfg_word_addr + i*4  ;
//                *failed_data = read_buf[i] ; 
//				return ERROR_ISP_CFG_WORD_CHECK_EMPTY;
//			} 				
//		} 
//        // Update variables
//        cfg_word_addr  += copy_size;
//        cfg_word_size  -= copy_size;
//        
//        // Check for end
//        if (code_size <= 0) {
//            break;
//        } 
//	} 
    
    return ERROR_SUCCESS;     
}  

/*******************************************************************************
*	函 数 名: swd_target_program_config_all
*	功能说明: 芯片配置字编程。
*	形    参: failed_addr：错误地址  
*	返 回 值: 错误类型
*******************************************************************************/
static error_t swd_target_program_config_all(uint32_t *failed_addr)
{
    error_t ret = ERROR_SUCCESS;
    
//    uint32_t cfg_word_addr;	
//	uint32_t cfg_word_size;	 
//    
//    uint32_t copy_size;
//    uint32_t read_addr;
//    uint8_t read_buf[FLASH_PRG_MIN_SIZE] = {0x00};
//    
//    ret = isp_chipid_check();
//    if(ERROR_SUCCESS != ret)
//        return ret; 
//        
//    cfg_word_addr =  isp_target_dev->config_word_start;
//	cfg_word_size =  isp_target_dev->config_word_size;
//    read_addr =  0;

//    while(true)
//    {
//        copy_size = MIN(cfg_word_size, sizeof(read_buf) );
//        
//        ret = online_file_read(CFG_WORD, read_addr, read_buf , copy_size);
//        if(ERROR_SUCCESS != ret)
//            return ret;     
//        ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
//        if(ERROR_SUCCESS != ret)
//            return ret;              
//        // Update variables
//        cfg_word_addr  += copy_size;
//        cfg_word_size  -= copy_size;
//        read_addr += copy_size;
//        // Check for end
//        if (cfg_word_size <= 0) 
//            break;       
//    }
    return ERROR_SUCCESS;     
} 
    
/*******************************************************************************
*	函 数 名: swd_target_program_all
*	功能说明: 芯片编程。flash和配置字编程
*	形    参: sn_enable：是否已编程序列号 sn：序列号代码 
*             failed_addr：错误地址   failed_data ：错误数据
*	返 回 值: 编程错误地址
*******************************************************************************/
static error_t swd_target_program_all(uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr) 
{
    error_t ret = ERROR_SUCCESS;
    uint32_t i;
    
    uint32_t code_addr;	
	uint32_t code_size;	
    uint32_t cfg_word_addr;	
	uint32_t cfg_word_size;	   
    
    uint32_t copy_size;      
    uint32_t read_addr;
    uint8_t read_buf[SWD_PRG_MINI_SIZE];
       
    ret = es_swd_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  swd_target_device.flash_start;
	code_size =  swd_target_device.flash_end;
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
            ret = es_swd_program_flash(code_addr, read_buf, copy_size, failed_addr); 
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
//    cfg_word_addr =  isp_target_dev->config_word_start;
//	cfg_word_size =  isp_target_dev->config_word_size;
//    read_addr =  0;
//    while(true)
//    {
//        copy_size = MIN(cfg_word_size, sizeof(read_buf) );          
//        ret = online_file_read(CFG_WORD, read_addr, read_buf , copy_size);
//        if(ERROR_SUCCESS != ret)
//            return ret;     		
//        ret = isp_prog_program_config(cfg_word_addr, read_buf, copy_size, failed_addr); 
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

/*******************************************************************************
*	函 数 名: isp_target_verify_all
*	功能说明: 芯片验证
*	形    参: sn_enable：是否已编程序列号 sn：序列号代码 
*             failed_addr：错误地址   
*	返 回 值: 编程错误地址
*******************************************************************************/
static error_t  swd_target_verify_all(uint8_t sn_enable, serial_number_t *sn , uint32_t *failed_addr, uint32_t *failed_data)
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
    uint8_t sf_buf[SWD_PRG_MINI_SIZE];     
  
    ret = es_swd_chipid_check();
    if(ERROR_SUCCESS != ret)
        return ret; 
        
    code_addr =  swd_target_device.flash_start;
	code_size =  swd_target_device.flash_end ;
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
        ret = es_swd_verify_flash(code_addr, sf_buf, verify_size,failed_addr,failed_data);                        
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
    
//    cfg_word_addr =  isp_target_dev->config_word_start;
//	cfg_word_size =  isp_target_dev->config_word_size;
//    sf_addr =  0;
//    checksum = 0;
//    while(true)
//    {
//        verify_size = MIN(cfg_word_size, sizeof(sf_buf) );          
//        ret = online_file_read(CFG_WORD, sf_addr, sf_buf , verify_size);
//        if( ret !=  ERROR_SUCCESS)
//            return ret; 
//        checksum += check_sum(verify_size, sf_buf);     //计算原始数据校验和
//        
//        ret = isp_prog_verify_config(cfg_word_addr, sf_buf, verify_size,failed_addr,failed_data); 
//        if( ret !=  ERROR_SUCCESS)
//            return ret; 
//        // Update variables
//        cfg_word_addr  += verify_size;
//        cfg_word_size  -= verify_size;
//        sf_addr += verify_size;
//        // Check for end
//        if (cfg_word_size <= 0) 
//            break;       
//    }
//    online_file_read(CFG_WORD_CHECKSUM, 0,(uint8_t*)&sf_checksum, 4);        
//    if(sf_checksum != (checksum&0x0000ffff))
//    {
//        ret = ERROR_CFG_WORD_CHECKSUM;
//        return  ret; 
//    }           
    return  ret; 
}


