#ifndef _CONFIG_ROM_SET_H
#define _CONFIG_ROM_SET_H

void config_rom_init(void) ;
uint32_t app_update_check(void);
uint8_t get_offline_info(uint8_t *data);
uint8_t set_offline_info(uint8_t *data);

uint8_t get_timing_info(uint8_t *data);
uint8_t set_timing_info_and_update(uint8_t *data);
uint8_t clear_timing_info_and_update(void);
uint8_t get_target_info(uint8_t *data);
#endif
