#ifndef _ISP_PROG_INTF_H_
#define _ISP_PROG_INTF_H_

#include "target_config.h"
#include "errno.h"
//isp编程接口
extern struct  es_prog_ops isp_prog_intf ;

#if ESLINK_RTC_ENABLE
error_t isp_read_rtc_info(uint32_t addr,  uint32_t *buf, uint32_t size);
error_t isp_erase_rtc_info(void);
error_t isp_program_rtc_info(uint32_t addr,  uint32_t *buf, uint32_t size);  
error_t isp_program_rtc_flash_info(void);
#endif

#endif
