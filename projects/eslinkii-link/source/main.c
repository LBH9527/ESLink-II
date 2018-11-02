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
#include "settings_rom.h"

#include "eslink_gpio.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "uart.h"
#include "spi_flash.h" 
#include "eeprom.h"
#include "key.h"  
#include "oled.h"   

#include "DAP.h"
#include "eslink.h"
#include "menu.h"
#include "offline_app.h"
#include "offline_file.h" 

/*******************************************************************************
								变量
*******************************************************************************/
/* 任务栈 */ 
static U64 stk_main_task[MAIN_TASK_STACK / sizeof(U64)]; 
static U64 stk_offline_task[OFFLINE_TASK_STACK / sizeof(U64)];
static U64 stk_eslink_task[ESLINK_TASK_STACK / sizeof(U64)];  
static U64 stk_key_task[KEY_TASK_STACK / sizeof(U64)];
static U64 stk_oled_diaplay_task[OLED_DISPLAY_TASK_STACK / sizeof(U64)];

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
#define FLAGS_MAIN_PROC_USB     (1 << 5)

// Reference to our main task
OS_TID main_task_id;
/*******************************************************************************
							函数声明
*******************************************************************************/
__task void main_task(void);
__task void timer_task(void);
__task void key_task(void);
__task void oled_display_task(void );
__task void offline_program_task(void);

