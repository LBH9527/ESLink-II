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
//#include "eslink_addr.h" 
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
#include "eeprom_port.h"
#include "key.h"  
#include "oled.h"   
#include "beep.h" 
#include "eeprom.h"
#include "DAP.h"
#include "eslink.h"
#include "menu.h"
#include "offline_app.h"
#include "offline_file.h" 
#include "systick.h" 
#include "target_config.h"
#include "rtc_calibrate.h"

/*******************************************************************************
								变量
*******************************************************************************/  
uint32_t  task_flags = 0;
/* 事件标志 */
// Event flags for main task
// Reset events
#define FLAGS_MAIN_RESET        (1 << 0)
//#define FLAGS_MAIN_RESET_TARGET (1 << 1)
#define FLAGS_MAIN_CDC_EVENT    (1 << 2)

#define FLAGS_GUI_REFRESH       (1 << 3) 
#define FLAGS_RTC_OUT           (1 << 4)        //RTC自校正
          
#define FLAGS_USB_HANDLER           (1 << 15)       
///*******************************************************************************
//							函数声明
//*******************************************************************************/
void oline_app(void);
void ofl_app(void);
void oline_mini_app(void);
void ofl_mini_app(void);

extern void cdc_process_event(void);

void USBD_SignalHandler(void)
{
    flag_send(task_flags, FLAGS_USB_HANDLER);
}    

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
    return;
}

void gui_refresh(void)
{
    flag_send(task_flags, FLAGS_GUI_REFRESH);
    return;
} 

#if ESLINK_RTC_ENABLE  
static uint8_t  rtc_out_activity = DISABLE;
uint8_t rtc_out_mode(uint8_t enable)
{   
     rtc_out_activity = enable;
     flag_send(task_flags, FLAGS_RTC_OUT);

     return TRUE;
}
#endif

int main (void) 
{	
    SCB->VTOR = SCB_VTOR_TBLOFF_Msk & ESLINK_ROM_LINK_START;    
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();   
    settings_rom_init();      
    gpio_init();     
    es_set_trget_power(TRGET_POWER_ENABLE);  
    key_init();              
    spi_flash_init();     
    
    LED_GREEN_ON();
    
    if(eslink_is_mini() != TRUE)
    {
        ofl_file_init(ESLINK_PLUS_TYPE); 
        #if ESLINK_RTC_ENABLE    
        rtc_Init();
        #endif
        oled_init(); 
        bsp_init_systick();
        fm24cxx_init(); 
        gui_msg_init();   	
        if(eslink_is_offline_mode() != TRUE)
        {   
            oline_app();        
        }   
        else
        {                
            ofl_app();
        }   
    }
    else
    {     
        bsp_init_systick();
        oline_mini_app();
    }
}

//联机模式
void oline_app(void)
{
    uint8_t key_value = 0;
    uint8_t menu_msg = MSG_NULL;
    menu_init(MENU_ONLINE_MODE);   //联机
    menu_display(); 
            
    DAP_Setup();  
    usbd_init();                        /* USB Device Initialization          */
    usbd_connect(1);                    /* USB Device Connect                 */
   
    es_burner_init();  
    while (1) 
    {    
        {
            //usb已经联机
            if( flag_recv(task_flags, FLAGS_USB_HANDLER) )
            {
                flag_clr(task_flags, FLAGS_USB_HANDLER);
                USBD_Handler();
            }
            if(!usbd_configured () )  // 检测usb是否联机
            {
//                usbd_connect(0);
                bsp_delay_ms(10); 
                usbd_connect(1);
            }
            if( flag_recv(task_flags, FLAGS_MAIN_RESET) )
            {
                flag_clr(task_flags, FLAGS_MAIN_RESET);
                clear_timing_info();    
                set_app_update(UPDATE_BOOT_APP);
                bsp_delay_ms(10);       //延时，USB回复数据。
                SystemSoftReset();           
            }
            if( flag_recv(task_flags, FLAGS_MAIN_CDC_EVENT)  ) 
            {
                flag_clr(task_flags, FLAGS_MAIN_CDC_EVENT);
                cdc_process_event();
            }  
#if ESLINK_RTC_ENABLE  
            //rtc_out
            if( flag_recv(task_flags, FLAGS_RTC_OUT)  ) 
            {
                flag_clr(task_flags, FLAGS_RTC_OUT);
                rtc_pwm_out(rtc_out_activity); 
            }              
#endif            
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
                case KEY_ENTER:     //长按
                    beep_key_press();
                    menu_msg = MSG_KEY_ENTER; 
                
                    break;
                default:
                    break;
            }  
            gui_msg_write_data(&menu_msg);
        }         
    }  
}
//脱机模式
void ofl_app(void)
{
    uint8_t key_value = 0;
    uint8_t menu_msg = MSG_NULL;
    menu_init(MENU_OFFLINE_MODE);  
    menu_display();    
    ofl_prog_init(OFFLINE_PROG_PLUS_MODE);           
    while(1)
    {
        ofl_prog_handle();
        
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
                    beep_key_press();
                    update_ofl_serial_number();    
                    menu_msg = MSG_KEY_ENTER; 
                break;
                default:
                break;
            }  
            gui_msg_write_data(&menu_msg);
        }           
    }   
}
//mini联机
void oline_mini_app(void)
{
    uint8_t key_value = 0;
    DAP_Setup();       
 
    usbd_init();                        /* USB Device Initialization          */
    usbd_connect(1);                    /* USB Device Connect                 */
    es_burner_init();        
    while (1) 
    {    
        {
            //usb已经联机
            if( flag_recv(task_flags, FLAGS_USB_HANDLER) )
            {
                flag_clr(task_flags, FLAGS_USB_HANDLER);
                USBD_Handler();
            }
            if(!usbd_configured () )  // 检测usb是否联机
            {
//                usbd_connect(0);
                bsp_delay_ms(10); 
                usbd_connect(1);
            }
            if( flag_recv(task_flags, FLAGS_MAIN_RESET) )
            {
                flag_clr(task_flags, FLAGS_MAIN_RESET);
                clear_timing_info();    
                set_app_update(UPDATE_BOOT_APP);
                bsp_delay_ms(10);       //延时，USB回复数据。
                SystemSoftReset();           
            }   
            
            if( flag_recv(task_flags, FLAGS_MAIN_CDC_EVENT)  ) 
            {
                flag_clr(task_flags, FLAGS_MAIN_CDC_EVENT);
                cdc_process_event();
            }     
        }         
        if(key_read_data(&key_value) != 0)        //有按键按下
        {
            if(key_value == KEY_ENTER)
                ofl_mini_app();  
        }         
    }

}
void ofl_mini_app(void)
{
    ofl_prog_init(OFFLINE_PROG_MINI_MODE);          
    while(1)
    {
        mini_ofl_prog_handle();                
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
    SystemSoftReset();
//    rt_hw_hard_fault_exception((struct exception_stack_frame *)__get_PSP());
//    rt_hw_hard_fault_exception((struct exception_stack_frame *)__get_MSP());
    
    while (1); // Wait for reset
}
