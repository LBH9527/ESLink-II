#ifndef _OFFLINE_APP_H_
#define _OFFLINE_APP_H_

 #define PROG_MODE_CHECK_TIME        5
//脱机编程状态
typedef enum  {
    IN_MODE_CHECK = 0,
    OFL_PROG_ING,
    OUT_MODE_CHECK,
} ofl_prog_state_t;

void ofl_prog_init(void);
uint8_t ofl_in_prog_mode(void);
uint8_t ofl_prog(void);

#endif

