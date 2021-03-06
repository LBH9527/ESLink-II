#include "eslink_gpio.h"

void gpio_init(void)
{   
    /********************************POWER**************************************/
    PIN_SELECT_3V3_PORT->PCR[PIN_SELECT_3V3_BIT] = PORT_PCR_MUX(1);
    PIN_SELECT_5V_PORT->PCR[PIN_SELECT_5V_BIT] = PORT_PCR_MUX(1);
    V33_SELECT_INIT(1);         //connfig pin as 3.3V select  
    V5_SELECT_INIT(1);         //connfig pin as 5V select   
    /********************************LED**************************************/
    // configure pin as GPIO     
    LED_G_PASS_PORT->PCR[LED_G_PASS_PIN] = PORT_PCR_MUX(1);
    // configure pin as GPIO
    LED_Y_BUSY_PORT->PCR[LED_Y_BUSY_PIN] = PORT_PCR_MUX(1);
    // configure pin as GPIO
    LED_R_ERROR_PORT->PCR[LED_R_ERROR_PIN] = PORT_PCR_MUX(1);
    LED_GREEN_PASS_INIT(0);
    LED_YELLOW_BUSY_INIT(0);
    LED_RED_ERROR_INIT(0);    

    // start in configured as gpio input
    PIN_START_IN_GPIO->PDDR &= ~PIN_START_IN;
    PIN_START_IN_PORT->PCR[PIN_START_IN_BIT] = PORT_PCR_MUX(1); 
    
    /********************************BEEP**************************************/
    BEEP_PORT->PCR[BEEP_PIN] = PORT_PCR_MUX(1);    
    BEEP_INIT(0); 
    /********************************KEY**************************************/
    // key0 configured as gpio input
    PIN_KEY0_GPIO->PDDR &= ~PIN_KEY0;
    PIN_KEY0_PORT->PCR[PIN_KEY0_BIT] = PORT_PCR_MUX(1);    
    
   /********************************OLED**************************************/
    // configure pin as GPIO
    PIN_OLED_CS_PORT->PCR[PIN_OLED_CS_BIT] = PORT_PCR_MUX(1);
    // configure pin as GPIO
    PIN_OLED_RST_PORT->PCR[PIN_OLED_RST_BIT] = PORT_PCR_MUX(1);
    // configure pin as GPIO
    PIN_OLED_SCK_PORT->PCR[PIN_OLED_SCK_BIT] = PORT_PCR_MUX(1);
    // configure pin as GPIO
    PIN_OLED_SDA_PORT->PCR[PIN_OLED_SDA_BIT] = PORT_PCR_MUX(1);     
    OLED_RST_INIT(1);
    OLED_CS_INIT(1);
    OLED_SCK_INIT(1);
    OLED_SDA_INIT(1);  

    //CTL IO
    PIN_CTL_PORT->PCR[PIN_CTL_BIT] = PORT_PCR_MUX(1);    
    CLT_INIT(0);    
}

//脱机启动/退出信号
uint8_t gpio_start_in_low(void)
{
    return (PIN_START_IN_GPIO->PDIR & PIN_START_IN) ? 0 : 1;
}  

uint8_t gpio_key0_down(void)
{
    return (PIN_KEY0_GPIO->PDIR & PIN_KEY0) ? 0 : 1;
}



