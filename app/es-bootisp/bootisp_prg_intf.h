#ifndef _BOOTISP_PRG_INTF_H_
#define _BOOTISP_PRG_INTF_H_

void ofl_bootisp_init(void);
error_t bootisp_erase_chip(uint8_t erase_mode);
error_t bootisp_program(uint32_t addr, uint8_t *data, uint32_t size);
error_t bootisp_check_empty(void);
error_t bootisp_verify(void);

#endif
