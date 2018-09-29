
#include "es_common.h"  
#include "eslink.h"
#include "config_rom_set.h"
#include "iap_flash_intf.h"



#define CFG_KEY             0x6b766c64
#define UPDATE_OFFLINE_APP  0x4F46464c     //OFFL
#define UPDATE_LINK_APP     0x4c494E4B     //LINK  
  
// WARNING - THIS STRUCTURE RESIDES IN NON-VOLATILE STORAGE!
// Be careful with changes:
// -Only add new members to end end of this structure
// -Do not change the order of members in this structure
// -Structure must remain packed so no padding bytes are added
typedef struct __attribute__((__packed__))  {
    uint32_t key;               // Magic key to indicate a valid record
    // Configurable values
    uint32_t app_update;    
    offline_info_t offline_info;
    timing_info_t timing_info;
    es_target_cfg target_dev;

    // Add new members here

} config_set_t;  

// Configuration ROM
static volatile const config_set_t config_rom __attribute__((section("cfgrom"), zero_init));
// Ram copy of ROM config
static config_set_t config_rom_copy;

// Configuration defaults in flash
static const config_set_t config_default = {
    .key = 0,
    .app_update = 0,
    .offline_info.version = 0,
    .timing_info.id = 0,
    .timing_info.version = 0,
    .timing_info.checksum = 0,
    .target_dev.code_start = 0,
    .target_dev.code_size = 0,
    .target_dev.config_word_start = 0,
    .target_dev.config_word_size = 0,
    .target_dev.encrypt_addr = 0,
    .target_dev.encrypt_value = 0,
    .target_dev.chipid_addr = 0,
    .target_dev.chipid_value = 0,
    
};  



// Buffer for data to flash
//0：OK  1：Failer
uint8_t config_rom_set(config_set_t *new_cfg)
{
    uint32_t status;
    uint32_t addr;
    uint8_t write_buffer[sizeof(config_set_t)];
    addr = (uint32_t)&config_rom;

    status = iap_erase_sector(addr);  
    if (status != 0) 
        return 1;
    memset(write_buffer, 0xFF, sizeof(write_buffer));
    memcpy(write_buffer, new_cfg, sizeof(config_set_t));  
    status = iap_flash_program(addr,write_buffer, sizeof(write_buffer) ); 
    if (0 != status) 
        return 1;   
    
    return TRUE;
}   

void config_rom_init(void)
{
    iap_Init();
    // Fill in the ram copy with the defaults
    memcpy(&config_rom_copy, &config_default, sizeof(config_rom_copy));

    // Read settings from flash if the key is valid
    if (CFG_KEY == config_rom.key) 
        memcpy(&config_rom_copy, (void *)&config_rom, sizeof(config_rom));        
    config_rom_copy.key = CFG_KEY;
}
uint32_t app_update_check(void)
{
    return  config_rom_copy.app_update;           
}

//获取脱机工程信息
//返回信息长度
uint8_t get_offline_info(uint8_t *data)
{
    uint8_t len = sizeof(offline_info_t);
    
    for(uint8_t i = 0; i< len; i++) 
        *(data++) =  *((uint8_t *)&config_rom_copy.offline_info + i);
    return len;
}
uint8_t set_offline_info(uint8_t *data)
{
    uint8_t len = sizeof(offline_info_t);
    
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.offline_info + i) = *(data++);
    if(config_rom_set(&config_rom_copy) != TRUE)
        return FALSE;
    return TRUE;
}
//获取时序信息
uint8_t get_timing_info(uint8_t *data)
{
    uint8_t len = 0;
    
    len = sizeof(timing_info_t);
    for(uint8_t i = 0; i< len; i++) 
        *(data++) =  *((uint8_t *)&config_rom_copy.timing_info + i);
    
    len = sizeof(es_target_cfg);
    for(uint8_t i = 0; i< len; i++) 
        *(data++) =  *((uint8_t *)&config_rom_copy.target_dev + i);
    
    return (sizeof(timing_info_t) + sizeof(es_target_cfg) );
}
//设置时序信息并更新APP
uint8_t set_timing_info_and_update(uint8_t *data)
{
    uint8_t len = 0;
    
    len = sizeof(timing_info_t);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.timing_info + i) = *(data++);
    
    len = sizeof(es_target_cfg);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.target_dev + i) = *(data++);
        
    config_rom_copy.app_update = UPDATE_LINK_APP;
    if(config_rom_set(&config_rom_copy) != TRUE)
        return FALSE;
    return TRUE;    
    
}
//清空时序信息和更新标志位
uint8_t clear_timing_info_and_update(void)
{
    uint8_t len = 0;
    
    len = sizeof(timing_info_t);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.timing_info + i) = 0;
    
    len = sizeof(es_target_cfg);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.target_dev + i) = 0;
        
    config_rom_copy.app_update = 0;
    if(config_rom_set(&config_rom_copy) != TRUE)
        return FALSE;
    return TRUE;   


}
//目标芯片信息
uint8_t get_target_info(uint8_t *data)
{
    uint8_t len = sizeof(es_target_cfg);
    for(uint8_t i = 0; i< len; i++) 
        *(data++) =  *((uint8_t *)&config_rom_copy.target_dev + i);
    return len;
}

//uint8_t set_target_info(uint8_t *data)
//{
//    uint8_t len = sizeof(es_target_cfg);
//    
//    for(uint8_t i = 0; i< len; i++) 
//        *((uint8_t *)&config_rom_copy.target_dev + i) = *(data++);
//    if(config_rom_set(&config_rom_copy) != TRUE)
//        return FALSE;
//    return TRUE;      
//    
//}
      


