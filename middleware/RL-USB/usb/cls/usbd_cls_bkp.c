/*------------------------------------------------------------------------------
 *      RL-ARM - USB
 *------------------------------------------------------------------------------
 *      Name:    usbd_cls.c
 *      Purpose: USB Device Custom Class specific module example with example 
 *               for handling custom class interface requests
 *      Rev.:    V4.70
 *------------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2013 KEIL - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/
#include "string.h"

#include "RTL.h"
#include "rl_usb.h"
#include "usb_for_lib.h"


//#define __NO_USB_LIB_C
//#include "usb_config.c"

int32_t icd_data_send_access;              /*!< Flag active while send data (in the send intermediate buffer) is being accessed */
int32_t icd_data_send_active;              /*!< Flag active while data is being sent */
int32_t icd_data_send_zlp;                 /*!< Flag active when ZLP needs to be sent */
int32_t icd_data_to_send_wr;               /*!< Number of bytes written to the send intermediate buffer */
int32_t icd_data_to_send_rd;               /*!< Number of bytes read from the send intermediate buffer */
uint8_t *icd_ptr_data_to_send;             /*!< Pointer to the send intermediate buffer to the data to be sent */
uint8_t *icd_ptr_data_sent;                /*!< Pointer to the send intermediate buffer to the data already sent */

int32_t icd_data_read_access;              /*!< Flag active while read data (in the receive intermediate buffer) is being accessed */
int32_t icd_data_receive_int_access;       /*!< Flag active while read data (in the receive intermediate buffer) is being accessed from the IRQ function*/
int32_t icd_data_received_pending_pckts;   /*!< Number of packets received but not handled (pending) */
int32_t icd_data_no_space_for_receive;     /*!< Flag active while there is no more space for reception */
uint8_t *icd_ptr_data_received;            /*!< Pointer to the receive intermediate buffer to the received unread data */
uint8_t *icd_ptr_data_read;                /*!< Pointer to the receive intermediate buffer to the received read data */

__weak int32_t USBD_CLS_PortInitialize(void)
{
    return (0);
}

//__weak BOOL USBD_EndPoint0_Setup_CLS_ReqToIF  (void) {
//  /* Should handle any custom class interface requests */
//  /* Sample code which demonstrates how to use it */
//  if (USBD_SetupPacket.wIndexL == 0x01) {         /* Check interface number of custom class */
//    if (1) {                                  /* Interface request is in SETUP stage (after setup packet has been received) */
//    /* USBD_SetupPacket contains information of received request: bRequest, wValue, wIndex, wLength 
//       which can be analyzed to determine what exactly needs to be done and to prepare response */
//      switch (USBD_SetupPacket.bRequest) {
//        case 0x01:                                /* Request: 0x01 - custom defined request */
//          if (USBD_SetupPacket.bmRequestType.Dir == REQUEST_HOST_TO_DEVICE) {
//            /* Request which expects data to be received from host */
//            /* Prepare buffer for data reception */
//            USBD_EP0Data.pData = data_recv;
//          } else {
//            /* Request which expects data to be sent to host */ 
//            /* Prepare data to send */
//            data_send[0] = 0xAB;
//            data_send[1] = 0xCD;
//            /* ... */
//            USBD_EP0Data.pData = data_send;
//            if (USBD_EP0Data.Count > NUMBER_OF_BYTES_TO_SEND) {
//              /* If more data is requested then we have to send, we have to correct Count. 
//                 Also if last packet is sized as maximum packet we have to generate
//                 Zero length packet to terminate the response. */
//              USBD_EP0Data.Count = NUMBER_OF_BYTES_TO_SEND;
////              if (!(USBD_EP0Data.Count & (USBD_MAX_PACKET0 - 1))) USBD_ZLP = 1;
//            }
//            USBD_DataInStage();
//          }
//          return (__TRUE);
//      }
//    } else {                                      /* Interface request is in OUT stage (after data has been received) */
//      switch (USBD_SetupPacket.bRequest) {
//        case 0x01:                                /* Request: 0x01 - custom defined request */
//          /* Received data available for processing
//               Data buffer: data_recv
//               Data length: USBD_SetupPacket.wLength
//           */
//          USBD_StatusInStage();                   /* send Acknowledge */
//          return (__TRUE);
//      }
//    }
//  }
//  return (__FALSE);                               /* If this was not a request for our custom class interface or 
//                                                     if it was a request that we do not handle */
//}
/* Functions that can be used by user to use standard Virtual COM port
   functionality                                                              */
