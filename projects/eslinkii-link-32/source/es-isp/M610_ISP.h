#ifndef M620_ISP_H
#define M620_ISP_H

#define ID_CHECK_CMD                0xFA559669 
#define ID_CHECK_VAL                0x2D01614D  //0x4D61012D
#define STATUS_CHECK_CMD            0xF0       //Unlock Check Command
#define ISP_MODE_CMD                0xF1       //ISP Mode Set Command
#define ENCRYPT_CHECK_CMD           0xF2       //加密字加载
#define UNLOCK_CHECK_VAL     		0xA0                                                //Unlock Success Value
#define ISP_MODE_CHECK_VAL       	0xA1                                                //ISP Mode Check Value

#define AREA_SET_CMD                0xE0            //主程序区/信息区选址命令
#define CODE_AREA                   0x4B            //主程序区
#define INFO_AREA                   0xBB            //信息区
#define IFREN_CHECK                 0xE1            //主程序区/信息区检测命令 

#define ADDR_SET_CMD                0xE2            //地址缓冲区设置
#define ADDR_CHECK                  0xE3            //校验ADDR[16:0]地址缓冲器 

#define DATA_WRITE_CMD              0xE4            //数据缓冲器设置

#define DATA_READ                   0xE5            //数据缓冲器读取
  
//#define FLASH_READ_CMD              0xE6            //flash数据读取
#define FLASH_READ_PLUS_CMD         0xE6            //flash数据读取，完成后地址+4字节

#define INFO_DATA_WRITE_CMD         0xE8

//INF0:config; INF1:cali; INF2:encry; INF3:encry

#define ERASE_CHECK_CMD             0xC0        //flash擦除完成判断
#define ERASE_ING                   0x96 
#define ERASE_OK                    0x69 

#define ERASE_M0                    0xC1
#define ERASE_OP0                   0xC1
//#define ERASE_OP3                   0xC1
//#define ERASE_M1                    0xC2
//#define ERASE_INF0                  0xC3

#define FLASH_MAIN_AREA             0xA0            //擦除时区域选择:主程序区
#define OPTION_0_AREA               0xA1
//#define flash_INFO_3_AREA           0xA1            //擦除时区域选择:信息区0

//#define MAIN_PAGE_AREA              0xA2 
//#define ALL_ENCRY_AREA              0xA3 

#define ERASE_INF1_UNLOCK           0x5A69
#define ERASE_INF1                  0xC4

//-------------------------------------------//

#define PROG_CHECK_CMD                  0xC3

#define CHIP_PROG_CMD                   0xC4
#define PLUS_PROG_CMD                   0xC5            //地址先+4字节，再进行编程
//#define PROG_PLUS_CMD                   0xC8            //先编程，再地址+4

#define PROG_ING                    0x96 
#define PROG_OK                     0x87 
#define PROG_FAIL                   0x78 






#endif
