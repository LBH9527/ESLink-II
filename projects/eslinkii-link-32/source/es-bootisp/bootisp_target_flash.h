#ifndef _BOOTISP_TARGET_FLASH_H_
#define _BOOTISP_TARGET_FLASH_H_

#include "stdint.h" 

int bootisp_start(void);
int check_empty(uint32_t addr, uint32_t size);
int extended_erase(uint8_t *data, uint8_t size);
int write_memory(uint32_t addr,  uint8_t *data, uint8_t size);


#endif
