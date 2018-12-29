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

//脱机启动/退出信号
uint8_t ofl_start_in_low(void)
{
    return (PIN_START_IN_GPIO->PDIR & PIN_START_IN) ? 0 : 1;
}


/*
 * 更新序列号到用户HEX中
 * sn: 序列号  addr：填入序列号的地址   data： HEX数据   size：hex大小
 */
void serial_number_intercept_write(serial_number_t *sn, uint32_t addr, uint8_t *data, uint32_t size)
{
    if( (sn->addr >= addr) &  ((sn->addr + sn->size) < addr+size) )
    {
         memcpy( &data[sn->addr - addr ] , sn->data, sn->size);    
    }     
    else if( (sn->addr > addr) & (sn->addr < (addr+size)) & ((sn->addr + sn->size) > addr + size) )
    {
         memcpy( &data[sn->addr - addr ] , sn->data, addr + size - sn->addr);     
    }
    else if( (sn->addr < addr) & ((sn->addr + sn->size) > addr) )
    {
         memcpy( data, &sn->data[addr -sn->addr ], sn->addr+sn->size - addr);   
    }    
}



void eslink_set_target_hold_reset(void)
{
    PIN_RST_OUT(0) ;
    es_set_trget_power(TRGET_POWER_DISABLE);
    es_delay_ms(10);
    es_set_trget_power(TRGET_POWER_ENABLE);

}  

void eslink_set_target_reset_run(uint8_t delay_ms)
{   
    PIN_RST_OUT(0) ;
    es_delay_ms(delay_ms);
    PIN_RST_OUT(1) ;  
}
