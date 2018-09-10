#ifndef __SWD_PRJ_DECODER_H__
#define __SWD_PRJ_DECODER_H__

#include "swd_pgr_intf.h"




//void ofl_swd_init(uint32_t size);
void ofl_set_swd_target_info(const uint8_t *data, target_cfg_t* target_flash);
void ofl_set_swd_timing(target_cfg_t* target_flash, uint32_t addr, const uint32_t *data, uint32_t size);
int ofl_swd_get_flash_algo( uint32_t addr, const uint8_t *data, uint32_t size );


#endif



