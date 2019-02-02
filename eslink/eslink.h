#ifndef __ESLINK_H__
#define __ESLINK_H__

#include "errno.h"
#include "MK22f12810.h"             // Debug Unit Cortex-M Processor Header File
#include "eslink_app_addr.h"
#include "eslink_gpio.h"
#include "eslink_config.h"
#include "eslink_def.h"
#include "es_burner.h"
#include "es_debug.h"
#include "es_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ES_DEBUG   0

#ifdef ES_DEBUG
//#include "daplink_debug.h"
#define printf    debug_msg
#else
#define printf      (...)
#endif

//******************************************************************************  
//es_link 调用函数    

// Configurable delay for clock generation
#ifndef ES_DELAY_SLOW_CYCLES
#define ES_DELAY_SLOW_CYCLES       3U      // Number of cycles for one iteration
#endif
static __forceinline void ES_DELAY_SLOW (uint32_t delay) {
  uint32_t count;

  count = delay;
  while (--count);
}
/*
60Mhz:    ES_DELAY_SLOW(1)--->500ns
ES_DELAY_SLOW_CYCLES = 2    es_delay_ms(1)------>1.5ms    
ES_DELAY_SLOW_CYCLES = 3    es_delay_ms(1)------>1ms     
ES_DELAY_SLOW_CYCLES = 4    es_delay_ms(1)------>750us
ES_DELAY_SLOW_CYCLES = 5    es_delay_ms(1)------>600us
ES_DELAY_SLOW_CYCLES = 6    es_delay_ms(1)------>500us
ES_DELAY_SLOW_CYCLES = 7    es_delay_ms(1)------>429us
ES_DELAY_SLOW_CYCLES = 8    es_delay_ms(1)------>375us
ES_DELAY_SLOW_CYCLES = 9    es_delay_ms(1)------>333us
ES_DELAY_SLOW_CYCLES = 10   es_delay_ms(1)------>300us

ES_DELAY_SLOW_CYCLES = 12   es_delay_ms(1)------>250us

ES_DELAY_SLOW_CYCLES = 14   es_delay_ms(1)------>215us
*/

// Fixed delay for fast clock generation
#ifndef ES_DELAY_FAST_CYCLES
#define ES_DELAY_FAST_CYCLES       2U      // Number of cycles: 0..3
#endif
static __forceinline void ES_DELAY_FAST (void) {
#if (ES_DELAY_FAST_CYCLES >= 1U)
  __nop();
#endif
#if (ES_DELAY_FAST_CYCLES >= 2U)
  __nop();
#endif
#if (ES_DELAY_FAST_CYCLES >= 3U)
  __nop();
#endif
}   

void es_delay_us(uint32_t delay);
void es_delay_ms(uint32_t delay);  
uint8_t eslink_is_mini(void);
uint8_t eslink_is_offline_mode(void) ;
void es_set_trget_power(trget_power_t power);
uint8_t ofl_start_in_low(void);
void serial_number_intercept_write(serial_number_t *sn, uint32_t addr, uint8_t *data, uint32_t size);
void eslink_set_target_hold_reset(void);
void eslink_set_target_reset_run(uint8_t delay_ms);
void eslink_set_target_power_reset(uint8_t delay_ms);   
#ifdef __cplusplus
}
#endif

#endif
