#ifndef _SWD_PRG_INTF_H_
#define _SWD_PRG_INTF_H_


#include "swd_flash_blob.h"
#include "swd_target_config.h"
#include "swd_flash_intf.h"
#include "swd_target_config.h"

//void ofl_swd_init(target_cfg_t target_device_cfg);
error_t swd_prg_init(target_cfg_t *target_dev, uint32_t addr, const uint8_t *data, uint32_t size);
error_t swd_erase_chip(void);
error_t swd_prg_program(uint32_t addr, const uint8_t *data, uint32_t size);
error_t swd_prg_check_empty(void);
error_t swd_prg_verify(void);
    

#endif
