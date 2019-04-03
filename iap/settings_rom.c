
#include "es_common.h"  
#include "eslink.h"
#include "settings_rom.h"
#include "iap_flash_intf.h"

#define CFG_KEY             0x6b766c64  
//上位机下发的时序、目标芯片等信息
typedef struct __attribute__((__packed__))  {
    uint32_t key;               // Magic key to indicate a valid record
    // Configurable values
    offline_info_t offline_info;    //脱机版本信息
    timing_info_t timing_info;      //时序信息
    es_target_cfg target_dev;       //32位机目标芯片信息
    hr_target_cfg hr_dev;           //8位机芯片信息
} config_set_t;  

// Configuration ROM
static volatile const config_set_t config_rom __attribute__((section("cfgrom"), zero_init));
// Ram copy of ROM config
static config_set_t config_rom_copy;

// Configuration defaults in flash
static const config_set_t config_default = {
    .key = 0,
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
    .hr_dev.version = 0,        //版本号
    .hr_dev.checksum = 0,       //校验和
    .hr_dev.reserved1 = 0,      //保留
    .hr_dev.timing_id = 0,      /*时序号*/
    .hr_dev.reset_voltage = 0,  //复位电平
    .hr_dev.chipid  = 0,         //芯片ID
    .hr_dev.reserved2 = 0,
    .hr_dev.prog_id = 0,             //编程时序号
    .hr_dev.code_start = 0,     //主程序区起始地址
    .hr_dev.code_size = 0,      //主程序区容量
    
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
    if (status != TRUE) 
        return FALSE;
    memset(write_buffer, 0xFF, sizeof(write_buffer));
    memcpy(write_buffer, new_cfg, sizeof(config_set_t));  
    status = iap_flash_program(addr,write_buffer, sizeof(write_buffer) ); 
    if (status != TRUE) 
        return FALSE;
    
    return TRUE;
} 

uint32_t get_timing_checksum(void)
{
      return  config_rom_copy.timing_info.checksum;
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

//32位获取时序信息
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


//目标芯片信息
uint8_t get_target_info(uint8_t *data)
{
    uint8_t len = sizeof(es_target_cfg);
    for(uint8_t i = 0; i< len; i++) 
        *(data++) =  *((uint8_t *)&config_rom_copy.target_dev + i);
    return len;
}
//8位机芯片信息
uint8_t get_hr_target_info( uint8_t *data)
{
    uint8_t len = sizeof(hr_target_cfg);
    for(uint8_t i = 0; i< len; i++) 
        *(data++) =  *((uint8_t *)&config_rom_copy.hr_dev + i);
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

uint8_t set_offline_info(uint8_t *data)
{
    uint8_t len = sizeof(offline_info_t);
    
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.offline_info + i) = *(data++);
    if(config_rom_set(&config_rom_copy) != TRUE)
        return FALSE;
    return TRUE;
}


//设置32位时序信息
uint8_t set_timing_info(uint8_t *data)
{
    uint8_t len = 0;
    
    len = sizeof(timing_info_t);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.timing_info + i) = *(data++);
    
    len = sizeof(es_target_cfg);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.target_dev + i) = *(data++);          
    
    if(config_rom_set(&config_rom_copy) != TRUE)
        return FALSE;
    
    return TRUE;    
    
}
uint8_t set_hr_timing_info(uint8_t *data)
{
    uint8_t len = sizeof(hr_target_cfg);
    
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.hr_dev + i) = *(data++);
        
    if(config_rom_set(&config_rom_copy) != TRUE)
        return FALSE;
    return TRUE;

}
//清空时序信息和更新标志位
uint8_t clear_timing_info(void)
{
    uint8_t len = 0;
    
    len = sizeof(timing_info_t);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.timing_info + i) = 0;
    
    len = sizeof(es_target_cfg);
    for(uint8_t i = 0; i< len; i++) 
        *((uint8_t *)&config_rom_copy.target_dev + i) = 0;
        
    if(config_rom_set(&config_rom_copy) != TRUE)
        return FALSE;
    return TRUE;   


}

typedef struct __attribute__((__packed__))  {
    uint32_t key;               // Magic key to indicate a valid record
    uint32_t serial_num;       //产品序列号
    // Configurable values
    uint32_t app_update;       //app 跳转标志位
    uint32_t link_mode;          //脱机/联机模式标志位

} eslink_info_set_t;  
// Configuration ROM
static volatile const eslink_info_set_t info_rom __attribute__((section("inforom"), zero_init));
// Ram copy of ROM info
static eslink_info_set_t info_rom_copy;

