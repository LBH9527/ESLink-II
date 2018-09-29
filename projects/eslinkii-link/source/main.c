/*******************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 :   任务设置：
*                1. os_idle_demon，   优先级0: 系统空闲任务。
*                2. 按键任务
*	修改记录 :
*		版本号    日期         作者        说明
*
*
*******************************************************************************/	
#include "fsl_device_registers.h"       
#include "board.h" 
#include "clock_config.h" 
#include "cortex_m.h"
#include "eslink_addr.h" 
#include "main.h" 
#include "RTL.h"
#include "rl_usb.h"
#include "tasks.h"
#include "config_rom_set.h"

#include "gpio.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "uart.h"
#include "spi_flash.h" 
#include "key.h"  
#include "oled.h"   

#include "DAP.h"
#include "eslink.h"
#include "menu.h"
#include "offline_file.h"
/*******************************************************************************
								变量
*******************************************************************************/
/* 任务栈 */  
static U64 stk_timer_task[TIMER_TASK_STACK / sizeof(U64)];
static U64 stk_main_task[MAIN_TASK_STACK / sizeof(U64)];
static U64 stk_key_task[KEY_TASK_STACK / sizeof(U64)];
static U64 stk_oled_diaplay_task[OLED_DISPLAY_TASK_STACK / sizeof(U64)];
//static U64 stk_offline_task[OFFLINE_TASK_STACK / sizeof(U64)];
/* 信号量 */
OS_SEM key_sem;
/* 事件标志 */
// Event flags for main task
// Timers events
#define FLAGS_MAIN_30MS         (1 << 0)
// Reset events
#define FLAGS_MAIN_RESET        (1 << 1)
#define FLAGS_MAIN_RESET_TARGET (1 << 2)
#define FLAGS_MAIN_CDC_EVENT    (1 << 3)
// Used by msd when flashing a new binary
#define FLAGS_LED_BLINK_30MS    (1 << 4)

// Reference to our main task
OS_TID main_task_id;
/*******************************************************************************
							函数声明
*******************************************************************************/
__task void main_task(void);
__task void timer_task(void);
__task void key_task(void);
__task void oled_display_task(void );

// Start CDC processing
void main_cdc_send_event(void)
{
    os_evt_set(FLAGS_MAIN_CDC_EVENT, main_task_id);
    return;
}
//reset system
void main_reset(void )
{
    os_evt_set(FLAGS_MAIN_RESET, main_task_id);
    return;
}

// Functions called from other tasks to trigger events in the main task
// parameter should be reset type??
void main_reset_target(uint8_t send_unique_id)
{
    os_evt_set(FLAGS_MAIN_RESET_TARGET, main_task_id);
    return;
}
// Flash CDC LED using 30mS tick
void main_blink_cdc_led(main_led_state_t state)
{
//    cdc_led_usb_activity = 1;
//    cdc_led_state = state;
    return;
}




