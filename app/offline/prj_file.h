#ifndef OFFLINE_PRJ_MANAGER_H_
#define OFFLINE_PRJ_MANAGER_H_

#include "stdint.h" 
#include "stdio.h" 
#include "error.h"
#include "offline_def.h"



//struct prj_ops
//{
//    /* common device interface */
//    error_t (*init) (struct prj_tbl* prj_table) ;
//    error_t (*open) ( struct prj_target *prj);
//    int (*write)(const struct prj_target *prj, uint32_t addr, const uint8_t *buf, size_t size);
//    int (*read)( struct prj_target *prj, uint32_t read_offset, uint8_t *buf, size_t size);

//} ;

int prj_file_init( void); 
int prj_file_get_table(struct prj_tbl *table);
int prj_file_find(const char *name);
int prj_file_open(struct prj_ctl_block *prj);
int prj_file_write(const struct prj_ctl_block *prj, uint32_t addr, const uint8_t *buf, size_t size);
int prj_file_read(struct  prj_ctl_block *prj, uint32_t read_offset, uint8_t *buf, size_t size);

#endif