extern void cdc_process_event(void);
// Start CDC processing
void main_cdc_send_event(void)
{
    os_evt_set(FLAGS_MAIN_CDC_EVENT, main_task_id);
    return;
}
//reset system
void main_reset(void )
{
    isr_evt_set(FLAGS_MAIN_RESET, main_task_id);
// os_evt_set(FLAGS_MAIN_RESET, main_task_id);
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
void USBD_SignalHandler()
{
    isr_evt_set(FLAGS_MAIN_PROC_USB, main_task_id);
} 

/*******************************************************************************
*	函 数 名: offline_program_task
*	功能说明: 联机任务
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 5    OFFLINE_TASK_PRIORITY
*******************************************************************************/
__task void eslink_task(void)
{
    uint8_t i;
    uint16_t flags = 0;
    
    DAP_Setup(); 
    // Get a reference to this task
    main_task_id = os_tsk_self();    
    usbd_init();                          /* USB Device Initialization          */
    usbd_connect(__TRUE);                 /* USB Device Connect                 */
    while (!usbd_configured ());          /* Wait for device to configure        */  
	// Update versions and IDs
    //    info_init();    
    es_program_init(PRG_INTF_ISP);  //上电默认ISp烧录
     while (1) 
     {
        // need to create a new event for programming failure
        os_evt_wait_or( FLAGS_MAIN_RESET            // reset
                       | FLAGS_MAIN_RESET_TARGET    // Put target in reset state
                       | FLAGS_MAIN_30MS            // 30mS tick
                       | FLAGS_MAIN_CDC_EVENT       // cdc event
                       | FLAGS_MAIN_PROC_USB        // process usb events
                       , NO_TIMEOUT);
        // Find out what event happened
        flags = os_evt_get();
//		if (flags & FLAGS_MAIN_PROC_USB) 
//        {
//            USBD_Handler();
//        }
		
        if (flags & FLAGS_MAIN_RESET)
        {
            clear_timing_info();    
            set_app_update(UPDATE_BOOT_APP);
            SystemSoftReset();           
        }
        if (flags & FLAGS_MAIN_RESET_TARGET) 
        {
//            target_set_state(RESET_RUN);
        }
        if (flags & FLAGS_MAIN_CDC_EVENT)
        {
            cdc_process_event();
        }
//        if (flags & FLAGS_MAIN_30MS) 
//        {
//            if(i++ % 30 == 0)
//            {                
//                LED_RED_TOGGLE();                  
//            }
////            if(i % 90 == 0)
////                SystemReset();            
//        }
        os_dly_wait(10); 
        
     }
    
}
/*******************************************************************************
*	函 数 名: offline_program_task
*	功能说明: 脱机编程
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 5    OFFLINE_TASK_PRIORITY
*******************************************************************************/
__task void offline_program_task(void)
{
    ofl_prog_state_t state = IN_MODE_CHECK;
    
    uint8_t menu_msg;
    
    ofl_prog_init();
    while(1)
    {
        switch(state)
        {
            case IN_MODE_CHECK :
                if(ofl_in_prog_mode() == TRUE)
                {                       
                     state = OFL_PROG_ING;
                     menu_msg = MSG_PROG_ING;
                     msg_write_data(&menu_msg) ;    
                }
                break;
            case OFL_PROG_ING:                 
                if(ofl_prog() != TRUE)
                    menu_msg = MSG_PROG_FAILE;
                else
                    menu_msg = MSG_PROG_OK;
                state = OUT_MODE_CHECK; 
                msg_write_data(&menu_msg);
                break;
            case OUT_MODE_CHECK:
                if(ofl_in_prog_mode() != TRUE)
                {
                    state = IN_MODE_CHECK; 
                    menu_msg = MSG_PROG_MODE_CHECK;
                    msg_write_data(&menu_msg);     
                
                }
                break;           
        }  
        os_dly_wait(100);    
    }
}
/*******************************************************************************
*	函 数 名: key_task
*	功能说明: 按键消息处理任务
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 2    KEY_TASK_PRIORITY
*******************************************************************************/
__task void key_task(void)
{
    uint8_t key_value;
    uint8_t menu_msg;
    /* 创建信号量计数值是0, 用于任务同步 */
//    os_sem_init (&key_sem, 0);

    while(1)
    {
        if(key_read_data(&key_value) != 0)        //有按键按下
        {
            switch (key_value)
            {
                case KEY_DOWN:
                    menu_msg = MSG_KEY_DOWN; 
                   
                break;
                case KEY_ENTER:
                    menu_msg = MSG_KEY_ENTER; 
                break;
                default:
                break;
            }  
            msg_write_data(&menu_msg);
        }          		
        os_dly_wait(10);    //100ms
    }
}


/*******************************************************************************
*	函 数 名: gui_refresh_task
*	功能说明: gui刷新任务
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 1  DISPALY_TASK_PRIORITY
*******************************************************************************/
__task void oled_display_task(void )
{     
    menu_init();
    while(1)
    {     
		menu_display();
        os_dly_wait(40);    //400ms
    }
}

/*******************************************************************************
*	函 数 名: main_task
*	功能说明: 启动任务，优先级最高
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 5       
*******************************************************************************/
__task void main_task(void)
{   
	 //创建任务
    
    // Start key tasks
    os_tsk_create_user(key_task, KEY_TASK_PRIORITY, (void *)stk_key_task, KEY_TASK_STACK);
    // Start oled display tasks
    os_tsk_create_user(oled_display_task, DISPALY_TASK_PRIORITY, (void *)stk_oled_diaplay_task, OLED_DISPLAY_TASK_STACK);
    
    if(get_link_mode() == LINK_OFFLINE_MODE)
    {
          os_tsk_create_user(offline_program_task, OFFLINE_TASK_PRIORITY, (void *)stk_offline_task, OFFLINE_TASK_STACK);//    
    }   
	else
	{
		 // Start timer tasks
		os_tsk_create_user(eslink_task, ESLINK_TASK_PRIORITY, (void *)stk_eslink_task, ESLINK_TASK_STACK);			
	}
    
     while (1) 
     {
		/* 按键扫描 */
		key_scan();
        os_dly_wait(2);
     }
    
}
int main (void) 
{	
	__set_PRIMASK(0); 
    SCB->VTOR = SCB_VTOR_TBLOFF_Msk & ESLINK_ROM_LINK_START;    
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();   
    settings_rom_init();
    
    gpio_init();
    es_set_trget_power(TRGET_POWER_3V3);     
    oled_init();
    key_init();     

    fm24cxx_init();
    
    spi_flash_init(); 
    //    sf_erase_chip();  
    ofl_file_init();
    
    msg_init();
	LED_GREEN_ON();
    os_sys_init_user(main_task, MAIN_TASK_PRIORITY, stk_main_task, MAIN_TASK_STACK);//
	while(1);
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
    rt_hw_hard_fault_exception((struct exception_stack_frame *)__get_MSP());
    
    while (1); // Wait for reset
}