int32_t USBD_CLS_DataSend(const uint8_t *buf, int32_t len);
int32_t USBD_CLS_PutChar(const uint8_t  ch);
int32_t USBD_CLS_DataRead(uint8_t *buf, int32_t len);
int32_t USBD_CLS_GetChar(void);
__weak int32_t USBD_CLS_DataReceived(int32_t len)
{
    return (0);
}
int32_t USBD_CLS_DataAvailable(void);

/* Local function prototypes                                                  */
static void USBD_CLS_EP_BULKOUT_HandleData(void);
static void USBD_CLS_EP_BULKIN_HandleData(void);


void usbd_cls_init(void)
{
    icd_data_send_access            = 0;
    icd_data_send_active            = 0;
    icd_data_send_zlp               = 0;
    icd_data_to_send_wr             = 0;
    icd_data_to_send_rd             = 0;
    icd_ptr_data_to_send            = USBD_CLS_SendBuf;
    icd_ptr_data_sent               = USBD_CLS_SendBuf;
    icd_data_read_access            = 0;
    icd_data_receive_int_access     = 0;
    icd_data_received_pending_pckts = 0;
    icd_data_no_space_for_receive   = 0;
    icd_ptr_data_received           = USBD_CLS_ReceiveBuf;
    icd_ptr_data_read               = USBD_CLS_ReceiveBuf;
    USBD_CLS_PortInitialize();
}
int32_t USBD_CLS_Reset(void)
{
    icd_data_send_access            = 0;
    icd_data_send_active            = 0;
    icd_data_send_zlp               = 0;
    icd_data_to_send_wr             = 0;
    icd_data_to_send_rd             = 0;
    icd_ptr_data_to_send            = USBD_CLS_SendBuf;
    icd_ptr_data_sent               = USBD_CLS_SendBuf;
    icd_data_read_access            = 0;
    icd_data_receive_int_access     = 0;
    icd_data_received_pending_pckts = 0;
    icd_data_no_space_for_receive   = 0;
    icd_ptr_data_received           = USBD_CLS_ReceiveBuf;
    icd_ptr_data_read               = USBD_CLS_ReceiveBuf;

    return (0);
}

/** \brief Number of free bytes in the Send buffer
*/
int32_t USBD_CLS_DataFree(void)
{
    return ((int32_t)usbd_cls_sendbuf_sz) - (icd_data_to_send_wr - icd_data_to_send_rd);
}

/** \brief  Sends data over the USB CDC ACM Virtual COM Port

    The function puts requested data to the send intermediate buffer and
    prepares it for sending over the Virtual COM Port.

    \param [in]         buf      Buffer containing data to be sent.
    \param [in]         len      Maximum number of bytes to be sent.
    \return                      Number of bytes accepted to be sent.
 */

int32_t USBD_CLS_DataSend(const uint8_t *buf, int32_t len)
{
    int32_t  len_data, len_available, len_before_wrap;
    uint8_t *buf_loc;
    buf_loc       = (uint8_t *)buf;       /* Pointer to buf                     */
    len_data      = icd_data_to_send_wr - icd_data_to_send_rd;  /* Num of data in buffer*/
    len_available = ((int32_t)usbd_cls_sendbuf_sz) - len_data;  /* Num of
                                           bytes of space available           */

    if (len_available <= 0) {             /* If no space for data to send       */
        return (0);
    }

    if (len > len_available)              /* If more data requested for sending
                                           then available space               */
    {
        len = len_available;    /* Correct to maximum available       */
    }

    len_before_wrap = 0;                  /* Circular buffer size before wrap   */

    if ((icd_ptr_data_to_send >= icd_ptr_data_sent) && /* If wrap is possible to happen */
            ((icd_ptr_data_to_send + len) >= (USBD_CLS_SendBuf + usbd_cls_sendbuf_sz))) {
        /* If data wraps around end of buffer */
        len_before_wrap   = USBD_CLS_SendBuf + usbd_cls_sendbuf_sz - icd_ptr_data_to_send;
        memcpy(icd_ptr_data_to_send, buf_loc, len_before_wrap); /* Copy data till end */
        buf_loc          += len_before_wrap;            /* Increment buf pointer  */
        len              -= len_before_wrap;            /* Decrement bytes to send*/
        icd_ptr_data_to_send  = USBD_CLS_SendBuf;       /* Wrap send buffer
                                                       pointer to beginning of
                                                       the send buffer        */
    }

    if (len) {                            /* If there are bytes to send         */
        memcpy(icd_ptr_data_to_send, buf_loc, len);   /* Copy data to send buffer     */
        icd_ptr_data_to_send += len;            /* Correct position of write pointer  */
    }

    len += len_before_wrap;               /* Total number of bytes prepared for
                                           send                               */
    icd_data_to_send_wr += len;               /* Bytes prepared to send counter     */
    return (len);                         /* Number of bytes accepted for send  */
}


