#ifndef SPI_FLASH_FS_H
#define SPI_FLASH_FS_H

int mount_filesystem(void);
int ol_file_write(uint32_t addr, const uint8_t *buf, size_t size);
int ol_file_read(uint32_t addr, uint8_t *buf, size_t size);
int fs_cmp_data(uint32_t addr, const uint8_t *buf, uint32_t size);
int fs_file_find(void);





#endif
