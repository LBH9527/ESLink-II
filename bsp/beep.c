#include "eslink_gpio.h"
#include "beep.h"

enum beep_state_t
{
    BEEP_ON,
    BEEP_MUTE,  
};
static beep_t beep;  

/********************************************************************************
*  函 数 名: BEEP_Start
*  功能说明: 启动蜂鸣音。
*  形    参：beep_time : 蜂鸣时间，单位10ms; 0 表示不鸣叫
*        stop_time : 停止时间，单位10ms; 0 表示持续鸣叫
*       cycle : 鸣叫次数， 0 表示持续鸣叫
*  返 回 值: 无
*******************************************************************************/
void beep_start(uint8_t beep_time, uint32_t stop_time, uint8_t cycle) 
{
  if (beep_time == 0)
  {
    return;
  }

  beep.beep_time = beep_time;
  beep.stop_time = stop_time;
  beep.count = 0;
  beep.state = 0;
  beep.enable = 1;  /* 设置完全局参数后再使能发声标志 */
     beep.cycle = cycle; 
  beep.cycle_count = 0;
  BEEP_ON();      /* 开始发声 */
}


void beep_stop(void)
{
  beep.enable = 0;

  if (beep.stop_time == 0)
  {
    BEEP_OFF();  /* 必须在清控制标志后再停止发声，避免停止后在中断中又开启 */
  }
}

void beep_key_press(void)
{
    beep_start(10, 10, 1);  /* 鸣叫100ms，停100ms， 1次 */
}
//烧录成功 蜂鸣
void beep_prog_success(void)
{
  beep_start(10, 10, 1);  /* 鸣叫100ms，停100ms， 1次 */
}
//烧录失败 蜂鸣
void beep_prog_fail(void)
{
  beep_start(10, 10, 2);  /* 鸣叫100ms，停100ms， 2次 */
}


void beep_scan(void)
{
  if ((beep.enable == 0) || (beep.stop_time == 0))
  {
    return;
  }

  if (beep.state == BEEP_ON)
  {
    if (beep.stop_time > 0)  /* 间断发声 */
    {
      if (++beep.count >= beep.beep_time)
      {
        BEEP_OFF();    /* 停止发声 */
        beep.count = 0;
        beep.state = BEEP_MUTE;
      }
    }
    else
    {
      ;  /* 不做任何处理，连续发声 */
    }
  }
  else if (beep.state == BEEP_MUTE)
  {   
    if (++beep.count >= beep.stop_time)
    {    
            /* 连续发声时，直到调用stop停止为止 */
            if (beep.cycle > 0)
            {
                if (++beep.cycle_count >= beep.cycle)
                {
                    /* 循环次数到，停止发声 */
                    beep.enable = 0;
                }

                if (beep.enable == 0)
                {
                    beep.stop_time = 0;
                    return;
                }
            }
      beep.count = 0;
      beep.state = BEEP_ON;

      BEEP_ON();      /* 开始发声 */
    }
  }
}


