#include "RTL.h"
#include "rl_usb.h"
#include "main.h"
#include "eslink.h"
#include "ES_ICD.h"


extern ICD_COMM_DATA_t icd_data;


#define CMD_HEADER_LEN        8           //协议头长度
//读数据状态

typedef enum {
    ESLINK_STATE_CLOSE,
    ESLINK_STATE_WRITE,
    ESLINK_STATE_READ,
    ESLINK_STATE_END,
    ESLINK_STATE_ERROR
} eslink_state_t;
static eslink_state_t cls_state;
static uint32_t cls_data_ops; 
static uint32_t cls_wr_cmd_len;                   //主机写命令码数据长度 
static uint32_t cls_rd_cmd_len;                   //主机读命令码数据长度 
uint8_t usb_es_write[ICD_COMM_PACKET_LEN]; //写数据buff         
uint8_t usb_es_read[ICD_COMM_PACKET_LEN];  //读数据buff

//函数声明
uint32_t eslink_process_command(uint8_t *write_buf, uint8_t *read_buf);
//uint32_t es_data_to_send_wr;
//uint32_t es_data_to_send_rd;
//uint32_t es_data_len;
//uint8_t es_data_access;         /*!< Flag active while read data (in the receive intermediate buffer) is being accessed */

void USBD_CLS_init(void)
{
    cls_wr_cmd_len = 0;
    cls_data_ops = 0; 
    USBD_CLS_WriteBuf =  usb_es_write;
    USBD_CLS_ReadBuf = usb_es_read;
    cls_state =   ESLINK_STATE_CLOSE;
    
}
void USBD_CLS_Reset_Event(void)
{
    cls_wr_cmd_len = 0;
    cls_rd_cmd_len = 0;
    cls_data_ops = 0; 
    cls_state =   ESLINK_STATE_CLOSE;
    
}

//接收到协议规定的长度后，处理数据，更新设备状态。
eslink_state_t cls_packet_write(uint8_t *data, uint8_t len)
{
    uint32_t n;
    
    for (n = 0; n < len; n++) 
    {
        usb_es_write[cls_data_ops + n] = data[n];
    }   
    cls_data_ops += len;
    cls_wr_cmd_len -= len;  
    
    if(cls_data_ops >= ICD_COMM_PACKET_LEN)
        USBD_CLS_Reset_Event();
    if ((cls_wr_cmd_len == 0) && (cls_data_ops != 0) )
    {
        cls_data_ops = 0;
        return  ESLINK_STATE_READ;
    }
        
    return ESLINK_STATE_WRITE;    
}
static uint32_t get_cmd_data_len( uint8_t *data)
{
    uint32_t header;
    uint32_t cmd_len;

    header = (data[0] << 24) |
                (data[1] << 16) |
                (data[2] <<  8) |
                (data[3] <<  0);  
    if(header == ICD_FRAME_HEAD)  
    {
        cmd_len = (data[4] << 8) | data[5];         
    }  
    else if(header == 0xbadccdab)
    {
        if( (data[5] == ID_DL_USERHEX)                  |
            (data[5] == ID_DL_TIMING)                   | 
            (data[5] == ID_READ_FLASH)                  | 
            (data[5] == ID_DL_OFFLINE_PRJ_TIMING_ING)   | 
            (data[5] == ID_DL_OFFLINE_PRJ_HEX) )
            cmd_len = 1024;
        else
            cmd_len = 512;          
    }
    else
    {
        cmd_len = 0;          
    }   
    return cmd_len;
}



uint32_t USBD_CLS_DataOutTransfer( uint8_t *data, uint8_t len)
{       
//    uint8_t n;

    switch(cls_state)
    {
        case ESLINK_STATE_CLOSE:
            
            if(len < CMD_HEADER_LEN)
                break;
            cls_wr_cmd_len = get_cmd_data_len(data);  
            if((cls_wr_cmd_len >= ICD_COMM_PACKET_LEN) | ( cls_wr_cmd_len == 0))    //包长渡错误
               USBD_CLS_Reset_Event(); 
            cls_state = cls_packet_write(data, len) ;                              
            break;
        case ESLINK_STATE_WRITE:            
            cls_state = cls_packet_write(data, len) ;                     
            break;        
        default:
            cls_state = ESLINK_STATE_CLOSE;
            USBD_CLS_Reset_Event();
            break;          
    }
    if( cls_state ==  ESLINK_STATE_READ)
    {
        cls_rd_cmd_len = eslink_process_command(usb_es_write, usb_es_read); 
        return cls_rd_cmd_len;
//               
//            cls_state = ESLINK_STATE_READ;
//            if(cls_rd_cmd_len >= ICD_COMM_PACKET_LEN)
//                usbd_cls_uninit();
    }       
    return 0;    
}


