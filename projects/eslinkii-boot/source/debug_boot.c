#include "eslink.h"  
#include "settings_rom.h"
#include "update.h"
#include "info.h"   

/*
 *  读芯片信息
 */
error_t read_hr_chipinfo(uint8_t *buf)
{
    error_t result = ERROR_SUCCESS;
    uint8_t len ;
    len = sizeof(hr_target_cfg)  ;
    
    if(get_hr_target_info(buf+FRAME_CODE_OFFSET) != len )
        result = ERROR_IAP_READ;  
    return result;  
}
/*
 *  下载芯片信息，跳转到APP
 */
static error_t download_hr_chipinfo(uint8_t *data)	
{
    error_t result = ERROR_SUCCESS;
    uint32_t size;
    
    size = (data[0] << 8) | data[1];
    if(size != sizeof(hr_target_cfg))
        result = ERR_CHECKSUM ;
    if(set_hr_timing_info(data+FRAME_CODE_OFFSET) != TRUE )
        result = ERROR_IAP_WRITE;     
    
    if(set_app_update(UPDATE_LINK_APP) != TRUE )
        result = ERROR_IAP_WRITE; 
        //跳转前关闭USB中断，不回复上位机数据，上位机会在跳转前延时1.5s发送下条命令
    USB0->INTEN  = 0;
    return result;       
}

/*
 *  时序下载    
 */
static error_t download_timing(uint8_t *data)
{
    error_t result = ERROR_SUCCESS;
    uint32_t addr;
    uint32_t size;

    size = (data[0] << 8) | data[1];
    addr = (data[2] <<  24) |
            (data[3] << 16) |
            (data[4] << 8) |
            (data[5] << 0);
        
    if(update_app_program(UPDATE_LINK_APP, addr, (data+9), size) != TRUE)
        result = ERROR_IAP_WRITE;
        
    
    return result;            
}
/*
 *  时序下载完成    
 */
static error_t download_timing_end( uint8_t *data)
{
    error_t result = ERROR_SUCCESS;
    uint32_t checksum;
    uint8_t data_temp[0x2C] = {0x00};
    
    update_app_program_end();
    get_update_app_checksum(&checksum);       
     
    data[0] = (checksum >> 8) & 0xff ;
    data[1] = (checksum & 0xff ) ;    
    //boot启动时，需要判断checksun，ide调试时的芯片信息没有校验和，把校验和保存在isp芯片信息的位置，
    data_temp[8] = data[1];
    data_temp[9] = data[0];
    if(set_timing_info(data_temp) != TRUE )
        result = ERROR_IAP_WRITE;    
    return result;      
    
} 

debug_comm_frame_t dbg_data;

