
#include "eslink_gpio.h"

void gpio_init(void)
{
    // key0 configured as gpio input
    PIN_KEY0_GPIO->PDDR &= ~PIN_KEY0;
    PIN_KEY0_PORT->PCR[PIN_KEY0_BIT] = PORT_PCR_MUX(1);
    
    // start in configured as gpio input
    PIN_START_IN_GPIO->PDDR &= ~PIN_START_IN;
    PIN_START_IN_PORT->PCR[PIN_START_IN_BIT] = PORT_PCR_MUX(1);
    
    // configure pin as GPIO
    PIN_OLED_CS_PORT->PCR[PIN_OLED_CS_BIT] = PORT_PCR_MUX(1);
    // rst cler - enable output 
    PIN_OLED_RST_GPIO->PCOR = 1UL << PIN_OLED_RST_BIT;
    PIN_OLED_RST_GPIO->PDDR = 1UL << PIN_OLED_RST_BIT;
    
    //CTL IO
    PIN_CTL_GPIO->PCOR = 1UL << PIN_CTL_BIT;
    PIN_CTL_GPIO->PDDR = 1UL << PIN_CTL_BIT;
    
    CLT_INIT(1);
    //SWO TEST
    PIN_SWO_GPIO->PDDR &= ~PIN_SWO_BIT;
    
    V33_SELECT_INIT(1);         //connfig pin as 3.3V select  
    V5_SELECT_INIT(1);         //connfig pin as 5V select  
    
    LED_GREEN_PASS_INIT(1);
    LED_YELLOW_BUSY_INIT(1);
    LED_RED_ERROR_INIT(1);

    BEEP_INIT(0);    

    OLED_RST_INIT(1);
    OLED_CS_INIT(1);
    OLED_SCK_INIT(1);
    OLED_SDA_INIT(1);        
}


uint8_t gpio_get_reset_btn_no_fwrd(void)
{
    return (PIN_KEY0_GPIO->PDIR & PIN_KEY0) ? 0 : 1; 
}

uint8_t gpio_key0_down(void)
{
    return (PIN_KEY0_GPIO->PDIR & PIN_KEY0) ? 0 : 1;
}
//uint8_t gpio_key1_down(void)
//{
//    return (PIN_KEY0_GPIO->PDIR & PIN_KEY0) ? 0 : 1;
//}
//uint8_t gpio_key2_down(void)
//{
//    return (PIN_KEY0_GPIO->PDIR & PIN_KEY0) ? 0 : 1;
//}


void es_set_trget_power(trget_power_t power)
{
    if(power == TRGET_POWER_DISABLE)
    {
        V33_OFF();     
        V5_OFF();        
    }
    else if(power == TRGET_POWER_ENABLE)
    {
         V33_ON();    
         V5_ON();  
    }
    else if(power == TRGET_POWER_3V3)
    {
        V33_ON();       
        V5_OFF();                 
    }else if(power == TRGET_POWER_5V){
        V33_OFF();       
        V5_ON();      
    }        
}

//脱机启动/退出信号
uint8_t ofl_start_in_low(void)
{
    return (PIN_START_IN_GPIO->PDIR & PIN_START_IN) ? 0 : 1;
}


