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
#include "gpio.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"   
#include "main.h"  
#include "spi_flash.h"
//#include "uart.h"
#include "key.h"
#include "spi_flash_fs.h"  
#include "oled.h"
#include "eslink_addr.h"
#include "systick.h"

#include "fonts.h"
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
    
    SCB->VTOR = SCB_VTOR_TBLOFF_Msk & ESLINK_ROM_OFFLINE_START;    
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    gpio_init();
    gpio_set_trget_power(TRGET_POWER_3V3);
    bsp_init_systick();

    spi_flash_init();
    sf_read_info();
    key_init();
    oled_init();
//    mount_filesystem();
//    fs_file_find();
    
//    //flash test
//    spiflash_test( 0,0x800000);
    oled_display();
        
    while (1)
    {

          LED_GREEN_TOGGLE();
        bsp_delay_ms(1000);
        if (key_read_data(&key_value))
        {
             if (key_value == KEY0_DOWN)
             {
                 
                 
             }
             else if (key_value == KEY0_LONG)
             {
                 
                 
             }
            
        }  
    }
}

void main_10ms_task(void)
{       
    key_scan();     
}

void oled_display(void)
{
    FONT_T Font_log;
        FONT_T Font16;
    
    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
    Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
    Font16.Space = 0;			/* 文字间距，单位 = 像素 */	
    OLED_DispStr(0,0,"es_link", &Font16);
    OLED_DispStr(0,16,"选择方案", &Font16);
    Font_log.FontCode = FC_ST_24;	/* 字体代码 16点阵 */
//    Font_log.FrontColor = 1;		/* 字体颜色 0 或 1 */
//    Font_log.BackColor = 0;		/* 文字背景颜色 0 或 1 */
//    Font_log.Space = 0;			/* 文字间距，单位 = 像素 */	
//    oled_display_log(0,0,0);
//    oled_display_log(12,0,1);
//    oled_display_log(24,0,2);
//    oled_display_log(36,0,3);
//    oled_display_log(48,0,4);
//    oled_display_log(60,0,5);

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
  