/*******************************************************************************
*	函 数 名: main_task
*	功能说明: 
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 5       
*******************************************************************************/
__task void main_task(void)
{
    uint8_t i;
    uint16_t flags = 0;
    // Get a reference to this task
    main_task_id = os_tsk_self();
    
    usbd_init();                          /* USB Device Initialization          */
    usbd_connect(__TRUE);                 /* USB Device Connect                 */
    while (!usbd_configured ());          /* Wait for device to configure        */  
    // Update versions and IDs
    //    info_init();
    
    // Start timer tasks
    os_tsk_create_user(timer_task, TIMER_TASK_PRIORITY, (void *)stk_timer_task, TIMER_TASK_STACK);
    // Start key tasks
//    os_tsk_create_user(key_task, KEY_TASK_PRIORITY, (void *)stk_key_task, KEY_TASK_STACK);
    // Start oled display tasks
    os_tsk_create_user(oled_display_task, DISPALY_TASK_PRIORITY, (void *)stk_oled_diaplay_task, OLED_DISPLAY_TASK_STACK);
     while (1) 
     {
        // need to create a new event for programming failure
        os_evt_wait_or( FLAGS_MAIN_RESET            // reset
                       | FLAGS_MAIN_RESET_TARGET    // Put target in reset state
                       | FLAGS_MAIN_30MS            // 30mS tick
                       | FLAGS_MAIN_CDC_EVENT       // cdc event
                       , NO_TIMEOUT);
        // Find out what event happened
        flags = os_evt_get();
        if (flags & FLAGS_MAIN_RESET)
        {
            clear_timing_info_and_update();
            SystemSoftReset();           
        }
        if (flags & FLAGS_MAIN_RESET_TARGET) 
        {
//            target_set_state(RESET_RUN);
        }
        if (flags & FLAGS_MAIN_CDC_EVENT)
        {
//            cdc_process_event();
        }
        if (flags & FLAGS_MAIN_30MS) 
        {
            if(30 == i++)
            {
                i = 0;
                LED_YELLOW_TOGGLE();
               
            }
        }
        
     }
    
}
__task void offline_task(void)
{
    
}
/*******************************************************************************
*	函 数 名: key_task
*	功能说明: 按键扫描任务
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 3
*******************************************************************************/
__task void key_task(void)
{
    uint8_t key_value;
    /* 创建信号量计数值是0, 用于任务同步 */
//    os_sem_init (&key_sem, 0);

    while(1)
    {
        if (key_read_data(&key_value))
        {
            if (key_value == KEY0_DOWN)
            {
    
             
            }
            else if (key_value == KEY0_LONG)
            {
             
             
            }
        } 		
        os_dly_wait(10);    //100ms
    }
}


/*******************************************************************************
*	函 数 名: gui_refresh_task
*	功能说明: gui刷新函数
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 3  优先级较低
*******************************************************************************/
__task void oled_display_task(void )
{     
//	eslink_display();


    menu_display_logo();
    while(1)
    {         
        os_dly_wait(10);    //100ms
    }
}
/*******************************************************************************
*	函 数 名: timer_task_10mS
*	功能说明: gui刷新函数
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 3  优先级较低
*******************************************************************************/
__task void timer_task(void)
{
    uint8_t i = 0;
    os_itv_set(1); // 10mS

    while (1) {
        /* 按键扫描 */
		key_scan();
        os_itv_wait();    
        if (!(i++ % 3)) {
            os_evt_set(FLAGS_MAIN_30MS, main_task_id);
        }
    }
}


int main(void)
{
//    SCB->VTOR = SCB_VTOR_TBLOFF_Msk & ESLINK_ROM_LINK_START;    
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();   
    config_rom_init();
    
    gpio_init();
    gpio_set_trget_power(TRGET_POWER_3V3);
    oled_init();
    key_init();     
    DAP_Setup();      
    ofl_init();
    eslink_init();
//    ofl_file_open("111") ;
    //创建联机任务
    os_sys_init_user(main_task, MAIN_TASK_PRIORITY, stk_main_task, MAIN_TASK_STACK);//
    //脱机任务
}

struct exception_stack_frame
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
};

void rt_hw_hard_fault_exception(struct exception_stack_frame *exception_stack)
{
//    printf("psr: 0x%08x\r\n", exception_stack->psr);
//    printf(" pc: 0x%08x\r\n", exception_stack->pc);
//    printf(" lr: 0x%08x\r\n", exception_stack->lr);
//    printf("r12: 0x%08x\r\n", exception_stack->r12);
//    printf("r03: 0x%08x\r\n", exception_stack->r3);
//    printf("r02: 0x%08x\r\n", exception_stack->r2);
//    printf("r01: 0x%08x\r\n", exception_stack->r1);
//    printf("r00: 0x%08x\r\n", exception_stack->r0);
}


void HardFault_Handler()
{
//    util_assert(0);
//    SystemReset();
//     printf("\r\n HardFault_Handler interrupt!\r\n");
    rt_hw_hard_fault_exception((struct exception_stack_frame *)__get_PSP());
    while (1); // Wait for reset
}
