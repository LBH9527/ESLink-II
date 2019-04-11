/**
  **************************************************************************************
  * @file    bsp_systick.c
  * @brief   系统时钟
  *
  * @version V0.01
  * @data    4/3/2018
  * @author  Eastsoft AE Team
  * @note
  *
  * Copyright (C) 2018 Shanghai Eastsoft Microelectronics Co., Ltd. ALL rights reserved.
  *
  **************************************************************************************
  */
#include "es_common.h "
#include "cortex_m.h"
#include "clock_config.h"
#include "systick.h"

__IO int32_t run_time = 0;
static volatile uint32_t delay_count = 0;
static volatile uint8_t time_out_flag = 0;

/*******************************************************************************
*  函 数 名:
*  功能说明: 配置systick中断，并初始化软件定时器变量
*  形    参:  无
*  返 回 值: 无
*******************************************************************************/
void bsp_init_systick(void)
{
  SysTick_Config(SystemCoreClock / 1000);             //1ms
}
/*******************************************************************************
*    函 数 名: bsp_delay_us
*    功能说明: us级延迟。 必须在systick定时器启动后才能调用此函数。
*    形    参:  n : 延迟长度，单位1 us
*    返 回 值: 无
*******************************************************************************/
void bsp_delay_us(uint32_t n)
{
  uint32_t ticks;
  uint32_t told;
  uint32_t tnow;
  uint32_t tcnt = 0;
  uint32_t reload;

  reload = SysTick->LOAD;
  ticks = n * (SystemCoreClock / 1000000);  /* 需要的节拍数 */
  tcnt = 0;
  told = SysTick->VAL;                        /* 刚进入时的计数器值 */

  while (1)
  {
    tnow = SysTick->VAL;

    if (tnow != told)
    {
      /* SYSTICK是一个递减的计数器 */
      if (tnow < told)
      {
        tcnt += told - tnow;
      }
      /* 重新装载递减 */
      else
      {
        tcnt += reload - tnow + told;
      }

      told = tnow;

      /* 时间超过/等于要延迟的时间,则退出 */
      if (tcnt >= ticks)
      {
        break;
      }
    }
  }
}

/*******************************************************************************
*  函 数 名: SysTick_Handler
*  功能说明: SysTick中断服务程序
*  形    参:  无
*  返 回 值: 无
*******************************************************************************/

extern void main_10ms_task(void);
void SysTick_Handler(void)
{
  static uint8_t count = 0;

  if (run_time++ == 0x7FFFFFFF)
  {
    run_time = 0;
  }
  if (delay_count > 0)
  {
    if (--delay_count == 0)
    {
      time_out_flag = 1;
    }
  }

  if (++count >= 10)
  {
    count = 0;
    main_10ms_task();
  }

}

/*******************************************************************************
*  函 数 名: bsp_DelayMS
*  功能说明: ms级延迟，延迟精度为正负1ms
*  形    参:  n : 延迟长度，单位1 ms。 n 应大于2
*  返 回 值: 无
*******************************************************************************/
void bsp_delay_ms(uint32_t Delay)
{
  if (Delay == 0)
  {
    return;
  }
  else if (Delay == 1)
  {
    Delay = 2;
  }

  DISABLE_INT();        /* 关中断 */

  delay_count = Delay;
  time_out_flag = 0;

  ENABLE_INT();          /* 开中断 */

  while (1)
  {
//    bsp_Idle();        /* CPU空闲执行的操作， 见 bsp.c 和 bsp.h 文件 */

    /*
      等待延迟时间到
      注意：编译器认为 time_out_flag = 0，所以可能优化错误，因此 time_out_flag 变量必须申明为 volatile
    */
    if (time_out_flag == 1)
    {
      break;
    }
  }
}


int32_t bsp_time_get(void)
{
  int32_t runtime;

  DISABLE_INT();

  runtime = run_time;

  ENABLE_INT();

  return runtime;
}


int32_t bsp_check_run_time(int32_t last_time)
{
  int32_t now_time;
  int32_t time_diff;

  DISABLE_INT();        /* 关中断 */

  now_time = run_time;  /* 这个变量在Systick中断中被改写，因此需要关中断进行保护 */

  ENABLE_INT();         /* 开中断 */

  if (now_time >= last_time)
  {
    time_diff = now_time - last_time;
  }
  else
  {
    time_diff = 0x7FFFFFFF - last_time + now_time;
  }

  return time_diff;
}



