#ifndef __ISP_M601_H
#define __ISP_M601_H

#define ID_CHECK_CMD                0x699655FA 
#define ID_CHECK_VAL                0x4D60113C   

#define STATUS_CHECK_CMD            0xF0        //ISP状态读取
#define ISP_MODE_CMD                0xF1        //设置ISP模式
#define ENCRYPT_CHECK_CMD           0xF2       //加密字加载
#define UNLOCK_CHECK_VAL     		0xA0                                                //Unlock Success Value
#define ISP_MODE_CHECK_VAL       	0xA1                                                //ISP Mode Check Value

/***********************************接口访问************************************/
#define AREA_SET_CMD                0xE1            //主程序区/信息区选址命令
#define CODE_AREA_VAL               0x4B            //主程序区
#define INFO_AREA_VAL               0xBB            //信息区

#define ADDR_SET_CMD                0xE3            //地址缓冲区设置
#define DATA_WRITE_CMD              0xE4            //数据缓冲器设置 
#define INFO_WRITE_CMD              0xE8            //info数据缓冲器设置

#define FLASH_READ_CMD              0xE8            //flash数据bit31-0读取  
#define FLASH_READ_PLUS_CMD         0xEA            //flash数据bit31-0读取，完成后地址+4字节
#define FLASH_READ1_CMD             0xEC            //flash数据bit63-32读取  

/***********************************ISP模式************************************/
#define ERASE_CHECK_CMD             0xC0            //flash擦除完成判断 
#define ERASE_ING_VAL               0x96            //正在擦除中
#define ERASE_OK_VAL                0x69            //擦除完成

#define ERASE_M0_CMD                0xC1            //flash擦除
#define FLASH_MAIN_AREA             0xA0            //擦除时区域选择:主程序区
#define FLASH_INFO_AREA             0xA1            //擦除时区域选择:信息区0

#define ERASE_M1_CMD                0xC2            //主程序区内的未加密分区全擦

#define ERASE_M3_UNLOCK_CMD         0x5A69A53456785A    //info 1 区操作保护解码
#define ERASE_M3_CMD                0xC4            //info1 擦除

#define PROG_CHECK_CMD              0xC5            //编程完成后判断是否正确
#define PROG_CMD                    0xC6            //编程
#define PLUS_PROG_CMD               0xC7            //地址先+4字节，再进行编程
#define PROG_PLUS_CMD               0xC8            //先编程，再地址+4

#define PROG_ING_VAL                0x96            //正在编程中
#define PROG_OK_VAL                 0x87            //编程完成
#define PROG_FAIL_VAL               0x78            //编程失败

/***********************************CRC校验************************************/
#endif
