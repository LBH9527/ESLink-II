#include "eslink.h"
//#include "iap_flash_intf.h"
#include "config_rom_set.h"
#include "update.h"
                    
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

static void int2array(uint8_t *res, uint32_t *data, uint8_t len) {
    uint8_t i = 0;

    for (i = 0; i < len; i+=4) {
        res[i]     = *data  & 0xff;
        res[i + 1] = (*data >> 8 ) & 0xff;
        res[i + 2] = (*data >> 16) & 0xff;
        res[i + 3] = (*data >> 24) & 0xff;
        data++;
    }
}    

//设置目标芯片电压
void eslink_set_trget_power(trget_power_t power)
{
    if(power == TRGET_POWER_DISABLE){
        V33_OFF();     
        V5_OFF();        
    }else if(power == TRGET_POWER_3V3){
        V33_ON();       
        V5_OFF();                 
    }else if(power == TRGET_POWER_5V){
        V33_OFF();       
        V5_ON();      
    }        
}

trget_power_t eslink_get_trget_power()
{
    
    
}    
//void flash_set_target_config(flash_t *obj)
/*
 *  读固件版本
 *  固件版本不对应，则更新固件
 */
static error_t read_offline_version(uint8_t *buf)
{   
    error_t result = ERROR_SUCCESS;     
    if(get_offline_info(buf) != sizeof(offline_info_t) )
        result = ERROR_IAP_READ;
    
    return result;  
}

static error_t download_offline_hex(uint8_t *data)    
{
    error_t result = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;
    
    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);
    if (update_app_program(UPDATE_OFFLINE_APP, addr, (data+8), size ) != 0)
//        result =  ERROR_IAP_PROG;
        result = ERROR_SUCCESS ;
    return result;    
}
    
static error_t download_offline_hex_end(uint8_t *data)    
{
    error_t result = ERROR_SUCCESS;
    uint32_t checksum;
    
    get_update_app_checksum(&checksum);    
    if( ( *data != (checksum & 0xff ))            |  
        ( *(data+1) != ((checksum >> 8) & 0xff )) |
        ( *(data+2) != ((checksum >> 16) & 0xff)) |
        ( *(data+3) != ((checksum >> 24) & 0xff)) )
        result = ERROR_IAP_WRITE;
    return result;    
}

/*
 *  读芯片时序信息
 */
error_t read_timinginfo(uint8_t *buf)
{
    error_t result = ERROR_SUCCESS;
    uint8_t len ;
    len = sizeof(timing_info_t) + sizeof(es_target_cfg) ;
    
    if(get_timing_info(buf) != len )
        result = ERROR_IAP_READ;
//    if(get_target_info(buf) != sizeof(es_target_cfg) )
//        result = ERROR_IAP_READ_TIMING_INFO;    
    return result;  
}

/*
 *  时序信息下载    
 *  下载时序信息并跟新APP
 */
error_t download_timinginfo(uint8_t *data)
{
    error_t result = ERROR_SUCCESS;

    if(set_timing_info_and_update(data) != TRUE )
        result = ERROR_IAP_WRITE;

    return result;  
}
/*
 *  时序下载    
 */
error_t download_timing(uint8_t *data)
{
    error_t result = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;

    addr = (*(data+0) <<  0) |
            (*(data+1) <<  8) |
            (*(data+2) << 16) |
            (*(data+3) << 24);
    size = (*(data+4) <<  0) |
            (*(data+5) <<  8) |
            (*(data+6) << 16) |
            (*(data+7) << 24);
    if(update_app_program(UPDATE_LINK_APP, addr, (data+8), size) != TRUE)
        result = ERROR_IAP_WRITE;
    
    return result;            
}
/*
 *  时序下载完成    
 */
error_t download_timing_end( uint8_t *data)
{
    error_t result = ERROR_SUCCESS;
    uint32_t checksum;
    
    update_app_program_end();
    get_update_app_checksum(&checksum);    
    *data = (checksum & 0xff ) ;          
    *(data+1) = (checksum >> 8) & 0xff ;
    *(data+2) = (checksum >> 16) & 0xff;
    *(data+3) = (checksum >> 24) & 0xff; 

    return result;      
    
} 


prog_comm_frame_t prog_data; 

