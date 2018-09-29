

#ifndef IAP_FLASH_INTF_H
#define IAP_FLASH_INTF_H
//#include "errno.h" 



uint32_t iap_Init(void) ;
uint32_t iap_erase_chip(uint32_t updt_start, uint32_t updt_size);
uint32_t iap_erase_sector(uint32_t adr);
uint32_t iap_flash_program(uint32_t adr, uint8_t *buf,  uint32_t sz);
uint32_t iap_flash_checksum( uint32_t addr, uint32_t size);

// void flash_test() ;

#endif
