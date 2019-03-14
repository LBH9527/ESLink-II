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

//ESLink
#define ESLINK_PLUS_TYPE        0x00        //ESLINK 全功能
#define ESLINK_MINI_TYPE        0x01        //ESLINK MINI

//联机工程模式
#define ESLINK_ONLINE_MODE    0x6F6E6d64         //onlink mode  'o' 'n' 'm' 'd'
#define ESLINK_OFFLINE_MODE   0x6f666d64         //offline mode 'o' 'f' 'm' 'd'

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
60Mhz:    ES_DELAY_SLOW(1)--->0.05us  (50ns)
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

// LED state
typedef enum led_state {
    LED_OK = 0,
    LED_FAIL,
    LED_BUSY
} led_state_t;


void es_delay_us(uint32_t delay);
void es_delay_ms(uint32_t delay);  
uint8_t eslink_is_mini(void);
uint8_t eslink_is_offline_mode(void) ;
void es_set_trget_power(trget_power_t power);
void eslink_led_set(led_state_t state);
  
#ifdef __cplusplus
}
#endif

#endif
