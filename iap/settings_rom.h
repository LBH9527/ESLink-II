#ifndef _SETTINGS_ROM_H
#define _SETTINGS_ROM_H   

//ESLink模式
#define ESLINK_MODE_COMPLETE    0x00        //ESLINK 全功能
#define ESLINK_MODE_MINI        0x01        //ESLINK MINI
 
//跳转的APP 标志
#define UPDATE_BOOT_APP     0x00
#define UPDATE_OFFLINE_APP  0x4F46464c     //OFFL   ‘O' ‘F’ ‘F' ’L‘
#define UPDATE_LINK_APP     0x4c494E4B     //LINK   'L' ‘I' 'N' 'K'
//联机工程模式
#define LINK_ONLINE_MODE    0x6F6E6d64         //onlink mode  'o' 'n' 'm' 'd'
#define LINK_OFFLINE_MODE   0x6f666d64         //offline mode 'o' 'f' 'm' 'd'

void settings_rom_init(void);
uint32_t get_timing_checksum(void);
uint32_t app_update_check(void);
uint8_t get_offline_info(uint8_t *data);
uint8_t set_offline_info(uint8_t *data); 
uint8_t get_timing_info(uint8_t *data);  
uint8_t set_timing_info(uint8_t *data); 
uint8_t get_target_info(uint8_t *data);
uint8_t get_hr_target_info( uint8_t *data);  
uint8_t set_hr_timing_info(uint8_t *data); 
uint8_t clear_timing_info(void);  
uint32_t get_update_app(void);
uint8_t set_app_update(uint32_t update_app);
uint32_t get_hardware_id(void);
uint32_t get_link_mode(void);
uint8_t set_link_mode(uint32_t mode); 
uint32_t get_offlink_app_version(void); 
uint32_t get_hardware_version(void) ;
uint32_t get_eslinkii_serial_number(void);
uint8_t set_eslinkii_serial_number(uint8_t *data);
#endif
