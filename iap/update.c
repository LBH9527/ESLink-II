#include "es_common.h" 
#include "cortex_m.h"
#include "eslink_addr.h" 
#include "errno.h" 
#include "update.h"
#include "config_rom_set.h"
#include "iap_flash_intf.h"

bootloader_context_t bl_ctx;
static bool flash_initialized = false;
static bootloader_context_t  upt_app;


static inline uint32_t test_range(const uint32_t test, const uint32_t min, const uint32_t max)
{
    return ((test < min) || (test > max)) ? 0 : 1;
}


static uint8_t check_app_validate(const uint8_t *buf)
{
    // test for known required NVIC entries
    //  00 is stack pointer (RAM address)
    //  04 is Reset vector  (FLASH address)
    //  08 NMI_Handler      (FLASH address)
    //  12 HardFault_Handler(FLASH address)
    uint32_t i = 4, nvic_val = 0;

    // test the initial SP value
    memcpy(&nvic_val, buf + 0, sizeof(nvic_val));

    if (0 == test_range(nvic_val, ESLINK_RAM_START, ESLINK_RAM_START+ESLINK_RAM_SIZE)) 
        return FALSE;  

    // Reset_Handler
    // NMI_Handler
    // HardFault_Handler
    for (; i <= 12; i += 4) 
    {
        memcpy(&nvic_val, buf + i, sizeof(nvic_val));   
        if (0 == test_range(nvic_val, bl_ctx.address, bl_ctx.address+bl_ctx.size)) 
            return FALSE;
    }

    return TRUE;
}
 

uint8_t stay_in_bootloader(void)
{
    uint32_t app_select;  
//    uint32_t check;
    
    app_select =  app_update_check();
    if( app_select ==  UPDATE_OFFLINE_APP)
    {
        bl_ctx.address = ESLINK_ROM_OFFLINE_START;
        bl_ctx.size = ESLINK_ROM_OFFLINE_SIZE;
        return FALSE; 
    }          
    else if ( app_select ==  UPDATE_LINK_APP)
    {
        bl_ctx.address = ESLINK_ROM_LINK_START;
        bl_ctx.size = ESLINK_ROM_LINK_SIZE;

//        upt_app.address = ESLINK_ROM_LINK_START;
//        upt_app.size = ESLINK_ROM_LINK_SIZE;        
//        get_update_app_checksum(&check);
        return FALSE;         
    }                
    return TRUE; 
}
   
static void jump_to_app(uint32_t sp, uint32_t pc)
{
    typedef void(*app_entry_t)(void);

    static uint32_t stack_pointer = 0;
    static uint32_t app_entry = 0;
    static app_entry_t jump_app = 0;

//    bl_deinit_interface();
    stack_pointer = sp;
    app_entry = pc;
    jump_app = (app_entry_t)app_entry;

    // Change MSP and PSP
    __set_MSP(stack_pointer);
    __set_PSP(stack_pointer);
    
    SCB->VTOR = SCB_VTOR_TBLOFF_Msk & bl_ctx.address;
    
    // Jump to application
    jump_app();

    // Should never reach here.
    __NOP();
}

static void application_run(void)
{
    uint32_t *vectorTable = (uint32_t*)bl_ctx.address;
    uint32_t sp = vectorTable[0];
    uint32_t pc = vectorTable[1];
    jump_to_app(sp, pc);
}

void application_check_and_run(void)
{
    if( check_app_validate((uint8_t *)bl_ctx.address ) == TRUE )  
    {
         application_run();
    }         
}

uint8_t update_app_init(uint32_t app_type)
{
    if(iap_Init() != 0)
        return 1;
    if( app_type ==  UPDATE_OFFLINE_APP)
    {
        upt_app.address = ESLINK_ROM_OFFLINE_START;
        upt_app.size = ESLINK_ROM_OFFLINE_SIZE;
    }          
    else if ( app_type ==  UPDATE_LINK_APP)
    {
        upt_app.address = ESLINK_ROM_LINK_START;
        upt_app.size = ESLINK_ROM_LINK_SIZE; 
    }     
    return TRUE;
    
}

uint8_t update_app_program(uint32_t app_type, uint32_t addr, uint8_t *data, uint32_t size )
{
    uint8_t status;
    
    if (!flash_initialized) 
    {
        update_app_init(app_type);
        status = iap_erase_chip(upt_app.address,upt_app.size); 
        if(status != 0)
             return 1;

        flash_initialized = true;
    }
    if (flash_initialized) 
    {
         status = iap_flash_program(  addr, data, size) ;
         if(status != 0)
             return 1;
    }  
    return TRUE;
}

void update_app_program_end(void)
{
    flash_initialized = false;
    
}
uint8_t get_update_app_checksum(uint32_t *checksum)
{

    *checksum = iap_flash_checksum( upt_app.address,  upt_app.size); 
    return TRUE;    
}

