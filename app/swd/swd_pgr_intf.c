
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
*		V1.0    2016-08-02	9527	 按键
*
*
*********************************************************************************************************
*/     
#include "string.h"
#include "stdbool.h"
#include "swd_pgr_intf.h"
#include "es_common.h"
#include "error.h"
// Set to 1 to enable debugging
#define DEBUG_FLASH_MANAGER     0

#if DEBUG_FLASH_MANAGER
#include "daplink_debug.h"
#define flash_manager_printf    debug_msg
#else
#define flash_manager_printf(...)
#endif

typedef enum {
    STATE_CLOSED,
    STATE_OPEN,
    STATE_ERROR
} state_t;

// Target programming expects buffer
// passed in to be 4 byte aligned
__attribute__((aligned(4)))
static uint8_t buf[1024];
static bool buf_empty;
static bool current_sector_valid;
static bool page_erase_enabled = false;
static uint32_t current_write_block_addr;
static uint32_t current_write_block_size;
static uint32_t current_sector_addr;
static uint32_t current_sector_size;
static uint32_t last_addr;
//static const flash_intf_t *intf;
static state_t state = STATE_CLOSED;

static bool flash_intf_valid(const flash_intf_t *flash_intf);
static error_t setup_next_sector(uint32_t addr);


error_t flash_manager_uninit(void)
{
    error_t flash_uninit_error;
    error_t flash_write_error = ERROR_SUCCESS;
    flash_manager_printf("flash_manager_uninit()\r\n");

    if (STATE_CLOSED == state) {
//        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Write out current page
    if ((STATE_OPEN == state) && (!buf_empty)) {
        flash_write_error = flash_intf_target->program_page(current_write_block_addr, buf, current_write_block_size);
        flash_manager_printf("    intf->program_page(addr=0x%x, size=0x%x) ret=%i\r\n",
                             current_write_block_addr, current_write_block_size, flash_write_error);
    }

    // Close flash interface (even if there was an error during program_page)
    flash_uninit_error = flash_intf_target->uninit();
    flash_manager_printf("    intf->uninit() ret=%i\r\n", flash_uninit_error);
    // Reset variables to catch accidental use
    memset(buf, 0xFF, sizeof(buf));
    buf_empty = true;
    current_sector_valid = false;
    current_write_block_addr = 0;
    current_write_block_size = 0;
    current_sector_addr = 0;
    current_sector_size = 0;
    last_addr = 0;
    state = STATE_CLOSED;

    // Make sure an error from a page write or from an
    // uninit gets propagated
    if (flash_uninit_error != ERROR_SUCCESS) {
        return flash_uninit_error;
    }

    if (flash_write_error != ERROR_SUCCESS) {
        return flash_write_error;
    }

    return ERROR_SUCCESS;
}

void flash_manager_set_page_erase(bool enabled)
{
    page_erase_enabled = enabled;
}

static bool flash_intf_valid(const flash_intf_t *flash_intf)
{
    // Check for all requried members
    if (0 == flash_intf) {
        return false;
    }

    if (0 == flash_intf->uninit) {
        return false;
    }

    if (0 == flash_intf->program_page) {
        return false;
    }

    if (0 == flash_intf->erase_sector) {
        return false;
    }

    if (0 == flash_intf->erase_chip) {
        return false;
    }

    if (0 == flash_intf->program_page_min_size) {
        return false;
    }

    if (0 == flash_intf->erase_sector_size) {
        return false;
    }

    return true;
}

static error_t setup_next_sector(uint32_t addr)
{
    uint32_t min_prog_size;
    uint32_t sector_size;
    error_t status;
    min_prog_size = flash_intf_target->program_page_min_size(addr);
    sector_size = flash_intf_target->erase_sector_size(addr);

    if ((min_prog_size <= 0) || (sector_size <= 0)) {
        // Either of these conditions could cause divide by 0 error
//        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Assert required size and alignment
//    util_assert(sizeof(hex_buf) >= min_prog_size);
//    util_assert(sizeof(hex_buf) % min_prog_size == 0);
//    util_assert(sector_size >= min_prog_size);
//    util_assert(sector_size % min_prog_size == 0);
    // Setup global variables
    current_sector_addr = ROUND_DOWN(addr, sector_size);
    current_sector_size = sector_size;
    current_write_block_addr = current_sector_addr;
    current_write_block_size = MIN(sector_size, sizeof(buf));

//    if(page_erase_enabled) {
        // Erase the current sector
        status = flash_intf_target->erase_sector(current_sector_addr);
        flash_manager_printf("    intf->erase_sector(addr=0x%x) ret=%i\r\n", current_sector_addr);
        if (ERROR_SUCCESS != status) {
            flash_intf_target->uninit();
            return status;
        }         
//    }

    // Clear out buffer in case block size changed
    memset(buf, 0xFF, current_write_block_size);
    flash_manager_printf("    setup_next_sector(addr=0x%x) sect_addr=0x%x, write_addr=0x%x,\r\n",
                         addr, current_sector_addr, current_write_block_addr);
    flash_manager_printf("        actual_write_size=0x%x, sector_size=0x%x, min_write=0x%x\r\n",
                         current_write_block_size, current_sector_size, min_prog_size);
    return ERROR_SUCCESS;
}



//------------------------------------------------------------------------------
//static target_cfg_t* swd_target_dev ;


//void ofl_swd_init(target_cfg_t target_device_cfg)
//{
//    target_device = target_device_cfg;
//    
//}

//1、初始化，把算法烧录到目标芯片的RAM中

error_t swd_prg_init(target_cfg_t *target_dev, uint32_t addr, const uint8_t *data, uint32_t size)
{
    error_t status;
    // Assert that interface has been properly uninitialized
    flash_manager_printf("flash_manager_init()\r\n");

    if (state != STATE_CLOSED) {
//        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Check for a valid flash interface
    if (!flash_intf_valid(flash_intf_target)) {
//        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Initialize variables
    memset(buf, 0xFF, sizeof(buf));
    buf_empty = true;
    current_sector_valid = false;
    current_write_block_addr = 0;
    current_write_block_size = 0;
    current_sector_addr = 0;
    current_sector_size = 0;
    last_addr = 0;

    // Initialize flash
    status = flash_intf_target->init();
    flash_manager_printf("    intf->init ret=%i\r\n", status);

    return status;
}

#include "swd_host.h"
error_t swd_prg_read(void)
{    
    flash_intf_target->init();
    uint8_t rb_buf[16];

    if (!swd_read_memory(0x20000020, rb_buf, 16)) {
        return ERROR_ALGO_DATA_SEQ;
    }
    if (!swd_read_memory(0x08000000, rb_buf, 16)) {
        return ERROR_ALGO_DATA_SEQ;
    }
    if (!swd_read_memory(0xE000ED00, rb_buf, 16)) {
        return ERROR_ALGO_DATA_SEQ;
    }
}

//擦除
//2、先擦除才能进入烧录步骤
error_t swd_erase_chip(void)
{
    error_t status;
//    // Assert that interface has been properly uninitialized
//    flash_manager_printf("flash_manager_init()\r\n");

//    if (state != STATE_CLOSED) {
////        util_assert(0);
//        return ERROR_INTERNAL;
//    }

    // Check for a valid flash interface
    if (!flash_intf_valid(flash_intf_target)) {
//        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Initialize variables
    memset(buf, 0xFF, sizeof(buf));
    buf_empty = true;
    current_sector_valid = false;
    current_write_block_addr = 0;
    current_write_block_size = 0;
    current_sector_addr = 0;
    current_sector_size = 0;
    last_addr = 0;

    // Initialize flash
    status = flash_intf_target->init();
    flash_manager_printf("    intf->init ret=%i\r\n", status);

    if (ERROR_SUCCESS != status) {
        return status;
    }
    swd_prg_read();
//    if (!page_erase_enabled) 
    {
        // Erase flash and unint if there are errors
        status = flash_intf_target->erase_chip();
        flash_manager_printf("    intf->erase_chip ret=%i\r\n", status);

        if (ERROR_SUCCESS != status) {
            flash_intf_target->uninit();
            return status;
        }
        
    }

    state = STATE_OPEN;
    return status;
}

//编程
error_t swd_prg_program(uint32_t addr, const uint8_t *data, uint32_t size)
{
    uint32_t size_left;
    uint32_t copy_size;
    uint32_t pos;
    error_t status = ERROR_SUCCESS;
    flash_manager_printf("flash_manager_data(addr=0x%x size=0x%x)\r\n", addr, size);

    if (state != STATE_OPEN) {
//        util_assert(0);
        return ERROR_INTERNAL;
    }

    // Enforce that addresses are sequential.  Currently flash manager
    // only supports sequential addresses.  In the future flash manager
    // could be updated to support this.
    if (addr < last_addr) {
//        util_assert(0);
        state = STATE_ERROR;
        return ERROR_INTERNAL;
    }

    // Setup the current sector if it is not setup already
    if (!current_sector_valid) {
        status = setup_next_sector(addr);

        if (ERROR_SUCCESS != status) {
            state = STATE_ERROR;
            return status;
        }
        current_sector_valid = true;
    }

    while (true) {
        // flush if necessary
        if (addr >= current_write_block_addr + current_write_block_size) {
            // Write out current buffer
            status = flash_intf_target->program_page(current_write_block_addr, buf, current_write_block_size);
            flash_manager_printf("    intf->program_page(addr=0x%x, size=0x%x) ret=%i\r\n", current_write_block_addr, current_write_block_size, status);

            if (ERROR_SUCCESS != status) {
                state = STATE_ERROR;
                return status;
            }

            // Setup for next page
            memset(buf, 0xFF, current_write_block_size);
            buf_empty = true;
            current_write_block_addr += current_write_block_size;
        }

        // Check for end
        if (size <= 0) {
            break;
        }

        // Change sector if necessary
        if (addr >= current_sector_addr + current_sector_size) {
            status = setup_next_sector(addr);

            if (ERROR_SUCCESS != status) {
                state = STATE_ERROR;
                return status;
            }
        }

        // write buffer
        pos = addr - current_write_block_addr;
        size_left = current_write_block_size - pos;
        copy_size = MIN(size, size_left);
        memcpy(buf + pos, data, copy_size);
        buf_empty = copy_size == 0;
        // Update variables
        addr += copy_size;
        data += copy_size;
        size -= copy_size;
    }

    last_addr = addr;
    return status;
}
//查空
//读目标芯片中的数据与下载的数据，比较是否都为0xFF
error_t swd_prg_check_empty(void)
{
    error_t status = ERROR_SUCCESS;
    
    return status;
}

//读目标芯片中的数据与下载的数据，比较是否一致
error_t swd_prg_verify(void)
{
    error_t status = ERROR_SUCCESS;
//                uint8_t rb_buf[16];
//                uint32_t verify_size = MIN(write_size, sizeof(rb_buf));
//                if (!swd_read_memory(addr, rb_buf, verify_size)) {
//                    return ERROR_ALGO_DATA_SEQ;
//                }
//                if (memcmp(buf, rb_buf, verify_size) != 0) {
//                    return ERROR_WRITE;
//                }
//                addr += verify_size;
//                buf += verify_size;
//                size -= verify_size;
//                write_size -= verify_size;   
    return status;
}

