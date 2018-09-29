#ifndef _COMMON_H
#define _COMMON_H

#include "eslink.h"
#include "eslink_config.h"

#define	ICD_SOFT_VER1		            0x01  //固件版本号SOFT_VER1.SOFT_VER2
#define	ICD_SOFT_VER2		            0x00


/*******************************************************************************
结构和联合定义
*******************************************************************************/
//icd通信帧格式定义
#define ICD_COMM_PACKET_LEN                  0x500               //数据包长度
typedef struct{
    uint32_t  frame_head;           //帧头
	uint16_t  data_length;          //数据长度（接收、发送）
    uint8_t   device_addr;          //设备地址
	uint8_t   fun_code ;            //功能码                           
    
    uint8_t   device_state;         //设备状态
    uint16_t  checksum;             //校验和
//    uint8_t   data[9];              //数据区
//    uint8_t  *code_data;            //代码区，长度不定
//    uint8_t   *buffer;              //指向接收/发送数据数组    
    uint8_t   *wrbuf;            //指向接收数组
    uint8_t   *rdbuf;            //指向发送数组
    
}ICD_COMM_DATA_t;

//目标芯片状态
typedef enum {
    
    ICD_RESET,

    ICD_HALT,                    // Halt the target without resetting it
    ICD_RUN                      // Resume the target without resetting it    
} ICD_TARGET_STATE;

extern ICD_TARGET_STATE icd_target_state;
//icd 标志位设置
typedef struct{
    
    uint8_t monitor;            //监视标志位。标志位为SET时，上传芯片运行停止信息
                                //1、调试到断点位置时上报。2、
    uint8_t stop;               //接收到上位机停止命令标志位。
    
}ICD_DEBUG_FLAG_t;
extern ICD_DEBUG_FLAG_t icd_debug_flag;
//icd main 地址相关寄存器
typedef struct{
    uint16_t addr;              //main的入口地址
    uint16_t pointer_back;      //HR7P169B未用到，不清楚作用
    uint8_t set_point;     //部分芯片的main地址需要手动设置
    
    
}ICD_DEBUG_MAIN_t;
extern ICD_DEBUG_MAIN_t icd_main;

/*********************************ICD 错误状态码*******************************/
//设备工作状态码定义
typedef enum {
    ICD_ERR_NONE = 0,
    
    
    ICD_ERR_RUN_OK                 = 0xF4,
	ICD_ERR_LASTINDEX,
    
}icd_error_t;

////////////////////错误码///////////////////////
#define HEAD_ERROR						0xA0
#define DEV_ADD_ERROR					0xA1
#define FUN_ERROR						0xA2
#define CHECKSUM_ERROR					0xA3
#define BUSY_ERROR  					0xA4
#define CHIP_INFO_ERROR					0xA5
#define SCHEDULE_CHECKSUM_ERROR         0xA6
#define SEND_DATE_ADDR_ERROR            0xA7
#define SEND_DATE_LENTH_ERROR           0xA8
#define UNLOCK_ERROR                    0xA9
#define BLANK_ERROR                     0xAA
#define PGM_ERROR                       0xAB
#define OPTION_PGM_ERROR                0xAC 
#define OPTION_VERIFY_ERROR             0xAD                
#define ENCYPT_ERROR                    0xAE
#define USER_CODE_CHECKSUM_ERROR        0xB0
#define WRITE_EEPROM_ERROR              0xB1
#define CHIP_CONTROL_ERROR 			    0xB2
#define ERASE_ERROR     			    0xB3

#define OPTION_CODE_CHECKSUM_ERROR      0xAF
#define MEMORY_CODE_CHECKSUM_ERROR      0xB0


/*******************************************************************************
    通信协议相关宏定义
*******************************************************************************/
//通信协议常量定义
#define ICD_FRAME_HEAD                  0xABCDDCBA          //帧头
#define ICD_DEV_ADDR                    0x33                //设备地址
#define ICD_ACK_NORMAL_LENTH            0x14            //通用消息回复帧长度

#define ICD_DATA_OFFSET					0x08                //数据区在buffer中的偏移
#define ICD_DATA_LEN					0x09                //数据区长度
#define ICD_CODE_OFFSET					0x11                //代码区在buffer中的偏移





