//#include "string.h"

//#include "RTL.h"
//#include "rl_usb.h"
//#include "usb_for_lib.h"
//#include "ES_ICD.h"
//#include "ES_ISP.h"
//#include "fsl_common.h"


///* Dummy Weak Functions that need to be provided by user */
//__weak void usbd_cls_init(void)
//{

//}
//__weak uint32_t usbd_cls_eslink(uint8_t *write_buf, uint8_t *read_buf)
//{
//    return 0;
//}

//__weak uint32_t cls_process_data_from_host( uint8_t *data, uint8_t len)
//{
//    return 0;    
//}
//__weak uint32_t cls_process_data_to_host( uint8_t data, uint8_t len)
//{
//    return 0;    
//}

//#define ESLINK_COMMON_HEADER_LEN        8           //协议头长度
//#define ESLINK_HEADER       0        
//#define ESLINK_DATA_OUT     1       
//#define ESLINK_DATA_IN      2       

//uint32_t eslink_out_data_len;               /* Bulk Out Length */
//uint32_t eslink_in_data_len;                /* Bulk In Length */

//uint32_t eslink_data_offset;  
//uint8_t eslink_stage;
//uint32_t len_received;

//void USBD_CLS_Reset_Event(void)
//{
//    eslink_stage = ESLINK_HEADER;
//    eslink_out_data_len = 0;
//    eslink_in_data_len = 0;
//    eslink_data_offset = 0;
//}

//void USBD_CLS_DataInTransfer(void)
//{
//    uint32_t send_size;
//    uint32_t n;
//    
////    eslink_data_offset = 0;
//    if (eslink_in_data_len != 0) 
//    {
//        send_size = MIN(eslink_in_data_len, usbd_cls_buf_sz);
//        for (n = 0; n < send_size; n++) 
//        {
//            USBD_CLS_BulkBuf[n] = USBD_CLS_ReadBuf[n+eslink_data_offset];
//        }
//        USBD_WriteEP(usbd_cls_ep_bulkin | 0x80, USBD_CLS_BulkBuf, send_size);
//        eslink_data_offset += send_size;
//        eslink_in_data_len -= send_size;
//        if(eslink_data_offset >= ICD_COMM_PACKET_LEN)
//            USBD_CLS_Reset_Event();
//    }     
//    if(eslink_in_data_len == 0)
//    {
//        USBD_CLS_Reset_Event();
//    }    
//}

//void USBD_CLS_ESLINK_Write(void)
//{
//    uint32_t n;
//    
//    for (n = 0; n < len_received; n++) 
//    {
//        USBD_CLS_WriteBuf[eslink_data_offset + n] = USBD_CLS_BulkBuf[n];
//    }   
//    eslink_data_offset += len_received;
//    eslink_out_data_len -= len_received;  
//    
//    if(eslink_data_offset >= ICD_COMM_PACKET_LEN)
//        USBD_CLS_Reset_Event();
//    if (len_received) 
//    {
//        if ((eslink_out_data_len == 0) && (eslink_data_offset != 0)) 
//        {
//            eslink_in_data_len = usbd_cls_eslink(USBD_CLS_WriteBuf, USBD_CLS_ReadBuf);
//            eslink_data_offset = 0;    
//            eslink_stage = ESLINK_DATA_IN;
//            if(eslink_in_data_len >= ICD_COMM_PACKET_LEN)
//                USBD_CLS_Reset_Event();
//        }
//    }
//    if( (eslink_out_data_len == 0) && (eslink_stage == ESLINK_DATA_IN) )
//    {
//        eslink_data_offset = 0;    
//        USBD_CLS_DataInTransfer();
//    }
//        
//}

//void USBD_CLS_BulkOut(void )
//{
//    uint32_t header;
//    cls_process_data_from_host(USBD_CLS_BulkBuf, len_received);
//    
// /*   switch(eslink_stage)
//    {
//        case ESLINK_HEADER:
//            if(len_received < ESLINK_COMMON_HEADER_LEN)
//                break;
//            header = (USBD_CLS_BulkBuf[0] << 24) |
//            (USBD_CLS_BulkBuf[1] << 16) |
//            (USBD_CLS_BulkBuf[2] <<  8) |
//            (USBD_CLS_BulkBuf[3] <<  0);  
//            if(header == ICD_FRAME_HEAD)  
//            {
//                eslink_out_data_len = (USBD_CLS_BulkBuf[4] << 8) | USBD_CLS_BulkBuf[5];               
//                    
//                eslink_stage = ESLINK_DATA_OUT;  
//                eslink_data_offset  = 0;
//                
//                USBD_CLS_ESLINK_Write();
//            }  
//            else if(header == 0xbadccdab)
//            {
//                eslink_out_data_len = 512;
//                eslink_data_offset  = 0;
//                eslink_stage = ESLINK_DATA_OUT;
//                USBD_CLS_ESLINK_Write();
//            }
//            else
//            {
//                USBD_CLS_Reset_Event();
//                
//            }
//            if(eslink_out_data_len >= ICD_COMM_PACKET_LEN)
//                USBD_CLS_Reset_Event();
//            break;
//        case ESLINK_DATA_OUT:            
//            eslink_stage = ESLINK_DATA_OUT;
//            USBD_CLS_ESLINK_Write();
//        
//            break;
//        default:
//            eslink_stage = ESLINK_HEADER;
//            USBD_CLS_Reset_Event();
//            break;
//        
//        
//    }
//          
//*/
//    
//}

