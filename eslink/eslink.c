#include "eslink.h"
#include "settings_rom.h"   
// Delay for specified time
//    delay:  delay time in ms
void es_delay_ms(uint32_t delay) 
{
    delay *= ((SystemCoreClock/1000U) + (ES_DELAY_SLOW_CYCLES-1U)) / ES_DELAY_SLOW_CYCLES;
    ES_DELAY_SLOW(delay);
}

// Delay for specified time
//    delay:  delay time in us
void es_delay_us(uint32_t delay) 
{
    delay *= ((SystemCoreClock/1000000U) + (ES_DELAY_SLOW_CYCLES-1U)) / ES_DELAY_SLOW_CYCLES;
    ES_DELAY_SLOW(delay);
} 

//eslink 是否为mini 
uint8_t eslink_is_mini(void)
{
    uint32_t hw_version;
    uint8_t eslink_mode;
    
    hw_version = get_hardware_version();
    eslink_mode = (hw_version >> 16)& 0xff ;
    if( eslink_mode == ESLINK_MODE_MINI)
        return TRUE;
    return FALSE;    
}

//eslink 为脱机模式
uint8_t eslink_is_offline_mode(void) 
{
    if(get_link_mode() == LINK_OFFLINE_MODE)
        return TRUE;
    return FALSE;
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