uint32_t debug_process_command(uint8_t *request, uint8_t *response)     
{
//    uint16_t i;
    error_t result ;
    
    dbg_data.wrbuf = request;
    dbg_data.rdbuf = response; 
    
    dbg_data.frame_head = (request[0] << 24) | (request[1] << 16) | (request[2] << 8) | (request[3]);
    if(dbg_data.frame_head != DEBUG_FRAME_HEAD)
    {
        result = ERR_FRAME_HEAD;
        goto __ACK;
    }      
    dbg_data.data_length = request[4]<<8 | request[5];     
    if(request[6] != DEBUG_DEV_ADDR)
    {
        result = ERR_DEVICE_TYPE;
        goto __ACK;
    }
    dbg_data.device_addr = DEBUG_DEV_ADDR;   

    dbg_data.fun_code = request[7];    
//    if(check_sum( dbg_data.data_length -2, request) !=    \
//        ((request[dbg_data.data_length-2] << 8) | request[dbg_data.data_length-1]))
//    {      
//        result = ERR_CHECKSUM;
//        goto __ACK;
//    }    
    memset(dbg_data.rdbuf, 0, FRAME_ACK_NORMAL_LEN);
    dbg_data.rdbuf = response;
    
    switch(dbg_data.fun_code)
    {
        case ID_HANDSHAKE:      //0x01                
            dbg_data.rdbuf[FRAME_DATA_OFFSET] = 1;//com_interface;// communication mode
			dbg_data.rdbuf[FRAME_DATA_OFFSET+1] = (ESLINK_ICD_DEBUG_VERSION>>8)&0xff; //record software version	
			dbg_data.rdbuf[FRAME_DATA_OFFSET+2] = ESLINK_ICD_DEBUG_VERSION&0xff;//
			dbg_data.rdbuf[FRAME_DATA_OFFSET+3] = 0x01;// record hardware version	
			dbg_data.rdbuf[FRAME_DATA_OFFSET+4] = 0x00;//
			dbg_data.rdbuf[FRAME_DATA_OFFSET+5] = 0x00;// record FPGA version	
			dbg_data.rdbuf[FRAME_DATA_OFFSET+6] = 0x00;//
			dbg_data.rdbuf[FRAME_DATA_OFFSET+7] = 0x00;// record FPGA version	
			dbg_data.rdbuf[FRAME_DATA_OFFSET+8] = 0x00;  
            
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            result = ERROR_SUCCESS;
            break;
        case ID_READ_CHIP_INFO:        //0x02   读芯片信息
            result = read_hr_chipinfo(&dbg_data.rdbuf[FRAME_DATA_OFFSET]);
             dbg_data.data_length = FRAME_ACK_NORMAL_LEN + 512;
            break;
        case ID_DL_CHIP_INFO:           //0x03 下载芯片信息
            result = download_hr_chipinfo(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);  
            result  =   ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            break;
        case ID_DL_SCHEDULE_HEX_START:
            result = ERROR_SUCCESS;
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            break;
        case ID_DL_SCHEDULE_HEX:        //0x04 下载时序文件 
            result = download_timing(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);  
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            break;
        case DL_SCHEDULE_HEX_END:       //0x05 时序下载完毕
            result  = download_timing_end(&dbg_data.rdbuf[FRAME_DATA_OFFSET]);
            dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
            break; 
        default:
            result = ERROR_FUN_CODE ;    
            break;
        
    }
    __ACK:   
    
    dbg_data.rdbuf[0] = 0xAB;
    dbg_data.rdbuf[1] = 0xCD;
    dbg_data.rdbuf[2] = 0xDC;
    dbg_data.rdbuf[3] = 0xBA;  
    dbg_data.rdbuf[4] = dbg_data.data_length >> 8;
    dbg_data.rdbuf[5] = dbg_data.data_length & 0xFF;
    dbg_data.rdbuf[6] = dbg_data.device_addr;
    dbg_data.rdbuf[7] = dbg_data.fun_code;
    
    if(result != ERROR_SUCCESS)
    {
        dbg_data.device_state = result;   
        dbg_data.data_length = FRAME_ACK_NORMAL_LEN;
    }
    else
    {
        dbg_data.device_state = ERROR_ESLINK_RUN_OK;
    }
    
    dbg_data.rdbuf[dbg_data.data_length - 3] = dbg_data.device_state;
    dbg_data.checksum = check_sum(dbg_data.data_length-2,dbg_data.rdbuf); 
    dbg_data.rdbuf[dbg_data.data_length - 2] = (uint8_t)(dbg_data.checksum >> 8);
    dbg_data.rdbuf[dbg_data.data_length - 1] = (uint8_t) dbg_data.checksum; 
    
    return dbg_data.data_length;

    
 /* 
    // 先应答，在进行对目标芯片的操作    
    switch(dbg_data.fun_code){        
        case ID_EMU_RUN:                //0x21  全速运行 IDE上位启动
            result = ES_run();	
            break;
        case ID_EMU_ASM_STEP_OVER:      //0x23  汇编步越
            result = ES_asm_step_over();
            break;
        case EMU_stepout:
            result = ES_asm_step_out();	//步出 0x27
            break;
//        case C_step:                    //C单步 0x25
//            result = ES_C_step(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);		
//            break;
//        case C_stepover:
//            result = ES_C_stepover(&dbg_data.wrbuf[FRAME_DATA_OFFSET]);	
//            break;
        default:
            break;
        
    }
    */
}

