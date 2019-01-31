#ifndef _BOOTISP_HOST_H_
#define _BOOTISP_HOST_H_

#include "uart.h"

uint8_t bootisp_start(void);
uint8_t bootisp_check_empty(uint32_t addr, uint32_t size);
uint8_t bootisp_extended_erase(uint8_t *data, uint8_t size);
uint8_t bootisp_write_memory(uint32_t addr,  uint8_t *data, uint32_t size);
uint8_t bootisp_read_memory(uint32_t addr, uint8_t *data, uint32_t size);


#endif
