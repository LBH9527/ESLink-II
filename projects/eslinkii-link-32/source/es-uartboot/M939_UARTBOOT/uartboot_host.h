#ifndef _UARTBOOT_HOST_H_
#define _UARTBOOT_HOST_H_

#include "uart.h"

uint8_t uartboot_start(void);
uint8_t uartboot_check_empty(uint32_t addr, uint32_t size);
uint8_t uartboot_extended_erase(uint8_t *data, uint8_t size);
uint8_t uartboot_write_memory(uint32_t addr,  uint8_t *data, uint32_t size);
uint8_t uartboot_read_memory(uint32_t addr, uint8_t *data, uint32_t size);


#endif
