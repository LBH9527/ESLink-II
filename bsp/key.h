/*
********************************************************************************
*
*	模块名称 : 按键驱动模块
*	文件名称 : bsp_key.h
*	版    本 : V1.0
*	说    明 : 头文件
*
********************************************************************************
*/
#ifndef __KEY_H
#define __KEY_H	

#include <stdbool.h>
#include <stdint.h>
/******************************************************************************/
/* 
    根据应用程序的功能重命名按键宏,用在其他函数调用
*/
#define KEY_DOWN		KEY0_DOWN
#define KEY_ENTER		KEY0_LONG

//#define KEY_DOWN_K1		KEY1_DOWN
//#define KEY_LONG_K1		KEY1_LONG

//#define KEY_DOWN_K2		KEY2_DOWN
//#define KEY_LONG_K2		KEY2_LONG

/******************************************************************************/
#define KEY_COUNT   1               //按键个数(有组合按键也要添加)
/*
	按键滤波时间100ms, 单位10ms。
	只有连续检测到100ms状态不变才认为有效，包括弹起和按下两种事件	
*/
#define KEY_DOWN_PERIOD     5		//单位20ms， 持续100ms，认为按键按下
#define KEY_LONG_PERIOD     100		//单位20ms， 持续2秒，认为长按事件
#define KEY_REPEAT_PERIOD   2       //单位20ms， 连续按键周期为40ms
/* 
    按键ID, 主要用于GetKeyState()函数的入口参数
*/
typedef enum
{    
    KID_K0,
//	KID_K1,
//	KID_K2,
}KEY_ID_E;
/* 
    按键状态
*/
enum
{    
    STATE_KEY_UP,    
    STATE_KEY_DEBOUNCE,    
    STATE_KEY_DOWN,    
    STATE_KEY_LONG,    
    STATE_KEY_REPEAT,    
    STATE_KEY_RELEASE
};

/*
	定义键值代码, 必须按如下次序定时每个键的按下、弹起和长按事件
	可以根据要求选取事件来处理程序
*/
enum
{
    KEY_NONE = 0,   /* 0 表示无按键事件 */

    KEY0_DOWN,      /* 0键按下 */
    KEY0_UP,        /* 0键弹起 */
    KEY0_LONG,      /* 0键长按 */

//    KEY1_DOWN,      /* 1键按下 */
//    KEY1_UP,        /* 1键弹起 */
//    KEY1_LONG,      /* 1键长按 */

//    KEY2_DOWN,      /* 2键按下 */
//    KEY2_UP,        /* 2键弹起 */
//    KEY2_LONG,      /* 2键长按 */      
};

/*
	每个按键对应1个全局的结构体变量。
*/
typedef struct
{
	uint8_t (*IsKeyDownFunc)(void); /* 按键按下的判断函数,1表示按下 */
    uint8_t FilterTime;             /* 滤波时间(最大255,表示2550ms) */  
	uint8_t Count;			        /* 按键计数器 */
	uint8_t LongTime;		        /* 按键按下持续时间, 0表示不检测长按 */
	uint8_t RepeatTime;	            /* 连续按键周期 , 0表示不检测长按*/
	uint8_t state;                  /*按键状态*/  
     
    uint8_t key_up_value;              /* 按键弹起的键值代码 */  
    uint8_t key_down_value;            /* 按键按下的键值代码 */  
    uint8_t key_long_value;            /* 按键长按的键值代码 */  

}key_param_t;


/* 供外部调用的函数声明 */
void key_init(void);
void key_scan(void);
uint8_t key_read_data(uint8_t *key_value);

#endif