// info defaults in flash
static const eslink_info_set_t info_default = {
    .key = 0,
    .serial_num = 0,
    .app_update = 0,
    .link_mode = 0,    
};  
 
/*******************************************************************************
*  函 数 名: info_rom_set
*  功能说明: 保存info区的参数
*  形    参: 
*  返 回 值: 无
*******************************************************************************/
// Buffer for data to flash
//0：OK  1：Failer
uint8_t info_rom_set(eslink_info_set_t *new_cfg)
{
    uint32_t status;
    uint32_t addr;
    uint8_t write_buffer[sizeof(eslink_info_set_t)];
    addr = (uint32_t)&info_rom;

    status = iap_erase_sector(addr);  
    if (status != TRUE) 
        return FALSE;
    memset(write_buffer, 0xFF, sizeof(write_buffer));
    memcpy(write_buffer, new_cfg, sizeof(eslink_info_set_t));  
    status = iap_flash_program(addr,write_buffer, sizeof(write_buffer) ); 
    if (status != TRUE) 
        return FALSE;  
    
    return TRUE;
}  
/*******************************************************************************
*  函 数 名: get_update_app
*  功能说明: 获取更新标志
*  形    参: 
*  返 回 值: 无
*******************************************************************************/
uint32_t get_update_app(void)
{
    return  info_rom_copy.app_update;           
} 


//更新app 标志
//UPDATE_LINK_APP    UPDATE_OFFLINE_APP
uint8_t set_app_update(uint32_t update_app)
{
    info_rom_copy.app_update = update_app;  
    if(info_rom_set(&info_rom_copy) != TRUE)
        return FALSE;   
    return TRUE;    
}
//LINK ON OFFLINE MODE
uint32_t get_link_mode(void)
{
    return info_rom_copy.link_mode;
}
//设置联机/脱机 模式
uint8_t set_link_mode(uint32_t mode)
{
    info_rom_copy.link_mode = mode;
    if(info_rom_set(&info_rom_copy) != TRUE)
        return FALSE;
    return TRUE;  
}

 //设置ESLink II产品序列号
uint8_t set_eslinkii_serial_number(uint8_t *data)
{     
    info_rom_copy.serial_num = (*(data+0) <<  0) |
                                (*(data+1) <<  8) |
                                (*(data+2) << 16) |
                                (*(data+3) << 24);
    if(info_rom_set(&info_rom_copy) != TRUE)
        return FALSE;
    return TRUE;
}
//获取ESLink II产品序列号
uint32_t get_eslinkii_serial_number(void)
{
    return info_rom_copy.serial_num;  
}

/*******************************************************************************
*  函 数 名: settings_rom_init
*  功能说明: info区的参数 初始化
*  形    参: 
*  返 回 值: 无
*******************************************************************************/
void settings_rom_init(void)
{
    iap_Init();
    // Fill in the ram copy with the defaults
    memcpy(&config_rom_copy, &config_default, sizeof(config_rom_copy));
    memcpy(&info_rom_copy, &info_default, sizeof(info_rom_copy));
    

    // Read settings from flash if the key is valid
    if (CFG_KEY == config_rom.key) 
        memcpy(&config_rom_copy, (void *)&config_rom, sizeof(config_rom));        
    config_rom_copy.key = CFG_KEY;     
    
    // Read settings from flash if the key is valid
    if (CFG_KEY == info_rom.key) 
        memcpy(&info_rom_copy, (void *)&info_rom, sizeof(info_rom));        
    info_rom_copy.key = CFG_KEY;   
//    info_rom_copy.hw_version =  ESLINK_VERSION;
}
      

 
 //固件版本号定义在固件升级程序中的中断向量表中。
#define VERSION_INFO_OFFSET         0x20
//固件版本号
uint32_t get_offlink_app_version(void)
{
    uint32_t ofl_version = *((uint32_t *)(ESLINK_ROM_OFFLINE_START + VERSION_INFO_OFFSET)) ;
    if(ofl_version == 0xffffffff)
         ofl_version = 0;
    return ofl_version;
} 

//获取硬件版本
uint32_t get_hardware_version(void)
{
    uint32_t hw_version = *((uint32_t *)(ESLINK_ROM_BL_START + VERSION_INFO_OFFSET)) ;
    if(hw_version == 0xffffffff)
         hw_version = 0;
    return hw_version;  
}