//ICD调试 通讯协议功能码ID
#define ID_ICD_HandShake         				0x01	//握手
#define READ_CHIP_INFO                  0x02	//读芯片信息
#define	DL_CHIP_INFO                    0x03	//芯片信息下载
#define	DL_SCHEDULE_HEX		            0x04	//时序下载
#define DL_SCHEDULE_HEX_END             0x05	//时序下载完成
#define Voltage_set						0x06    //调试电压设置
#define READ_VOLTAGE_SET_INFO			0x07	//读电压				
#define	Chipset							0x10	//芯片型号设置
#define	DownloadConfig					0x11	//配置下载
#define	PCdownload						0x12	//用户程序下载
#define	PC_downloadend   				0x13  	//用户程序下载完成
#define	Download_NoStopLab 				0x14	//发送库函数地址段
#define	ID_SET_MAIN_ADDR 				0x15	//发送main首地址（仅用于c程序调试）
#define Download_eep_data 				0x16	//

#define EMU_mrst	  					0x20
#define	ID_EMU_RUN	  						0x21    //全速运行
#define	ID_EMU_ASM_STEP					  	0x22    //汇编单步
#define	ID_EMU_ASM_STEP_OVER				  	0x23
#define EMU_halt					  	0X24
#define C_step							0x25
#define C_stepover						0x26
#define EMU_stepout		 				0x27
#define	SetintPointer					0x28
#define ClrintPointer					0x29
#define	Delallbreakpointer				0x2A
#define Set_bk_condition  				0x2B
#define Clr_bk_condition				0x2C
#define	Delall_bk_condition				0x2D

#define ReadAllRam						0x30
//#define	Readvariable				  	0x31
#define	ReadRam	  						0x32
#define	Readextendram 					0x33
#define	ReadPc 							0x35
#define	ReadUsedStack					0x36	//read NO.n stackcontext
#define ID_ICD_DisplayAllStack					0x37
#define Rd_data_stack				    0x38
#define Rd_eep_data    				    0x39
#define Resend_ack						0x3A  

#define Writeram    					0x40
#define ModifycunrrentPc  				0x41
#define Modify_eep_data  				0x44
#define Wr_data_stack			        0x45
#define Modifyextendram					0x46
#define Import_Various_RAM				0x47
//#define CLR_COMMON_RAM				    0x48
#define UpdateChipStatus                0x70










//大端模式
#define ICD_LEN(len, a, b) \
                len =  ((uint16_t)(a) & 0xff) << 8) | \
                         (uint16_t)((b) & 0xff)  

#define ICD_ADDR(addr, a, b, c, d) \
                addr =  ((uint32_t)((a) & 0xff) << 24) | \
                         ((uint32_t)((b) & 0xff) << 16) | \
                         ((uint32_t)((c) & 0xff) << 8)  | \
                          (uint32_t)((d) & 0xff)

#define COMBUFFERLEN 0x500//3200//2100	//发送缓冲区最大长度(8k)
#define LARGESTBUF 0x500//1600 	//接收缓冲区的最大长度(270 line ) 



////////////////////////////////////
//union two8_are16{
//	struct bb{
//		INT8 high8bit;
//		INT8 low8bit;
//	}BB;
//	INT16 the16Bit;
//};
////////////////////////////////////
//union two16_are32{
//	struct ww{
//		INT16 high16bit;
//		INT16 low16bit;
//	}WW;
//	INT32 the32Bit;
//};
////////////////////////////////////
//struct _sendBack{
//	uint8_t next;
//	uint16_t curAddress;
//	uint16_t lastAddress;
//};

//struct _sysSta{
//	enum system_state{		//system state
//		Sys_Reset,          
//		Sys_Monitor,        //监视状态
//		Sys_Run,            //系统处于运行状态
//		Sys_Halt            //停止
//	}sysSta;
//	struct _sendBack ROMbackData;
//	uint8_t sendSFRFinish;
//};
////////////////////////////////////
//struct __serCtrl{
//	uint8_t serTimeScope;	//to control series byte interval time  
//	uint8_t hadOne;
//};
//////////////////04 emulator//////////////
//union debug_signal{
//	struct ctrl{
//        uint8_t 	lowbit:1;	//signals associate with debug
//        uint8_t 	high_bit:6;
//        uint8_t  	high7bit:1; 
//   
//	}ctrl;
//	uint8_t ctrl_byte;
//};


