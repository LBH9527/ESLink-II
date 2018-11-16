#ifndef _COMMON_H
#define _COMMON_H

#include "es_debug_def.h"

#define ESLINK_ICD_DEBUG_VERSION       0x0100       //icd调试  
void debug_process_handle(void);
uint32_t debug_process_command(uint8_t *request, uint8_t *response)    ;

#endif