uint32_t prog_process_command(uint8_t *request, uint8_t *response)
{
    uint8_t temp;
    uint8_t result = ERROR_SUCCESS;
    uint32_t ack_len = 512;         //isp通信采用变长协议，用来返回回复数据的长度
    uint32_t sum;
    
    prog_data.wrbuf = request;
    prog_data.rdbuf = response;     

    if( (*request != 0xBA) | (*(request+1) != 0xDC) | (*(request+2) != 0xcd) | (*(request+3) != 0xAB) )
    {
        result = ERROR_FRAME_HEAD;
        goto __ACK;
    }
    if(*(request+4) != ESLINKII_DEVICE_TYPE){
        result = ERROR_DEVICE_TYPE;
        goto __ACK;
    }

    prog_data.device_type   = *(request+4);
    prog_data.fun_code      = *(request+5);
    prog_data.data_length   = (*(request+6) << 0) | (*(request+7) << 8);
    
    memset(prog_data.rdbuf, 0, PROG_FRAME_SIZE);   
    
    switch(prog_data.fun_code){   
        case ID_HANDSHAKE:
            prog_data.data_length = 0;
            break;
        //------------------------Boot判断--------------------------------
        case ID_READ_BOOT_VERSION:           //0xD7 读脱机工程版本 
            #if 0
            result = read_offline_version(&prog_data.rdbuf[8]);
            prog_data.data_length = sizeof(offline_info_t);
            #else
            //add by 9527 for test: 设置固件版本为固定值
            prog_data.data_length = 4;
            prog_data.rdbuf[8] = 0x01;
            prog_data.rdbuf[9] = 0x01;
            prog_data.rdbuf[10] = 0x00;
            prog_data.rdbuf[11] = 0x00;
            #endif
            break;                     
        case ID_DL_BOOT_START:               //0xD8  脱机工程下载开始 
            update_app_init(UPDATE_OFFLINE_APP);
            prog_data.data_length = 0;    
            break;
        case ID_DL_BOOT_HEX:                 //0xD9 下载脱机工程HEX  //1024
            result =  download_offline_hex( &prog_data.wrbuf[8]);
            ack_len = 1024;
            prog_data.data_length = 0;    
            break;
        case ID_DL_BOOT_HEX_END:              //0xDA 下载结束  
            result =  download_offline_hex_end( &prog_data.wrbuf[8]);  
            prog_data.data_length = 0;                
            break;
        //------------------------时序判断--------------------------------
        //读flash中的数据，与上位机选择的目标芯片信息做比较，如果不一致则更新时序。
        case ID_READ_TIMING_INFO:               //0x28 读芯片信息
            result =  read_timinginfo(&prog_data.rdbuf[8]);
            prog_data.data_length = sizeof(es_target_cfg)+sizeof(timing_info_t);     
            break; 
         case ID_DL_TIMING_INFO:                //0x27             
            result = download_timinginfo(&prog_data.wrbuf[8]);
            prog_data.data_length = 0; 
            break;              
        //上位机判断时序需要更新，下发新的时序，在APP中擦除时序信息，跳转到BOOT中
        case ID_DL_TIMING_START:                    //0x31.下载时序开始
            
            prog_data.data_length = 0; 
            break;           
        case ID_DL_TIMING_ING:                      //0x32
            result =  download_timing( &prog_data.wrbuf[8]);
            ack_len = 1024;
            prog_data.data_length = 0;    
            break;          
        case ID_DL_TIMING_END:                      //0x33.时序下载结束
            result =  download_timing_end( &prog_data.rdbuf[8]);           
            prog_data.data_length = 4;  
            break;
        default:
            break;
    }  
//    memset(prog_data.data_buf, 0, ISP_PACKET_SIZE);      
    __ACK:   
    prog_data.rdbuf[0] = 0xBA;
    prog_data.rdbuf[1] = 0xDC;
    prog_data.rdbuf[2] = 0xCD;
    prog_data.rdbuf[3] = 0xAB;
    prog_data.rdbuf[4] = ESLINKII_DEVICE_TYPE;
    prog_data.rdbuf[5] = prog_data.fun_code;
    prog_data.state = ERROR_ESLINK_RUN_OK; 
    if(result != ERROR_SUCCESS)
    {
        if(result == ERROR_FRAME_HEAD)
        {
            prog_data.rdbuf[5] = 0x00;
            prog_data.data_length = 0x00;    
        } 
        prog_data.state = result;
    }   
    prog_data.rdbuf[6] = (uint8_t)(prog_data.data_length&0xFF);    
    prog_data.rdbuf[7] = (uint8_t)(prog_data.data_length>>8); 
    prog_data.rdbuf[prog_data.data_length+8] = prog_data.state;    
    prog_data.checksum = check_sum(prog_data.data_length +9, prog_data.rdbuf);
    prog_data.rdbuf[prog_data.data_length + 9] = (uint8_t)(prog_data.checksum&0xFF);            
    prog_data.rdbuf[prog_data.data_length + 10] = (uint8_t)(prog_data.checksum>>8); 
    
    return ack_len;
}
