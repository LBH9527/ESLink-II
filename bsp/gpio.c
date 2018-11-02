
#include "eslink_gpio.h"

void gpio_init(void)
{
    // key0 configured as gpio input
    PIN_KEY0_GPIO->PDDR &= ~PIN_KEY0;
    PIN_KEY0_PORT->PCR[PIN_KEY0_BIT] = PORT_PCR_MUX(1);
//    // key1 configured as gpio input
//    PIN_KEY1_GPIO->PDDR &= ~PIN_KEY1;
//    PIN_KEY1_PORT->PCR[PIN_KEY0_BIT] = PORT_PCR_MUX(1);
//    // key2 configured as gpio input
//    PIN_KEY0_GPIO->PDDR &= ~PIN_KEY2;
//    PIN_KEY0_PORT->PCR[PIN_KEY0_BIT] = PORT_PCR_MUX(1);
    
    
    // configure pin as GPIO
    PIN_OLED_CS_PORT->PCR[PIN_OLED_CS_BIT] = PORT_PCR_MUX(1);
    // rst cler - enable output 
    PIN_OLED_RST_GPIO->PCOR = 1UL << PIN_OLED_RST_BIT;
    PIN_OLED_RST_GPIO->PDDR = 1UL << PIN_OLED_RST_BIT;
    
    // led on
    LED_CONNECTED_GPIO->PCOR = 1UL << LED_CONNECTED_BIT;
    
    
    V33_SELECT_INIT(1);         //connfig pin as 3.3V select  
    V5_SELECT_INIT(1);         //connfig pin as 5V select  
    
    LED_GREEN_INIT(1);
    LED_RED_INIT(1);

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
uint8_t gpio_key1_down(void)
{
    return (PIN_KEY0_GPIO->PDIR & PIN_KEY0) ? 0 : 1;
}
uint8_t gpio_key2_down(void)
{
    return (PIN_KEY0_GPIO->PDIR & PIN_KEY0) ? 0 : 1;
}


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

