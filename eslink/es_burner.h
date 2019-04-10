#ifndef __ES_BURNER_H__
#define __ES_BURNER_H__

#include "errno.h"
#include "es_burner_def.h"

uint32_t prog_process_command(uint8_t *request, uint8_t *response);
error_t es_burner_init(void);
#endif
