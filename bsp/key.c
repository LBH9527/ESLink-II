/*
*********************************************************************************************************
*
*	模块名称 : 独立按键驱动模块
*	文件名称 : bsp_key.c
*	说    明 : 扫描独立按键，具有软件滤波机制，具有按键FIFO。可以检测如下事件：
*				(1) 按键按下
*				(2) 按键弹起
*				(3) 长按键
*				(4) 长按时自动连发
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2016-08-02	9527	 按键
*
*
*********************************************************************************************************
*/

#include "key.h"
#include "circ_buf.h"
#include "gpio.h"

#define BUFFER_SIZE   10
circ_buf_t key_buffer;
uint8_t key_buffer_data[BUFFER_SIZE];
static key_param_t key_param[KEY_COUNT];      //按键参数

/*******************************************************************************
*函数名：Key_InitVar()
* 描述 ：初始化按键变量
* 输入 ：
* 输出 ：
*******************************************************************************/
static void Key_InitVar(void)
{
    uint8_t i;
    
    /* 给每个按键结构体成员变量赋一组缺省值 */
    for (i=0; i<KEY_COUNT; i++)
    {
		key_param[i].FilterTime = KEY_DOWN_PERIOD;
        key_param[i].LongTime = KEY_LONG_PERIOD;       /* 长按时间 0 表示不检测长按键事件 */       
        key_param[i].Count = 0;
        key_param[i].RepeatTime = KEY_REPEAT_PERIOD;  /* 按键连发的速度，0表示不支持连发，5表示每个50ms自动发送键值*/
        key_param[i].state = STATE_KEY_UP;
    }

    /* 如果需要单独更改某个按键的参数，可以在此单独重新赋值 ，比如长按键检测等*/
    
    key_param[KID_K0].key_down_value = KEY0_DOWN;
    key_param[KID_K0].key_up_value = KEY0_UP;
    key_param[KID_K0].key_long_value = KEY0_LONG;

    key_param[KID_K1].key_down_value = KEY1_DOWN;
    key_param[KID_K1].key_up_value = KEY1_UP;
    key_param[KID_K1].key_long_value = KEY1_LONG;

    key_param[KID_K2].key_down_value = KEY2_DOWN;
    key_param[KID_K2].key_up_value = KEY2_UP;
    key_param[KID_K2].key_long_value = KEY2_LONG;

    /* 判断按键按下的函数 */
    key_param[KID_K0].IsKeyDownFunc = gpio_key0_down;
    key_param[KID_K1].IsKeyDownFunc = gpio_key1_down;
    key_param[KID_K2].IsKeyDownFunc = gpio_key2_down;

	
}
void key_clear_buffers(void)
{
    circ_buf_init(&key_buffer, key_buffer_data, sizeof(key_buffer_data));
}

uint8_t key_write_data(uint8_t *key_value)
{
	uint8_t cnt;
    
    cnt = circ_buf_write(&key_buffer, key_value, 1);
    
    return cnt;
}

uint8_t key_read_data(uint8_t *key_value)
{
    return circ_buf_read(&key_buffer, key_value, 1);
}


/*******************************************************************************
*	函 数 名: key_init
*	功能说明: 初始化按键.
*	形    参: 无
*	返 回 值: 无
*******************************************************************************/
void key_init(void)
{
	Key_InitVar();		/* 初始化按键变量 */
    key_clear_buffers();
}


/*******************************************************************************
*	函 数 名: bsp_detect_key()
*	功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*	形    参:  按键结构变量指针
*	返 回 值: 无
*******************************************************************************/
static void bsp_detect_key(uint8_t i)
{
    key_param_t *key;

    key = &key_param[i];
    switch (key->state)
    {
        case STATE_KEY_UP:
            if (key->IsKeyDownFunc())
            {
                 key->state = STATE_KEY_DEBOUNCE;
                 key->Count = 0;
            }
            break;
        case STATE_KEY_DEBOUNCE:            
            if (key->IsKeyDownFunc())         
            {                
                key->state = STATE_KEY_DOWN;    
				key->Count = 0;				
            }            
            else            
            {                
               key->state = STATE_KEY_UP;            
            }            
            break;
        case STATE_KEY_DOWN:            
            if (key->IsKeyDownFunc())  
            {                
                key->Count++;                            
                if (key->Count >= key->FilterTime)             
                {                   
                    /* 100ms 发送按钮按下的消息 */                    
                    key_write_data(&(key->key_down_value));
					if (key->LongTime > 0)    	 	//检测长按键
					{
						key->state = STATE_KEY_LONG;   
					}
					else							//不检测长按键
					{
						key->state = STATE_KEY_RELEASE;   
					}                                 
                }            
            }            
            else          
            {                
                key->state = STATE_KEY_UP;           
             }
             break;
         case STATE_KEY_LONG:
            if (key->IsKeyDownFunc())  
            {               
				key->Count++;  
				if (key->Count >= key->LongTime)
				{
					/* 2s，发送按钮长按下的消息 */ 
					key_write_data(&(key->key_long_value));
					if (key->RepeatTime> 0)     	//检测连续按键
					{
						key->state = STATE_KEY_REPEAT;
						key->Count = 0;						
					}
					else
					{
						 key->state = STATE_KEY_RELEASE;
					}

				}              
            }
            else          
            {                
               key->state = STATE_KEY_RELEASE; 
            }
            break;
        case  STATE_KEY_REPEAT:
            if (key->IsKeyDownFunc())  
            {                
				 key->Count++;  
				 if (key->Count >= key->RepeatTime)
				 {
					 /* 常按键后，每隔KEY_REPEAT_PERIOD发送1个按键消息 */
					 key_write_data(&(key->key_down_value));
					 key->state = STATE_KEY_REPEAT;
					 key->Count = 0;
				 }                 
            }
            else
            {
                key->state = STATE_KEY_RELEASE;
            }
            break;
        case STATE_KEY_RELEASE:
            /* 发送按钮弹起的消息 */
//            key_write_data(&(key->key_up_value));
            key->state = STATE_KEY_UP;
            break;
        default:
            key->state = STATE_KEY_UP;
            break;
                
    }
}
/*******************************************************************************
*	函 数 名: key_scan
*	功能说明: 扫描所有按键。非阻塞，周期性的调用
*	形    参:  无
*	返 回 值: 无
*******************************************************************************/
void key_scan(void)
{
	uint8_t i;

	for (i = 0; i < KEY_COUNT; i++)
	{
		bsp_detect_key(i);
	}
}