/** \brief  Sends a single character over the USB CDC ACM Virtual COM Port

    The function puts requested data character to the send intermediate buffer
    and prepares it for sending over the Virtual COM Port.

    \param [in]         ch       Character to be sent.
    \return             -1       Function failed.
    \return                      Character accepted to be sent.
 */

int32_t USBD_CLS_PutChar(const uint8_t ch)
{
    if ((USBD_CLS_DataSend(&ch, 1)) == 1) {
        return ((uint32_t) ch);
    }

    return (-1);
}


/** \brief  Reads data received over the USB CDC ACM Virtual COM Port

    The function reads data from the receive intermediate buffer that was
    received over the Virtual COM Port.

    \param [in]         buf      Buffer to where data will be read.
    \param [in]         len      Maximum number of bytes to be read.
    \return                      Number of bytes actually read.
 */

int32_t USBD_CLS_DataRead(uint8_t *buf, int32_t len)
{
    int32_t len_data;

    if (icd_ptr_data_received > icd_ptr_data_read) { /*If there is already received data   */
        len_data = icd_ptr_data_received - icd_ptr_data_read; /* Available bytes of data  */

        if (len > len_data) {               /* If more requested then available   */
            len = len_data;    /* correct to return maximum available*/
        }

        memcpy(buf, icd_ptr_data_read, len);    /* Copy received data to provided buf */
        icd_ptr_data_read      += len;          /* Correct position of read pointer   */
    } else {
        len = 0;                            /* No data received                   */
    }

    return (len);                         /* Number of bytes actually read      */
}


/** \brief  Reads one character of data received over the USB CDC ACM Virtual COM Port

    The function reads data character from the receive intermediate buffer that
    was received over the Virtual COM Port.

    \return             -1       No character available.
    \return                      Received character.
 */

int32_t USBD_CLS_GetChar(void)
{
    uint8_t ch;

    if ((USBD_CLS_DataRead(&ch, 1)) == 1) {
        return ((int32_t) ch);
    }

    return (-1);
}


/** \brief  Retrieves number of bytes received over the USB CDC ACM Virtual COM Port

    The function retrieves number of bytes available in the intermediate buffer
    that were received over the Virtual COM Port.

    \return                      Number of bytes available for read.
 */

int32_t USBD_CLS_DataAvailable(void)
{
    return (icd_ptr_data_received - icd_ptr_data_read);
}


/*----------------- USB CDC ACM communication event handlers -----------------*/

/** \brief  Handle Reset Events

    The function handles Reset events.
 */

void USBD_CLS_Reset_Event(void)
{
    USBD_CLS_Reset();
}


/** \brief  Handle SOF Events

    The function handles Start Of Frame events. It checks if there is pending
    data on the Bulk Out endpoint and handles it
    (USBD_CLS_EP_BULKOUT_HandleData) if there is enough space in the
    intermediate receive buffer and it calls received function callback
    (USBD_CLS_DataReceived) it also activates data send over the Bulk In
    endpoint if there is data to be sent (USBD_CLS_EP_BULKIN_HandleData).
 */

