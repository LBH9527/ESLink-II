#include "eslink.h"
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

////设置目标芯片电压
//void eslink_set_trget_power(trget_power_t power)
//{
//    if(power == TRGET_POWER_DISABLE){
//        V33_OFF();     
//        V5_OFF();        
//    }else if(power == TRGET_POWER_3V3){
//        V33_ON();       
//        V5_OFF();                 
//    }else if(power == TRGET_POWER_5V){
//        V33_OFF();       
//        V5_ON();      
//    }        
//}

//trget_power_t eslink_get_trget_power()
//{
//    
//    
//}  

/*
 * 更新序列号到用户HEX中
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

