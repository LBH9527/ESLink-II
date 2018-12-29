#ifndef __ISP_M939_H
#define __ISP_M939_H


#define STATUS_CHECK_CMD            0xF0        //ISP状态读取
#define ISP_MODE_CMD                0xF1        //设置ISP模式
#define ENCRYPT_CHECK_CMD           0xFF       //加密字加载
#define UNLOCK_CHECK_VAL     		0xA0                                                //Unlock Success Value
#define ISP_MODE_CHECK_VAL       	0xA1                                                //ISP Mode Check Value

//接口访问
#define AREA_SET_CMD                0xE1            //主程序区/信息区选址命令
#define CODE_AREA_VAL               0x4B            //主程序区
#define INFO_AREA_VAL               0xBB            //信息区

#define ADDR_SET_CMD                0xE3            //地址缓冲区设置
#define DATA0_WRITE_CMD             0xE5            //数据缓冲器0设置
#define DATA1_WRITE_CMD             0xE7            //数据缓冲器1设置
#define INFO_WRITE_CMD              0xEF            //info数据缓冲器设置

#define FLASH_READ_CMD              0xE8            //flash数据bit31-0读取  
#define FLASH_READ_PLUS_CMD         0xEA            //flash数据bit31-0读取，完成后地址+8字节
#define FLASH_READ1_CMD             0xEC            //flash数据bit63-32读取  

//ISP模式
#define UNLOCK_SET_CMD              0xB1            //info0区操作保护解锁设置
#define ERASE_CHECK_CMD             0xC0            //flash擦除完成判断 
#define ERASE_ING_VAL               0x96            //正在擦除中
#define ERASE_OK_VAL                0x69            //擦除完成

#define ERASE_M0_CMD                0xC1            //flash擦除
#define FLASH_MAIN_AREA             0xF0            //擦除时区域选择:主程序区

#define ERASE_M1_CMD                0xC2            //info 0擦除
#define INFO_0_AREA                 0xE1            //擦除时区域选择:信息区0

#define ERASE_M2_CMD                0xC3            //info 擦除
#define INFO_1_AREA                 0xD2            //擦除时区域选择:信息区1

#define ERASE_M3_CMD                0xC4            //info 擦除
#define INFO_2_AREA                 0xC3            //擦除时区域选择:信息区2

#define ERASE_M4_CMD                0xC5            //info 擦除
#define INFO_3_AREA                 0xB4            //擦除时区域选择:信息区3

#define ERASE_M5_CMD                0xC6            //info 擦除
#define INFO_4_AREA                 0xA5            //擦除时区域选择:信息区4
#define INFO_5_AREA                 0x96            //擦除时区域选择:信息区5
#define INFO_6_AREA                 0x87            //擦除时区域选择:信息区6
#define INFO_7_AREA                 0x78            //擦除时区域选择:信息区7


#define PROG_CHECK_CMD              0xC8            //编程完成后判断是否正确
#define PROG_CMD                    0xC9            //编程
#define PLUS_PROG_CMD               0xCA            //地址先+4字节，再进行编程
#define PROG_PLUS_CMD               0xCB            //先编程，再地址+4

#define PROG_ING_VAL                0x96            //正在编程中
#define PROG_OK_VAL                 0x87            //编程完成
#define PROG_FAIL_VAL               0x78            //编程失败



#endif
