#include "eslink.h"
#include "program_port.h"
/*
 * �������кŵ��û�HEX��
 * sn: ���к�  addr���������кŵĵ�ַ   data�� HEX����   size��hex��С
 */
void serial_number_intercept_write(serial_number_t *sn, uint32_t addr, uint8_t *data, uint32_t size)
{        
    if( (sn->addr >= addr) &  ((sn->addr + sn->size) <= addr+size) )
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

void eslink_set_target_hold_reset(uint8_t delay_ms)
{
    PIN_RST_OUT(0) ;
    es_set_trget_power(TRGET_POWER_DISABLE);
    es_delay_ms(delay_ms);
    es_set_trget_power(TRGET_POWER_ENABLE);  
}  

void eslink_set_target_reset_run(uint8_t delay_ms)
{   
    PIN_RST_OUT(0) ;
    es_delay_ms(delay_ms);
    PIN_RST_OUT(1) ;  
}
void eslink_set_target_power_reset(uint8_t delay_ms)
{
    PIN_RST_OUT(1) ;  
    es_set_trget_power(TRGET_POWER_DISABLE);
    es_delay_ms(delay_ms);
    es_set_trget_power(TRGET_POWER_ENABLE); 
}


//void program_create_cb(struct es_prog_ops *intf, program_data_callback_t callback, void *userData)                                       
//{
//    intf->cb = callback;
//    intf->userData = userData;
//}   
