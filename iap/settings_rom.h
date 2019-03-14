#ifndef _SETTINGS_ROM_H
#define _SETTINGS_ROM_H   

 
//跳转的APP 标志
#define UPDATE_BOOT_APP     0x00
#define UPDATE_OFFLINE_APP  0x4F46464c     //OFFL   ‘O' ‘F’ ‘F' ’L‘
#define UPDATE_LINK_APP     0x4c494E4B     //LINK   'L' ‘I' 'N' 'K'


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
