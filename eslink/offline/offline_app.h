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
    
    OFL_ERR_ENTRY_MODE,         //进模式失败
    OFL_ERR_CHIPID_CHECK,       //ID检测失败
    OFL_ERR_ERASE,              //擦除失败
    OFL_ERR_CHECK_EMPTY,        //查空
    OFL_ERR_PROG,               //编程失败
    OFL_ERR_VERIFY,             //校验
    OFL_ERR_ENCRYPT,            //加密
    OFL_ERR_COUNT_FULL,         //烧录计数溢出
    OFL_ERR_PROG_INTF,          //编程接口设置失败
    
} ofl_error_t ;

#define OFFLINE_PROG_PLUS_MODE          0x00    //
#define OFFLINE_PROG_MINI_MODE          0x01    //
#define OFFLINE_PROG_MINI_DEFAULT_INTF   (PRG_INTF_ISP)           //

ofl_error_t ofl_prog_init(uint8_t mode);
uint8_t ofl_in_prog_mode(void);
uint8_t ofl_out_prog_mode(void);
ofl_error_t ofl_prog(void);
ofl_error_t ofl_mini_prog(void);
ofl_error_t update_ofl_serial_number(void);
void ofl_prog_handle(void);
void mini_ofl_prog_handle(void);
#endif

