#ifndef __OFFLINE_FILE_H__
#define __OFFLINE_FILE_H__

#include "offline_def.h"



error_t ofl_init(void);

error_t ofl_file_open( char *path);
error_t ofl_file_write( uint8_t type, const uint8_t *buf, uint32_t size);
error_t ofl_file_read(uint8_t type, uint32_t addr, uint8_t *data, uint32_t len);
error_t delete_ofl_file(const char *name);
error_t get_all_ofl_file(void);
error_t get_ofl_file_info(uint8_t num, uint8_t *data);
error_t get_ofl_file_name (uint8_t num, char *data);
error_t get_ofl_file_num( uint8_t *data);
error_t get_ofl_file_checksum(uint8_t type, uint8_t *data);
#endif
