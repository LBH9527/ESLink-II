
#ifndef MAIN_H
#define MAIN_H

#include "stdint.h"
#include "stdbool.h"


void main_cdc_send_event(void);
void main_reset(void );
void main_reset_target(uint8_t send_unique_id);
void main_icd_debug_handle(void) ;
void USBD_SignalHandler(void);
#endif
