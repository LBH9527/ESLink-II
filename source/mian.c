/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 :   任务设置：
*                总共创建了15个任务
*                1. os_idle_demon，   优先级0: 系统空闲任务。
*                2. AppTaskStatistic  优先级1：CPU利用率检测。
*                3. AppTaskGUI，      优先级3：GUI主任务。
*                4. AppTaskTCPMain，  优先级4：TCPnet网络任务。
*                5. AppTaskUserIF，   优先级5：按键消息处理任务。
*                6. AppTaskMsgPro，   优先级6：主要用于截图任务。
*                7. AppTaskStart，    优先级7：启动任务。
*                8. AppTaskCAN，      优先级8：CAN总线任务。
*                9. AppTaskModbus，   优先级9：Modbus任务。
*               10. AppTaskUSB，      优先级10：USB任务，
*               11. AppTaskMusic，    优先级11：音乐任务，用于音乐播放器，视频播放器，录音机和收音机处理。
*               12. AppTaskPinDetect，优先级12：SD卡插拔检测任务。
*               13. AppTaskTCPTimer， 优先级13：TCPnet网络定时更新任务。
*                   USB在运行的过程中会创建2两个任务
*               14. USBD_RTX_EndPoint0          优先级2：USB设备端点0任务。
*               15. USBD_RTX_MSC_EP_BULK_Event  优先级2：USB BULK数据传输任务。
*             中断优先级：
*               NVIC优先级分组采用4，仅抢占优先级，范围0-15。
*               LTDC中断，       抢占优先级 0
*               ETH以太网中断，  抢占优先级 0
*               USB FS中断，     抢占优先级 0
*               CAN2 TX中断，    抢占优先级 1
*               CAN2 RX0中断，   抢占优先级 1
*               DMA2_Steam5中断，抢占优先级 1
*               DMA2_Steam1中断，抢占优先级 1
*               EXTI6中断，      抢占优先级 1
*               USART3中断，     抢占优先级 2
*               EXTI2中断，      抢占优先级 2
*               TIM2中断，       抢占优先级 4
*               USART1中断，     抢占优先级 13
*               RTC Alarm中断，  抢占优先级 13
*               SVC中断，        抢占优先级 14
*               Systick中断，    抢占优先级 15
*               PendSV中断，     抢占优先级 15

*
*	修改记录 :
*		版本号    日期         作者        说明
*
*
*********************************************************************************************************
*/	
#include "fsl_device_registers.h"

#include "board.h"
#include "gpio.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "main.h"

#include "RTL.h"
#include "rl_usb.h"
#include "tasks.h"


#include "spi_flash.h"
#include "uart.h"
#include "key.h"
#include "spi_flash_fs.h"  
#include "oled.h"

#include "DAP.h"
#include "eslink_io_config.h"
#include "ES_ISP.h"
#include "swd_target_reset.h"

#include "eslink_debug.h"

//for test
#include "DAP_config.h"
#include "eslink_config.h"
#include "isp_m939.h"

/*******************************************************************************
							函数声明
*******************************************************************************/
__task void timer_task_30mS(void);
__task void key_task(void);


//extern task
//extern __task void hid_process(void);
//extern void hid_send_packet(void);
extern void cdc_process_event(void);
extern void icd_process_continue(void);
extern void offline_process_event(void);
//__attribute__((weak)) void prerun_board_config(void) {}
//__attribute__((weak)) void prerun_target_config(void) {}

//add by 9527
//extern __task void isp_process(void);
/*******************************************************************************
								变量
*******************************************************************************/
/* 任务栈 */  
static U64 stk_timer_30_task[TIMER_TASK_30_STACK / sizeof(U64)];
//static U64 stk_dap_task[DAP_TASK_STACK / sizeof(U64)];
static U64 stk_main_task[MAIN_TASK_STACK / sizeof(U64)];
//add by 9527 (down)
//static U64 stk_es_task[ES_TASK_STACK / sizeof(U64)]; 
static U64 stk_key_task[KEY_TASK_STACK / sizeof(U64)];
static U64 stk_offline_task[OFFLINE_TASK_STACK / sizeof(U64)];
/* 信号量 */
OS_SEM key_sem;

/* 事件标志 */
// Event flags for main task
// Timers events
#define FLAGS_MAIN_90MS         (1 << 0)
#define FLAGS_MAIN_30MS         (1 << 1)
// Reset events
#define FLAGS_MAIN_RESET        (1 << 2)
// Other Events
#define FLAGS_MAIN_POWERDOWN    (1 << 4)
#define FLAGS_MAIN_DISABLEDEBUG (1 << 5)
#define FLAGS_MAIN_PROC_USB     (1 << 9)
// Used by hid when no longer idle
//#define FLAGS_MAIN_HID_SEND     (1 << 10)
// Used by cdc when an event occurs
#define FLAGS_MAIN_CDC_EVENT    (1 << 11)
// Used by msd when flashing a new binary
#define FLAGS_LED_BLINK_30MS    (1 << 6)

