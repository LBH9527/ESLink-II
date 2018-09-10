#ifndef _HR7P169B_ICD_DEBUG_H
#define _HR7P169B_ICD_DEBUG_H



//==========================================
#define BreakPointNum 	0x08  //支持断点个数
#define STACK_NUM		8	  //堆栈级数



#define ICD_BREAKPOINT_NUM  8   //支持断点个数
typedef struct {
    uint16_t addr_buffer[ICD_BREAKPOINT_NUM];               //保存断点的地址
    uint8_t empty_flag;                                     //断点标志，是否存在断点。ENABLE没有断点， DISABLE 有断点
    uint8_t monitor_flag;                                   //是否需要上传监视数据
}icd_breakpoint_t;

//ICD物理层通讯接收应答位//physical layer
#define ICD_PL_READ_ACK     0		    	
#define ICD_PL_READ_NACK    1

//----------------------------------------------



#define BLANK_VALUE          0xFFFF
  
#define IC_ID_55             0xAA55

#define CODE_FRAME_LENTH     0x400
#define OPTION_FRAME_LENTH   0x200

//#define CONFIG_BYTES		0x24 //配置信息的个数（byte）

#define ENCYPT_CP0           0x55AA
#define ENCYPT_NO            0xFFFF

#define OFFLINE_EN           0xAA

//--------------------------------------------//
#define SERIAL_NO_EN         0x12
#define SERIAL_NO_DIS        0x00   

#define LAST_ONE_FRAME       0x00
#define NOT_LAST_ONE_FRAME   0x34

#define BURNER_BEEP_EN       0x00
#define BURNER_BEEP_DIS      0x56

//=================================调试命令码1：M300前的芯片
#define step_1640 				0x24
#define stepover_1640 			0x25
#define halt_1640				0x26
#define run_1640				0x27
#define set_reg_addr_1640   	0x10
#define wr_reg_data_1640		0x11
#define wr_pc_1640				0x12
#define set_breakpoint_1640 	0x13
#define unset_breakpoint_1640	0x14
#define set_stacklevel_1640 	0x15
#define set_condition_1640		0x16
#define set_status_bk_1640		0x17
#define set_abk_data_1640		0x18
#define set_rbk_addr_1640		0x19
#define set_rbk_data_1640		0x1a
#define set_dbk_level_1640		0x1b
#define set_dbk_data_1640		0x1c
#define set_pcbk_times_1640		0x1d
#define set_rbk_times_1640		0x1e
#define set_dbk_times_1640		0x1f
#define rd_reg_data_1640		0x34
#define rd_last_pc_1640			0x35
#define rd_current_pc_1640		0x36
#define rd_stacklevel_1640		0x37
#define rd_stack_1640			0x38
#define rd_stopwatch_h_1640     0x39
#define rd_stopwatch_l_1640     0x3a
/***************define reg type****************/
#define extend_reg_1640  0x0000
#define sram_reg_1640	 0x2000
#define stack_reg_1640   0x4000
#define a_reg_1640		 0x6000
#define b_reg_1640		 0x6001

//============================== 调试命令码2：M300后的芯片
#define SET_ADDR	0x33
#define WR_DATA		0x36
#define RD_DATA		0x66

#define ICD_STEP_CMD	0x53
#define STEP_OUT_CMD	0x56
#define STEP_OVER_CMD	0x5C
#define ICD_HALT_CMD	0x55
#define RUN_FLAG_CMD 	0x5A
#define HALT_CHECK_CMD	0x59
#define ICD_RESET_CMD   0x35

//调试寄存器地址
#define AREG_REG_ADDR    	    0x4000
#define PCR_REG_ADDR			0x4002
#define LSTPCR_REG_ADDR    	    0x4004
#define STKLV_REG_ADDR    	    0x4006
#define STK0_REG_ADDR   		0x4007
#define STK1_REG_ADDR   		0x4008
#define STK2_REG_ADDR   		0x4009
#define STK3_REG_ADDR   		0x400A
#define STK4_REG_ADDR   		0x400B
#define STK5_REG_ADDR   		0x400C
#define STK6_REG_ADDR   		0x400D
#define STK7_REG_ADDR   		0x400E

#define STPWHL_REG_ADDR   	    0x4100
#define STPWHH_REG_ADDR		    0x4101
#define BKS_REG_ADDR		   	0x4104
#define STBK_REG_ADDR		    0x4105
#define ABK_REG_ADDR	    	0x4106
#define RBKA_REG_ADDR	   	    0x4107
#define RBKD_REG_ADDR		    0x4108
#define RBKT_REG_ADDR	   	    0x4109
#define PCBKT_REG_ADDR		    0x410A

#define SPCBK_REG_ADDR   	    0x4110
#define CPCBK_REG_ADDR		    0x4111
#define CDBK_REG_ADDR    	    0x4112
#define CONFIGBIT  	0x7fff //ME片调试芯片使用

#define PSW         0xff84      //HR7P169B 程序状态寄存器地址

//
uint8_t icd_write_data_operation(uint16_t address, uint16_t write_data);
uint8_t icd_halt_check(void);
uint8_t emu_reset(void);
uint8_t emu_asm_step(void);
uint8_t emu_asm_step_out(void);
uint8_t emu_asm_step_over(void);

uint8_t emu_stop(void);
uint8_t emu_run(void);
uint8_t emu_C_step(uint8_t *wr_buf);
uint8_t emu_C_stepover(uint8_t *wr_buf)	;
uint8_t emu_set_breakpoint(uint8_t *buf);
uint8_t emu_clr_breakpoint(uint8_t *buf);
uint8_t emu_clr_all_breakpointer(void) ;

uint8_t emu_read_used_stack(uint8_t *rd_buf, uint16_t *read_len);
uint8_t emu_read_all_stack(uint8_t *rd_buf, uint16_t *read_len);
uint8_t emu_modify_PC(uint8_t *data_buf);
uint8_t emu_read_PC(uint8_t *buf);

uint8_t emu_read_ram(uint8_t *wr_buf, uint8_t *rd_buf, uint16_t *read_len)	;
uint8_t emu_import_ram(uint8_t *wr_buf, uint8_t *rd_buf, uint16_t *read_len);
uint8_t emu_modify_ram(uint8_t *wr_buf, uint8_t *rd_buf, uint16_t *read_len);
uint8_t emu_set_condition_breakpoint(uint8_t *wr_buf);


uint8_t update_chip_status(uint8_t *rd_buf, uint16_t *read_len)	;
//uint8_t halt_check(void);

uint8_t Ack_DownloadNoStopLab(void);

#endif






