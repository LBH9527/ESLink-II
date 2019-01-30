/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块
*	文件名称 : main.c
*	版    本 : V1.0
*	说    明 :   任务设置：
*
*	修改记录 :
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
#include "main.h"  
#include "spi_flash.h"
#include "eeprom.h"
#include "eslink.h"
#include "key.h"
#include "beep.h" 
#include "oled.h"
#include "eslink_app_addr.h"
#include "systick.h"

#include "update_menu.h"
#include "eslink.h"
#include "offline_file.h"
#include "ofl_prj_decoder.h"
#include "settings_rom.h"
#include "info.h"
/*******************************************************************************
							函数声明
*******************************************************************************/
void oled_display(void);
/*******************************************************************************
								变量
*******************************************************************************/


/*******************************************************************************
							函数声明
*******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t key_value;
    uint8_t menu_msg ;
	
    SCB->VTOR = SCB_VTOR_TBLOFF_Msk & ESLINK_ROM_OFFLINE_START;    
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    settings_rom_init();
   
    gpio_init();  
    oled_init();
    key_init();  
    fm24cxx_init();
    spi_flash_init(); 
    //    sf_erase_chip();      
    ofl_file_init();
    menu_init();
    bsp_init_systick();
	
    LED_GREEN_ON();            
    while (1)
    {            
		if(key_read_data(&key_value) != 0)        //有按键按下
		{
		    switch (key_value)
			{
				case KEY_DOWN:
					menu_msg = MSG_KEY_DOWN; 					
				break;
				case KEY_ENTER:
                    beep_key_press();
					menu_msg = MSG_KEY_ENTER; 
				break;
				default:
				break;
			}  
			gui_msg_write_data(&menu_msg);			
		}
         menu_display(); 	 

//		bsp_delay_ms(1000);
    }   

}

void main_10ms_task(void)
{       
    key_scan(); 
    // 蜂鸣器扫描
    beep_scan();
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
  
