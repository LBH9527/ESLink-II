/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 :   任务设置：
*
*	修改记录 :
*       boot主程序，负责更新app和link程序。在boot中判断app和link程序的合法性。
*		版本号    日期         作者        说明
*
*
*********************************************************************************************************
*/	
#include "fsl_device_registers.h"

#include "board.h"
#include "eslink_gpio.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h" 
#include "es_common.h" 
#include "main.h" 
#include "RTL.h"
#include "rl_usb.h"
#include "settings_rom.h"
#include "uart.h"
//#include "spi_flash.h" 
//#include "key.h"  
//#include "oled.h"

#include "systick.h"
#include "iap_flash_intf.h"
#include "update.h"
/*******************************************************************************
							函数声明
*******************************************************************************/
static void delay_ms(uint32_t delay) 
{   
    delay *= ((SystemCoreClock/1000U) + (4-1U)) / 4;
    while (--delay);
}
/*******************************************************************************
								变量
*******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{        
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    
    settings_rom_init();
    get_offlink_app_version();
    //判断是否需要跳转到APP
    if(stay_in_bootloader() != TRUE)
    {
        //TODO:更新失败处理
         application_check_and_run();          
    }         
    
    gpio_init();
    es_set_trget_power(TRGET_POWER_3V3);
    bsp_init_systick();  
    usbd_init();                          /* USB Device Initialization          */
    usbd_connect(__TRUE);                 /* USB Device Connect                 */
    while (!usbd_configured ());          /* Wait for device to configure        */
    LED_GREEN_ON();
    while (1)
    {              
        if(stay_in_bootloader() != TRUE)
        {
            delay_ms(10);   //for USB ack 
            application_check_and_run();          
        } 	
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
    while (1); // Wait for reset
}

void main_10ms_task(void)
{
    
}
  