// Timing constants (in 90mS ticks)
// USB busy time (~3 sec)
#define USB_BUSY_TIME           (33)
// Delay before a USB device connect may occur (~1 sec)
#define USB_CONNECT_DELAY       (11)
// Timeout for USB being configured (~2 sec)
#define USB_CONFIGURE_TIMEOUT   (22)
// Delay before target may be taken out of reset or reprogrammed after startup
#define STARTUP_DELAY           (1)

// Decrement to zero
#define DECZERO(x)              (x ? --x : 0)

// Reference to our main task
OS_TID main_task_id;

// USB busy LED state; when TRUE the LED will flash once using 30mS clock tick
static uint8_t hid_led_usb_activity = 0;
static uint8_t cdc_led_usb_activity = 0;
static uint8_t msc_led_usb_activity = 0;
static main_led_state_t hid_led_state = MAIN_LED_FLASH;
static main_led_state_t cdc_led_state = MAIN_LED_FLASH;
static main_led_state_t msc_led_state = MAIN_LED_FLASH;

// Global state of usb
main_usb_connect_t usb_state;
static bool usb_test_mode = false;



/*******************************************************************************
							函数声明
*******************************************************************************/
// Functions called from other tasks to trigger events in the main task
// parameter should be reset type??
void main_reset_target(uint8_t send_unique_id)
{
    os_evt_set(FLAGS_MAIN_RESET, main_task_id);
    return;
}

// Flash HID LED using 30mS tick
void main_blink_hid_led(main_led_state_t permanent)
{
    hid_led_usb_activity = 1;
    hid_led_state = (permanent) ? MAIN_LED_FLASH_PERMANENT : MAIN_LED_FLASH;
    return;
}

// Flash CDC LED using 30mS tick
void main_blink_cdc_led(main_led_state_t permanent)
{
    cdc_led_usb_activity = 1;
    cdc_led_state = (permanent) ? MAIN_LED_FLASH_PERMANENT : MAIN_LED_FLASH;
    return;
}

// Flash MSC LED using 30mS tick
void main_blink_msc_led(main_led_state_t permanent)
{
    msc_led_usb_activity = 1;
    msc_led_state = (permanent) ? MAIN_LED_FLASH_PERMANENT : MAIN_LED_FLASH;
    return;
}

// Power down the interface
void main_powerdown_event(void)
{
    os_evt_set(FLAGS_MAIN_POWERDOWN, main_task_id);
    return;
}

// Disable debug on target
void main_disable_debug_event(void)
{
    os_evt_set(FLAGS_MAIN_DISABLEDEBUG, main_task_id);
    return;
}

//// Send next hid packet
//void main_hid_send_event(void)
//{
//    os_evt_set(FLAGS_MAIN_HID_SEND, main_task_id);
//    return;
//}

// Start CDC processing
void main_cdc_send_event(void)
{
    os_evt_set(FLAGS_MAIN_CDC_EVENT, main_task_id);
    return;
}

void main_usb_set_test_mode(bool enabled)
{
    usb_test_mode = enabled;
}

