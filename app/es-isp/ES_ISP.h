#ifndef _ES_ISP_H
#define _ES_ISP_H

#include "eslink.h"

#ifdef __cplusplus
extern "C" {
#endif


///*
// *  用户代码类型（是否使能序列号）
// */
//typedef enum {
//    code_only,          //用户代码
//    serial_number,      //序列号代码使能。
//}isp_code_t;
/*
 *  序列号代码
 */
typedef struct {
    uint8_t enable_flag;     //序列号是否有效.
    uint8_t addr;           //序列号起始地址
    uint8_t size;           //序列号数据长度
    uint32_t data[2];       //序列号代码值
}isp_serial_number_t;
extern isp_serial_number_t isp_serial_number;

//isp操作错误地址和错误数据
 typedef struct {
    uint32_t addr;          //错误地址
    uint32_t data;          //错误数据
}isp_process_error_t;
extern isp_process_error_t isp_process_err;





/*******************************************************************************
  函数声明
*******************************************************************************/
//读芯片ID
error_t isp_read_chipid(uint32_t addr,uint32_t *buf);
//擦除
error_t isp_erase_chip (uint8_t erase_mode) ;
// 查空
error_t isp_check_empty(void);
//芯片配置字编程
error_t isp_programe_config(uint32_t addr, const uint8_t *data, uint32_t size) ;
//读芯片配置字
error_t isp_read_config(uint32_t addr, uint32_t size, uint32_t *buf);
//用户代码编程
error_t isp_program_hex( uint32_t start_addr, uint32_t image_size);
//读芯片信息
error_t isp_read_rom(uint32_t addr, uint32_t szie, uint32_t *buf);
//芯片加密
error_t isp_encrypt_chip(void);
//校验
error_t isp_verify(uint32_t start_addr, uint32_t image_size) ;


#ifdef __cplusplus
}
#endif

#endif