//////////////////////与PC通讯协议中的常量///////////////////////
#define	HEAD0_N							0x00
#define	HEAD1_N							0x01
#define	HEAD2_N							0x02
#define	HEAD3_N							0x03
#define	FRAME_LEN_H						0x04
#define	FRAME_LEN_L						0x05
#define	DEV_ADD_N						0x06
#define	FUN_N							0x07
#define	DATA1_N							0x08
#define	DATA2_N							0x09
#define	DATA3_N							0x0A
#define	DATA4_N							0x0B
#define	DATA5_N							0x0C
#define	DATA6_N							0x0D
#define	DATA7_N							0x0E
#define	DATA8_N							0x0F
#define	DATA9_N							0x10
#define CODE_START_N					0x11

#define HEAD0_D							0xAB
#define HEAD1_D							0xCD
#define HEAD2_D							0xDC
#define HEAD3_D							0xBA
#define DEV_ADD							0x33
#define NORMAL_LENTH					0x14
#define READ_PROGRAM_LENTH              0x0414
#define READ_OPTION_LENTH               0x0214





//----------------------------------------------/Burner

//#define HARDWARE_SELFTEST_FAIL          0xB4    //硬件自检失败

//#define ENCAPSULATION_ERROR             0xB5    //封装错误
//#define SERIAL_CODE_CHECKSUM_ERROR      0xB6    //序列号代码校验和错
//#define PGM_VOLTAGE_ERROR               0xB7    //编程电压错误

//#define PIN_DETECT_ERROR                0xB8    //引脚检测失败
//#define CHIP_ID_NOT_MATCH               0xB9
//#define OPTION_NOT_MATCH                0xBA    //与界面设置不匹配 
//#define NOT_IN_MODE                     0xBB    //没有进模式

//#define RD_EE_ERR                       0xBC 
//#define WR_EE_ERR                       0xBD

//#define VERIFY_ERROR                    0xBE
//#define EE_CODE_VERIFY_ERROR            0xBF

//#define POWER_ERR                       0xC0

//#define ICD_RUN_OK                          0xF4




///////////////////////////////////////////////////////////////////

//#define	Burner_ERASE_ALL                0x50
//#define Burner_BLANK                    0x51
//#define Burner_PROGRAM                  0x52
//#define Burner_READ_MEN                 0x53
//#define Burner_READ_END                 0x54
//#define Burner_WRITE_CFG                0x55
//#define Burner_ENCRYPT                  0x56
//#define Burner_READ_CFG                 0x57
//#define Burner_VERIFY                   0x5A

//#define Burner_DL_CODE_START            0x5B
//#define Burner_DL_CODE_END              0x5C
//#define Burner_DL_OPTION                0x5D
//#define Burner_READ_CHK                 0x5E
//#define Burner_DL_SERIAL                0x5F

//#define Burner_ERASE_EE                 0x60//单独擦EE
//#define Burner_WR_EE                    0x61//单独写EE
//#define Burner_VERIFY_EE                0x62//单独校验EE
//#define Burner_V_RESULT_EE              0x63//单独校验EE后的结果确认
//#define Burner_RD_EE                    0x64//单独读EE

//#define Burner_DETECT_POWER             0x65

//#define OFFLINE_PGM                     0x66
//#define Burner_PIN_OK_CHK               0x67
//#define Burner_PIN_NO_CHK               0x68

//#define Burner_READ_chipID              0x6A
//#define Burner_READ_chipCHKSUM			0x6B
/////////////////////////////////////////////////////////////////////

//#define UpdateChipStatus 			    0x70 //pc received ChipStatus

//#define	BOOTLOAD				        0x99
///////////////////////////////////////////////////////////////////

uint32_t debug_process_command(uint8_t *request, uint8_t *response)    ;

#endif





