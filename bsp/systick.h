/*******************************************************************************
*
*	模块名称 : 定时器模块
*	文件名称 : bsp_SysTick
*	版    本 : V1.0
*	说    明 : 头文件
*
*
*******************************************************************************/

#ifndef _SYSTICK_H
#define _SYSTICK_H

/* 提供给其他C文件调用的函数 */
void bsp_init_systick(void);    
void bsp_delay_ms( uint32_t Delay);
void bsp_delay_us(uint32_t n);  
uint32_t bsp_time_get(void);

#endif