//void USBD_CLS_BulkIn(void)
//{
////    uint32_t send_size;
////    uint32_t n;
//    
//    if(eslink_stage != ESLINK_DATA_IN)
//        return;
//    USBD_CLS_DataInTransfer();
////    if (eslink_in_data_len) 
////    {
////        send_size = MIN(eslink_in_data_len, usbd_cls_buf_sz);
////        for (n = 0; n < send_size; n++) 
////        {
////            USBD_CLS_BulkBuf[n] = USBD_CLS_ReadBuf[n+eslink_data_offset];
////        }
////        USBD_WriteEP(usbd_cls_ep_bulkin | 0x80, USBD_CLS_BulkBuf, send_size);
////        eslink_data_offset += send_size;
////        eslink_in_data_len -= send_size;
////    } 
////    eslink_stage = ESLINK_HEADER;    
//}

//void USBD_CLS_EP_BULKIN_Event(uint32_t event)
//{
//    USBD_CLS_BulkIn();    
//}

//void USBD_CLS_EP_BULKOUT_Event(uint32_t event)
//{

//    len_received = USBD_ReadEP(usbd_cls_ep_bulkout, USBD_CLS_BulkBuf, usbd_cls_buf_sz);
//    USBD_CLS_BulkOut();    
//    
//}

//void USBD_CLS_EP_BULK_Event(uint32_t event)
//{
//    if (event & USBD_EVT_OUT) {
//        USBD_CLS_EP_BULKOUT_Event(event);
//    }

//    if (event & USBD_EVT_IN) {
//        
//        USBD_CLS_EP_BULKIN_Event(event);
//    }
//}

#include "RTL.h"
#include "rl_usb.h"
#include "usb_for_lib.h"   

uint8_t  len_rcv;
uint32_t len_send ;
uint32_t send_offset;

uint8_t *USBD_CLS_WriteBuf;
uint8_t *USBD_CLS_ReadBuf;

#if !defined(MIN)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

__weak void USBD_CLS_init(void)
{
//    return 0;    
}
//__weak uint32_t USBD_CLS_DataInTransfer( uint8_t *data, uint8_t len)
//{
//    return 0;    
//}
__weak uint32_t USBD_CLS_DataOutTransfer( uint8_t *data, uint8_t len)
{
    return 0;    
}

__weak void USBD_CLS_Reset_Event(void)
{
//    return 0;  
}
   
void USBD_CLS_DataInTransfer(void)
{
    uint32_t copy_size;
    
    uint32_t n;
    
    copy_size = 0;

//    eslink_data_offset = 0;
    if (len_send > 0) 
    {
        copy_size = MIN(len_send, usbd_cls_buf_sz);
        for (n = 0; n < copy_size; n++) 
        {
            USBD_CLS_BulkBuf[n] = USBD_CLS_ReadBuf[n+send_offset];
        }
        USBD_WriteEP(usbd_cls_ep_bulkin | 0x80, USBD_CLS_BulkBuf, copy_size);
        send_offset += copy_size;
        len_send -= copy_size; 
    } 
    if(len_send == 0)
    {
        USBD_CLS_Reset_Event();
    }    
}

void USBD_CLS_EP_BULKIN_Event(uint32_t event)
{
    USBD_CLS_DataInTransfer();
}

void USBD_CLS_BulkOut()
{
    len_send = USBD_CLS_DataOutTransfer(USBD_CLS_BulkBuf, len_rcv); 
    if(len_send >0 )
    {
        send_offset = 0;
        USBD_CLS_DataInTransfer();  
    }
          
    
}

void USBD_CLS_EP_BULKOUT_Event(uint32_t event)
{   
    len_rcv = USBD_ReadEP(usbd_cls_ep_bulkout, USBD_CLS_BulkBuf, usbd_cls_buf_sz);  
    USBD_CLS_BulkOut() ;
}

void USBD_CLS_EP_BULK_Event(uint32_t event)
{
    if (event & USBD_EVT_OUT) {
        USBD_CLS_EP_BULKOUT_Event(event);
    }

    if (event & USBD_EVT_IN) {
        
        USBD_CLS_EP_BULKIN_Event(event);
    }
}

