#ifndef _OFFLINE_APP_H_
#define _OFFLINE_APP_H_

 #define PROG_MODE_CHECK_TIME        5
//脱机编程状态
typedef enum  {
    IN_MODE_CHECK = 0,
    OFL_PROG_ING,
    OUT_MODE_CHECK,
} ofl_prog_state_t;

//编程错误状态
typedef enum{
    OFL_SUCCESS,
    OFL_COUNT_FULL,         //烧录计数溢出
    OFL_PROG_FAIL,          //编程失败
} ofl_prog_error_t ;

error_t ofl_prog_init(void);
uint8_t ofl_in_prog_mode(void);
uint8_t ofl_out_prog_mode(void);
ofl_prog_error_t ofl_prog(void);
error_t update_ofl_serial_number(void);


#endif