void USBD_SignalHandler()
{
    isr_evt_set(FLAGS_MAIN_PROC_USB, main_task_id);
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
    uint8_t i,key_value;
    // State processing
    uint16_t flags = 0;
    // LED
//    gpio_led_state_t hid_led_value = GPIO_LED_OFF;
//    gpio_led_state_t cdc_led_value = GPIO_LED_OFF;
//    gpio_led_state_t msc_led_value = GPIO_LED_OFF;
    // USB
    uint32_t usb_state_count = USB_BUSY_TIME;
    uint32_t usb_no_config_count = USB_CONFIGURE_TIMEOUT;
    // thread running after usb connected started
    uint8_t thread_started = 0;
    // button state
    uint8_t reset_pressed = 0;
    // Initialize settings - required for asserts to work
//    config_init();
    // Update bootloader if it is out of date
//    bootloader_check_and_update();
    // Get a reference to this task
    main_task_id = os_tsk_self();
    // leds
//    gpio_init();
    // Turn off LED
//    gpio_set_hid_led(GPIO_LED_OFF);
//    gpio_set_cdc_led(GPIO_LED_OFF);
//    gpio_set_msc_led(GPIO_LED_OFF);
    // Initialize the DAP
    DAP_Setup();
    //add by 9527
//    eslink_init();
    // do some init with the target before USB and files are configured
//    prerun_board_config();
//    prerun_target_config();
    // Update versions and IDs
//    info_init();
    // USB
    usbd_init();
//    vfs_mngr_fs_enable(true);
    usbd_connect(0);
    usb_state = USB_CONNECTING;
    usb_state_count = USB_CONNECT_DELAY;
    // Start timer tasks
    os_tsk_create_user(timer_task_30mS, TIMER_TASK_30_PRIORITY, (void *)stk_timer_30_task, TIMER_TASK_30_STACK);

    // Start key tasks
    os_tsk_create_user(key_task, KEY_TASK_10_PRIORITY, (void *)stk_key_task, KEY_TASK_STACK);
//    os_tsk_create_user(display_task, KEY_TASK_10_PRIORITY, (void *)stk_key_task, KEY_TASK_STACK);
    while (1) {
        // need to create a new event for programming failure
        os_evt_wait_or(FLAGS_MAIN_RESET             // Put target in reset state
                       | FLAGS_MAIN_90MS            // 90mS tick
                       | FLAGS_MAIN_30MS            // 30mS tick
                       | FLAGS_MAIN_POWERDOWN       // Power down interface
                       | FLAGS_MAIN_DISABLEDEBUG    // Disable target debug
                       | FLAGS_MAIN_PROC_USB        // process usb events
                       | FLAGS_MAIN_CDC_EVENT       // cdc event
                       , NO_TIMEOUT);
        // Find out what event happened
        flags = os_evt_get();

        if (flags & FLAGS_MAIN_PROC_USB) {
            if (usb_test_mode) {
                // When in USB test mode Insert a delay to
                // simulate worst-case behavior.
                os_dly_wait(1);
            }
            USBD_Handler();
        }


        if (flags & FLAGS_MAIN_RESET) {
//            target_set_state(RESET_RUN);
        }

        if (flags & FLAGS_MAIN_POWERDOWN) {
            // Disable debug
//            target_set_state(NO_DEBUG);
//            // Disable board power before USB is disconnected.
//            gpio_set_board_power(false);
            // Disconnect USB
            usbd_connect(0);
//            // Turn off LED
//            gpio_set_hid_led(GPIO_LED_OFF);
//            gpio_set_cdc_led(GPIO_LED_OFF);
//            gpio_set_msc_led(GPIO_LED_OFF);

            // TODO: put the interface chip in sleep mode
            while (1);
        }

        if (flags & FLAGS_MAIN_DISABLEDEBUG) {
            // Disable debug
//            target_set_state(NO_DEBUG);
        }

        if (flags & FLAGS_MAIN_CDC_EVENT) {
//            cdc_process_event();
        }

        if (flags & FLAGS_MAIN_90MS) {
            // Update USB busy status
//            vfs_mngr_periodic(90); // FLAGS_MAIN_90MS

            // Update USB connect status
            switch (usb_state) {
                case USB_DISCONNECTING:
                    usb_state = USB_DISCONNECTED;
                    // Disable board power before USB is disconnected.
//                    gpio_set_board_power(false);
                    usbd_connect(0);
                    break;

                case USB_CONNECTING:
                    // Wait before connecting
                    if (DECZERO(usb_state_count) == 0) {
                        usbd_connect(1);
                        usb_state = USB_CHECK_CONNECTED;
                        // Reset connect timeout
                        usb_no_config_count = USB_CONFIGURE_TIMEOUT;
                    }

                    break;

                case USB_CHECK_CONNECTED:
                    if (usbd_configured()) {
                        
                        if (!thread_started) {
//                            os_tsk_create_user(icd_process_continue, ES_TASK_PRIORITY, (void *)stk_es_task, ES_TASK_STACK);
                            // Start offline tasks
//                            os_tsk_create_user(offline_process_event, OFFLINE_TASK_PRIORITY, (void *)stk_offline_task, OFFLINE_TASK_STACK);
                            thread_started = 1;
                        }
//                        // Let the HIC enable power to the target now that high power has been negotiated.
//                        gpio_set_board_power(true);

                        usb_state = USB_CONNECTED;
           

                    }
                    else if (DECZERO(usb_no_config_count) == 0) {
                        // USB configuration timed out, which most likely indicates that the HIC is
                        // powered by a USB wall wart or similar power source. Go ahead and enable
                        // board power.
//                        gpio_set_board_power(true);
                    }

                    break;

                case USB_CONNECTED:
                case USB_DISCONNECTED:
                default:
                    break;
            }
        }

        // 30mS tick used for flashing LED when USB is busy
        if (flags & FLAGS_MAIN_30MS) {
            if (key_read_data(&key_value)){
                if(key_value == 1)
                {                       
	                 os_sem_send (&key_sem);                     
                }

            }
                
            
            
//                        // handle reset button without eventing
//            if (!reset_pressed && gpio_get_reset_btn_fwrd()) {
//                // Reset button pressed
//                target_set_state(RESET_HOLD);
//                reset_pressed = 1;
//            } else if (reset_pressed && !gpio_get_reset_btn_fwrd()) {
//                // Reset button released
//                target_set_state(RESET_RUN);
//                reset_pressed = 0;
//            }
 //           if (msc_led_usb_activity && ((msc_led_state == MAIN_LED_FLASH) || (msc_led_state == MAIN_LED_FLASH_PERMANENT))) {
                // Flash MSD LED ONCE
//                msc_led_value = (GPIO_LED_ON == msc_led_value) ? GPIO_LED_OFF : GPIO_LED_ON;
//                msc_led_usb_activity = ((GPIO_LED_ON == msc_led_value) && (MAIN_LED_FLASH == msc_led_state)) ? 0 : 1;
//                // Update hardware
//                gpio_set_msc_led(msc_led_value);
            
//            }
            if(30 == i++)
            {
                i = 0;
                LED_YELLOW_TOGGLE();
               
            }
                
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
//    uint8_t temp[512];   //    
    
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    gpio_init();
    gpio_set_trget_power(TRGET_POWER_3V3);
    

//    bsp_Init();
//    	DAP_Setup();
//    ISP_Setup();            //添加到握手成功后
//PORT_ISP_SETUP();
//PORT_SWD_SETUP();
//    PIN_SWDIO_OUT_ENABLE();

//    usbd_connect(1);

    spi_flash_init();
    key_init();
    mount_filesystem();
    fs_file_find();
    
    //flash test
//    memset (temp, 0x22, 512);  
//    sf_erase_sector(0x00,0x2000);
//    spi_flash_read (0x00,temp,512);
//    spi_flash_read (0x1000,temp,512);
//    memset (temp, 0x44, 512);   
//    sf_page_write(0x00,temp,512);
//    memset (temp, 0x55, 512);      
//    spi_flash_read (0x00,temp,512);
//    
//        memset (temp, 0x22, 512);  
//    spi_flash_read (0x1000,temp,512);
//    memset (temp, 0x44, 512);   
//    sf_page_write(0x1000,temp,512);
//    memset (temp, 0x55, 512);      
//    spi_flash_read (0x1000,temp,512);
        
    while (1)
    {

        os_sys_init_user(main_task, MAIN_TASK_PRIORITY, stk_main_task, MAIN_TASK_STACK);//        /* 创建启动任务 */

    }
}


/* 10KB的内部SRAM初始化为动态内存池 */
//uint64_t AppMalloc[10*1024/8];       
/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
//	/* 创建信号量计数值是0, 用于任务同步 */
//	os_sem_init (&semaphore, 0);
	
//	 /* 创建消息邮箱 */
//	os_mbx_init (&mailbox, sizeof(mailbox));
	
	/* 从内部SRAM分配100KB给动态内存使用 */	
//	os_init_mem(AppMalloc, 1024*80);


}

// Timer task, set flags every 30mS and 90mS
__task void timer_task_30mS(void)
{
    uint8_t i = 0;
    os_itv_set(3); // 30mS

    while (1) {
        os_itv_wait();
        os_evt_set(FLAGS_MAIN_30MS, main_task_id);

        if (!(i++ % 3)) {
            os_evt_set(FLAGS_MAIN_90MS, main_task_id);
        }
    }
}



/*******************************************************************************
*	函 数 名: key_task
*	功能说明: 按键扫描任务
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 5   优先级需要根据实际情况改变        
*******************************************************************************/
__task void key_task(void)
{
            /* 创建信号量计数值是0, 用于任务同步 */
//        os_sem_init (&key_sem, 0);
    os_itv_set(2); //周期唤醒的时间间隔:20mS
    while(1)
    {
		/* 按键扫描 */
		key_scan();
        os_itv_wait (); 
    }
}


/*
********************************************************************************
*	函 数 名: gui_refresh_task
*	功能说明: gui刷新函数
*	形    参: 无
*	返 回 值: 无
*   优 先 级: 3  优先级较低
********************************************************************************

*/
static void display_task(void )
{     
//	eslink_display();	
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
    printf("psr: 0x%08x\r\n", exception_stack->psr);
    printf(" pc: 0x%08x\r\n", exception_stack->pc);
    printf(" lr: 0x%08x\r\n", exception_stack->lr);
    printf("r12: 0x%08x\r\n", exception_stack->r12);
    printf("r03: 0x%08x\r\n", exception_stack->r3);
    printf("r02: 0x%08x\r\n", exception_stack->r2);
    printf("r01: 0x%08x\r\n", exception_stack->r1);
    printf("r00: 0x%08x\r\n", exception_stack->r0);
}


void HardFault_Handler()
{
//    util_assert(0);
//    SystemReset();
     printf("\r\n HardFault_Handler interrupt!\r\n");
    rt_hw_hard_fault_exception((struct exception_stack_frame *)__get_PSP());
    while (1); // Wait for reset
}
  
