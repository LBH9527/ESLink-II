#ifndef _ISP_PROG_INTF_H_
#define _ISP_PROG_INTF_H_

#include "es_isp.h"
//isp编程接口
extern struct  es_prog_ops isp_prog_intf ;
//error_t isp_read_rtc_info( uint32_t addr, uint8_t *buf, uint32_t size);
//error_t isp_program_rtc_info( uint32_t addr, uint8_t *buf, uint32_t size);

#endif
