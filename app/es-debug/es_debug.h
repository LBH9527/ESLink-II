#ifndef _COMMON_H
#define _COMMON_H

#include "eslink.h"
#include "eslink_config.h"

#define	ICD_SOFT_VER1		            0x01  //固件版本号SOFT_VER1.SOFT_VER2
#define	ICD_SOFT_VER2		            0x00



uint32_t debug_process_command(uint8_t *request, uint8_t *response)    ;

#endif