void USBD_CLS_SOF_Event(void)
{
    if (!USBD_Configuration) {
        // Don't process events until CDC is
        // configured and the endpoints enabled
        return;
    }
    if ((!icd_data_read_access)         &&    /* If not read active                 */
            (icd_ptr_data_received == icd_ptr_data_read) &&     /* If received and read
                                                     pointers point to same
                                                     the location             */
            (icd_ptr_data_received != USBD_CLS_ReceiveBuf)) {
        /* and if receive
                                                       pointer does not already
                                                       point to the start of
                                                       the receive buffer       */
        icd_data_read_access = 1;               /* Block access to read data          */
        icd_ptr_data_received = USBD_CLS_ReceiveBuf;  /* Correct received pointer
                                                     to point to the start of
                                                     the receive buffer       */
        icd_ptr_data_read     = USBD_CLS_ReceiveBuf;  /* Correct read pointer to
                                                     point to the start of the
                                                     receive buffer           */
        icd_data_no_space_for_receive  = 0;               /* There is space for
                                                     reception available      */
        icd_data_read_access = 0;               /* Allow access to read data          */
    }

    if (icd_data_received_pending_pckts &&    /* If packets are pending             */
            (!icd_data_read_access)          &&    /* and if not read active             */
            (!icd_data_no_space_for_receive)) {    /* and if there is space to receive   */
        icd_data_read_access = 1;               /* Disable access to read data        */
        USBD_CLS_EP_BULKOUT_HandleData(); /* Handle received data             */
        icd_data_read_access = 0;               /* Enable access to read data         */

        if (icd_ptr_data_received != icd_ptr_data_read) {
            USBD_CLS_DataReceived(icd_ptr_data_received - icd_ptr_data_read);
        }  /* Call

                                           received callback                  */
    }

    if ((!icd_data_send_access)         &&    /* If send data is not being accessed */
            (!icd_data_send_active)         &&    /* and send is not active             */
            (icd_data_to_send_wr - icd_data_to_send_rd) /* and if there is data to be sent    */
//&& ((control_line_state & 3) == 3)    /* and if DTR and RTS is 1            */
       ) {
        icd_data_send_access = 1;               /* Block access to send data          */
        icd_data_send_active = 1;               /* Start data sending                 */
        USBD_CLS_EP_BULKIN_HandleData();/* Handle data to send                */
        icd_data_send_access = 0;               /* Allow access to send data          */
    }
}


/** \brief  Handle Interrupt In Endpoint Events

    The function handles Interrupt In endpoint events.

    \param [in]         event    Type of event (USBD_EVT_IN - input event).
 */

void USBD_CLS_EP_INTIN_Event(uint32_t event)
{
    /* Notification will be loadad aynchronously and sent automatically upon
       Interrupt IN token reception                                             */
}


/** \brief  Handle Bulk Out Endpoint Received Data

    The function handles data received on the Bulk Out endpoint. It reads the
    received data to the receive intermediate buffer if there is enough space
    available.
 */

static void USBD_CLS_EP_BULKOUT_HandleData()
{
    uint32_t len_free_to_recv;
    int32_t len_received;

    if ((usbd_cls_receivebuf_sz - (icd_ptr_data_received - USBD_CLS_ReceiveBuf)) >= usbd_cls_sendbuf_sz) {
        /* If there is space for 1 max packet */
        /* Read received packet to receive buf*/
        len_free_to_recv = usbd_cls_receivebuf_sz - (icd_ptr_data_received - USBD_CLS_ReceiveBuf);
        len_received       = USBD_ReadEP(usbd_cls_ep_bulkout, icd_ptr_data_received, len_free_to_recv);
        icd_ptr_data_received += len_received;  /* Correct pointer to received data   */

        if (icd_data_received_pending_pckts &&  /* If packet was pending              */
                !icd_data_receive_int_access) {      /* and not interrupt access           */
            icd_data_received_pending_pckts--;    /* Decrement pending packets number   */
        }
    } else {
        icd_data_no_space_for_receive = 1;      /* There is no space in receive buffer
                                           for the newly received data        */

        if (icd_data_receive_int_access) {
            /* If this access is from interrupt
                                                   function                           */
            icd_data_received_pending_pckts++;    /* then this is new unhandled packet  */
        }
    }
}


/** \brief  Handle Bulk In Endpoint Data to Send

    The function handles data to be sent on the Bulk In endpoint. It transmits
    pending data to be sent that is already in the send intermediate buffer,
    and it also sends Zero Length Packet if last packet sent was not a short
    packet.
 */

