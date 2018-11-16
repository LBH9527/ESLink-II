#ifndef __BEEP_H
#define __BEEP_H

typedef struct 
{
	uint8_t enable;     //使能
	uint8_t state;      //状态
	uint16_t beep_time; //蜂鸣时间
	uint16_t stop_time; //停止时间
	uint16_t cycle;     //循环次数
    uint16_t cycle_count;  //循环次数计数
	uint16_t count;     //时间计数标志

}beep_t;

void beep_scan(void);
void beep_prog_success(void);
void beep_prog_fail(void);

#endif
