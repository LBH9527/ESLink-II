
#ifndef MAIN_H
#define MAIN_H

#include "stdint.h"
#include "stdbool.h"
#include "target_config.h"

void USBD_SignalHandler(void);
void main_cdc_send_event(void);
void main_reset(void );
//void main_reset_target(uint8_t send_unique_id);
void gui_refresh(void);

uint8_t rtc_out_mode(uint8_t enable);

#endif
