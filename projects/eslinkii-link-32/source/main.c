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
//#include "tasks.h"
#include "settings_rom.h"   
#include "eslink_gpio.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "uart.h"
#include "spi_flash.h" 
#include "eeprom.h"
#include "key.h"  
#include "oled.h"   
#include "beep.h" 
#include "DAP.h"
#include "eslink.h"
#include "menu.h"
#include "offline_app.h"
#include "offline_file.h" 
#include "systick.h"

/*******************************************************************************
								变量
*******************************************************************************/

uint32_t  task_flags = 0;
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
#define FLAGS_GUI_REFRESH       (1 << 5)

///*******************************************************************************
//							函数声明
//*******************************************************************************/
extern void cdc_process_event(void);
// Start CDC processing
void main_cdc_send_event(void)
{
    flag_send(task_flags, FLAGS_MAIN_CDC_EVENT);
    return;
}
//reset system
void main_reset(void )
{
    flag_send(task_flags, FLAGS_MAIN_RESET);
// os_evt_set(FLAGS_MAIN_RESET, main_task_id);
    return;
}

// Functions called from other tasks to trigger events in the main task
// parameter should be reset type??
void main_reset_target(uint8_t send_unique_id)
{
    flag_send(task_flags, FLAGS_MAIN_RESET_TARGET);
    return;
}
// Flash CDC LED using 30mS tick
void main_blink_cdc_led(main_led_state_t state)
{
//    cdc_led_usb_activity = 1;
//    cdc_led_state = state;
    return;
} 

void gui_refresh(void)
{
    flag_send(task_flags, FLAGS_GUI_REFRESH);
} 


int main (void) 
{	
    uint8_t key_value = 0;
    uint8_t menu_msg = MSG_NULL;
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
    bsp_init_systick();
    menu_init();
    
    LED_GREEN_ON();  
    //脱机模式
    if(get_link_mode() == LINK_OFFLINE_MODE)
    {    
        static  ofl_prog_state_t state = IN_MODE_CHECK;
        ofl_prog_error_t  prog_error ;
           
        ofl_prog_init();           
        while(1)
        {
            switch(state)
            {
                case IN_MODE_CHECK :
                    //保证信号正确，LED应该存在问题
                    LED_RED_ERROR_ON();  
                    LED_GREEN_PASS_ON(); 
                    LED_YELLOW_BUSY_ON();  
                    if(ofl_in_prog_mode() == TRUE)
                    {                       
                         state = OFL_PROG_ING;
                         menu_msg = MSG_PROG_ING;
                         msg_write_data(&menu_msg) ;    
                    }
                    break;
                case OFL_PROG_ING:   
                    //busy 信号
                    LED_YELLOW_BUSY_OFF();  
                    //
                    prog_error = ofl_prog() ;
                    
                    LED_YELLOW_BUSY_OFF();
                    switch(prog_error)
                    {
                        case OFL_SUCCESS:                              
                             menu_msg = MSG_PROG_OK;
                            break;
                        case OFL_COUNT_FULL:
                            beep_prog_fail();
                             menu_msg = MSG_PROG_COUNT_FULL;
                             break;
                        case OFL_PROG_FAIL:                             
                             menu_msg = MSG_PROG_FAILE;
                             break;
                         default:
                            menu_msg = MSG_NULL;
                            break;                    
                    }  
                    if( prog_error != OFL_SUCCESS)      //编程失败
                    {
                        //error灯(红灯)
                        beep_prog_fail();
                        LED_RED_ERROR_OFF();   
                    }
                    else
                    {
                       //pass灯(绿灯)
                        LED_GREEN_PASS_OFF();                          
                        beep_prog_success();
                    }
                        
                    state = OUT_MODE_CHECK; 
                    msg_write_data(&menu_msg);
                    break;
                case OUT_MODE_CHECK:
                    if(ofl_out_prog_mode() == TRUE)
                    {
                        state = IN_MODE_CHECK; 
                        menu_msg = MSG_PROG_MODE_CHECK;
                        msg_write_data(&menu_msg);                     
                    }
                    break; 
                 default:
                    break;
            }  
            if( flag_recv(task_flags, FLAGS_GUI_REFRESH))
            {
                flag_clr(task_flags, FLAGS_GUI_REFRESH);
                menu_display(); 
            }
            
            if(key_read_data(&key_value) != 0)        //有按键按下
            {
                switch (key_value)
                {
                    case KEY_DOWN:
                    menu_msg = MSG_KEY_DOWN;                
                    break;
                    case KEY_ENTER:
                    update_ofl_serial_number();
                    menu_msg = MSG_KEY_ENTER; 
                    break;
                    default:
                    break;
                }  
                msg_write_data(&menu_msg);
            }           
        }           
    }   
	else
	{    
        DAP_Setup(); 
        // Get a reference to this task
        //  main_task_id = os_tsk_self();    
        usbd_init();                          /* USB Device Initialization          */
        usbd_connect(__TRUE);                 /* USB Device Connect                 */
        while (!usbd_configured ());          /* Wait for device to configure        */  
        // Update versions and IDs
        //    info_init();    
        es_burner_init(PRG_INTF_ISP);  //上电默认ISp烧录 
        while (1) 
        {         
            if( flag_recv(task_flags, FLAGS_MAIN_RESET) )
            {
                flag_clr(task_flags, FLAGS_MAIN_RESET);
                clear_timing_info();    
                set_app_update(UPDATE_BOOT_APP);
                bsp_delay_ms(10);       //延时，USB回复数据。
                SystemSoftReset();           
            }
            if( flag_recv(task_flags, FLAGS_MAIN_RESET_TARGET)  )
            {
                flag_clr(task_flags, FLAGS_MAIN_RESET_TARGET);
            //            target_set_state(RESET_RUN);
            }
            if( flag_recv(task_flags, FLAGS_MAIN_CDC_EVENT)) 
            {
                flag_clr(task_flags, FLAGS_MAIN_CDC_EVENT);
                cdc_process_event();
            }
            if( flag_recv(task_flags, FLAGS_GUI_REFRESH))
            {
                flag_clr(task_flags, FLAGS_GUI_REFRESH);
                menu_display(); 
            }
            
            if(key_read_data(&key_value) != 0)        //有按键按下
            {
                switch (key_value)
                {
                    case KEY_DOWN:
                    menu_msg = MSG_KEY_DOWN; 
//                    beep_prog_success();                    
                    break;
                case KEY_ENTER:
                    menu_msg = MSG_KEY_ENTER; 
                    break;
                default:
                    break;
                }  
                msg_write_data(&menu_msg);
            }          
        }        
    }        
}

void main_10ms_task(void)
{
    static uint8_t count = 0;
    /* 按键扫描 */
	key_scan();
    // 蜂鸣器扫描
    beep_scan();
    if (!(count++ % 10))        //100ms刷新 oled
    {
        gui_refresh();
    }
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