static void USBD_CLS_EP_BULKIN_HandleData(void)
{
    int32_t len_to_send, len_sent;

    if (!icd_data_send_active) {              /* If sending is not active           */
        return;
    }

    len_to_send = icd_data_to_send_wr - icd_data_to_send_rd;  /* Num of data to send    */

    /* Check if sending is finished                                             */
    if (!len_to_send    &&                /* If all data was sent               */
            !icd_data_send_zlp)  {                /* and ZLP was sent if necessary also */
        icd_data_send_active = 0;               /* Sending not active any more        */
        return;
    }

    /* Check if data needs to be sent                                           */
    if (len_to_send) {
        /* If there is data available do be
                                                 sent                               */
        if ((icd_ptr_data_sent >= icd_ptr_data_to_send) && /* If data before end of buf avail*/
                ((icd_ptr_data_sent + len_to_send) >= (USBD_CLS_SendBuf + usbd_cls_sendbuf_sz))) {
            /* and if available data wraps around
               the end of the send buffer         */
            /* Correct bytes to send to data
               available untill end of send buf   */
            len_to_send = USBD_CLS_SendBuf + usbd_cls_sendbuf_sz - icd_ptr_data_sent;
        }

        if (len_to_send > usbd_cls_sendbuf_sz) {
            /* If
                                                   there is more data to be sent then
                                                   can be sent in a single packet     */
            /* Correct to send maximum pckt size  */
            len_to_send = usbd_cls_sendbuf_sz;
        }
    } else if (icd_data_send_zlp) {           /* or if ZLP should be sent           */
        len_to_send = 0;
    }

    icd_data_send_zlp = 0;
    /* Send data                          */
    len_sent = USBD_WriteEP(usbd_cls_ep_bulkin | 0x80, icd_ptr_data_sent, len_to_send);
    icd_ptr_data_sent    += len_sent;         /* Correct position of sent pointer   */
    icd_data_to_send_rd  += len_sent;         /* Correct num of bytes left to send  */

    if (icd_ptr_data_sent == USBD_CLS_SendBuf + usbd_cls_sendbuf_sz)
        /* If pointer to sent data wraps      */
    {
        icd_ptr_data_sent = USBD_CLS_SendBuf;
    } /* Correct it to beginning of send

                                           buffer                             */

    if ((icd_data_to_send_wr == icd_data_to_send_rd) &&   /* If there are no more
                                           bytes available to be sent         */
            (len_sent == usbd_cls_sendbuf_sz)) {
        /* If last packet size was same as
           maximum packet size                */
        icd_data_send_zlp = 1;                  /* ZLP packet should be sent          */
    } else {
        icd_data_send_zlp = 0;                  /* No ZLP packet should be sent       */
    }
}


/** \brief  Handle Bulk Out Endpoint Events

    The function handles Bulk Out endpoint events. It calls
    USBD_CLS_EP_BULKOUT_HandleData function to handle received data
    unless data was being accessed in which case function just acknowledges
    that there is data to be handled later.

    \param [in]         event    Type of event (USBD_EVT_OUT - output event).
 */

void USBD_CLS_EP_BULKOUT_Event(uint32_t event)
{
    if (icd_data_read_access) {
        /* If data is being accessed from
                                                 read function                      */
        icd_data_received_pending_pckts++;      /* 1 more packet received and not
                                           handled                            */
        return;
    }

    icd_data_read_access = 1;                 /* Block access to read data          */
    icd_data_receive_int_access = 1;          /* Read access from interrupt function*/
    USBD_CLS_EP_BULKOUT_HandleData(); /* Handle received data               */
    icd_data_receive_int_access = 0;          /* Read access from interrupt func end*/
    icd_data_read_access = 0;                 /* Allow access to read data          */

    if (icd_ptr_data_received != icd_ptr_data_read) {
        USBD_CLS_DataReceived(icd_ptr_data_received - icd_ptr_data_read);
    }    /* Call

                                           received callback                  */
}




/** \brief  Handle Bulk In Endpoint Events

    The function handles Bulk In endpoint events. It calls
    USBD_CLS_EP_BULKIN_HandleData function to handle send data
    unless data was being accessed in which case function just returns.

    \param [in]         event    Type of event (USBD_EVT_IN - input event).
 */

void USBD_CLS_EP_BULKIN_Event(uint32_t event)
{
    if (icd_data_send_access                  /* If send data is being accessed     */
// ||((control_line_state & 3) != 3)    /* or if DTR or RTS is 0              */
       ) {
        return;
    }

    icd_data_send_access = 1;                 /* Block access to send data          */
    USBD_CLS_EP_BULKIN_HandleData();  /* Handle data to send                */
    icd_data_send_access = 0;                 /* Allow access to send data          */
}


/** \brief  Handle Bulk In/Out Endpoint Events

    The function handles Bulk In/Out endpoint events. It is used for endpoints
    that do In and Out functionality on the same endpoint number. It dispatches
    events to appropriate In or Out event handlers.

    \param [in]         event    Type of event (
                                   USBD_EVT_IN  - input event,
                                   USBD_EVT_OUT - output event).
 */

void USBD_CLS_EP_BULK_Event(uint32_t event)
{
    if (event & USBD_EVT_OUT) {
        USBD_CLS_EP_BULKOUT_Event(event);
    }

    if (event & USBD_EVT_IN) {
        USBD_CLS_EP_BULKIN_Event(event);
    }
}


