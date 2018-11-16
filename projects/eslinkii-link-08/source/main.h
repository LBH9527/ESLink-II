
#ifndef MAIN_H
#define MAIN_H

#include "stdint.h"
#include "stdbool.h"

// LED state
typedef enum main_led_state {
    MAIN_LED_DEF = 0,
    MAIN_LED_FLASH,
    MAIN_LED_FLASH_PERMANENT
} main_led_state_t;

void main_cdc_send_event(void);
void main_reset(void );
void main_reset_target(uint8_t send_unique_id);
void main_icd_debug_handle(void) ;
//void main_blink_cdc_led(main_led_state_t state);
#endif
