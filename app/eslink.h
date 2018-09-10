#ifndef __ESLINK_H__
#define __ESLINK_H__


#include "MK22f12810.h"             // Debug Unit Cortex-M Processor Header File
#include "es_common.h"
#include "error.h"
#include "spi_flash.h"
#include "gpio.h"
#include "eslink_config.h"
#include "eslink_def.h"
#include "eslink_spi_flash_addr.h"

#define ES_DEBUG   0

#ifdef ES_DEBUG
//#include "daplink_debug.h"
#define printf    debug_msg
#else
#define printf(...)
#endif

//包含es_link isp和icd中用到的数据结构，芯片内部flash中信息地址。spi_flash中信息地址

#ifdef __cplusplus
extern "C" {
#endif
    
  
    

//******************************************************************************  



//******************************************************************************  
//es_link 调用函数    

// Configurable delay for clock generation
#ifndef ES_DELAY_SLOW_CYCLES
#define ES_DELAY_SLOW_CYCLES       4U      // Number of cycles for one iteration
#endif
static __forceinline void ES_DELAY_SLOW (uint32_t delay) {
  uint32_t count;

  count = delay;
  while (--count);
}
/*
80Mhz:
ES_DELAY_SLOW(1)------>200ns
*/

// Fixed delay for fast clock generation
#ifndef ES_DELAY_FAST_CYCLES
#define ES_DELAY_FAST_CYCLES       0U      // Number of cycles: 0..3
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


//void es_flash_write(uint32_t addr, uint32_t size, uint32_t *buf);
//void es_flash_read(uint32_t addr, uint32_t size, uint8_t *buf);




uint32_t es_process_command(uint8_t *request, uint8_t *response);
    
#ifdef __cplusplus
}
#endif

#endif