uint32_t eslink_process_command(uint8_t *write_buf, uint8_t *read_buf)
{
    uint32_t header;
    uint32_t read_len;
    
    header = (write_buf[0] << 24) |
            (write_buf[1] << 16) |
            (write_buf[2] <<  8) |
            (write_buf[3] <<  0);        
    if(header == ICD_FRAME_HEAD) 
    {
        //调试命令处理
//        read_len = debug_process_command(write_buf, read_buf);
//        es_data_access = 1;        
    }   
    else if(header != ISP_FRAME_HEAD) 
    {
        //连接命令处理
        read_len = es_process_command(write_buf, read_buf);
    }
    else 
    {
        
        
    }
    return read_len;
    
}




////uint32_t usbd_cls_read_eslink(uint8_t *buf, int32_t len)
//uint32_t usbd_cls_read_eslink(void)
//{
//    if(es_data_access == 0)
//        return 0;
//    es_data_access = 0;
//    return es_data_len;
//    
//}
//void es_preprocess_event()
//{
//    int32_t len_data = 0;    
//    uint8_t n;
//    uint8_t data[64] = {0x55, 0xaa, 0x33, 0x44};
//    
//    uint32_t offset = 0;
//    

//    if( USBD_CLS_DataRead(data, 8) == 8)
//    {
//        for (n = 0; n < len_data; n++) 
//        {
//            usb_es_buffer[n+offset] = data[n];
//        }  
//        header = (usb_es_buffer[0] << 24) |
//            (usb_es_buffer[1] << 16) |
//            (usb_es_buffer[2] <<  8) |
//            (usb_es_buffer[3] <<  0);        
//        if( (header != ICD_FRAME_HEAD) || (header != ISP_FRAME_HEAD) )
//        {
//           
//                    
//        }    
//         offset += 8;    
//        while(USBD_CLS_DataRead(data, sizeof(data)))
//        {
//            for (n = 0; n < len_data; n++) 
//            {
//                usb_es_buffer[n+offset] = data[n];
//            }                 
//        }    
//        if(header == ICD_FRAME_HEAD)   
//        {
//            icd_process_command(usb_es_buffer);
//        }
//        else if(header == ISP_FRAME_HEAD)
//        {
//            
//        }
//    }   
//    // Always process events
//    main_es_send_event();
//}
////    
//__task void  es_process(void *argv)
//{
//    int32_t len_data = 0;
//    uint32_t offset = 0;
//    uint8_t n;
//    uint8_t data[64];
//    uint32_t header;

//    if( USBD_CLS_DataRead(data, 8) == 8)
//    {
//        for (n = 0; n < len_data; n++) 
//        {
//            usb_es_buffer[n+offset] = data[n];
//        }  
//        header = (usb_es_buffer[0] << 24) |
//            (usb_es_buffer[1] << 16) |
//            (usb_es_buffer[2] <<  8) |
//            (usb_es_buffer[3] <<  0);        
//        if( (header != ICD_FRAME_HEAD) || (header != ISP_FRAME_HEAD) )
//        {
//           
//                    
//        }    
//         offset += 8;    
//        while(USBD_CLS_DataRead(data, sizeof(data)))
//        {
//            for (n = 0; n < len_data; n++) 
//            {
//                usb_es_buffer[n+offset] = data[n];
//            }                 
//        }    
//        if(header == ICD_FRAME_HEAD)   
//        {
//            icd_process_command(usb_es_buffer);
//        }
//        else if(header == ISP_FRAME_HEAD)
//        {
//            
//        }
//    }
//                
//            
//            
//}

