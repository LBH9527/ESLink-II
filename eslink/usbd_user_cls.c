#include "RTL.h"
#include "rl_usb.h"
#include "main.h"
#include "eslink.h"

#ifdef HR_8BIT
  #define ES_COMM_FRAME_MAX_LEN         DEBUG_FRAME_PACKET_LEN               //数据包长度
#elif ES_32BIT
  #define ES_COMM_FRAME_MAX_LEN         PROG_FRAME_MAX_SIZE               //数据包长度
#else
  #define ES_COMM_FRAME_MAX_LEN         DEBUG_FRAME_PACKET_LEN
#endif
//读数据状态

typedef enum
{
  ESLINK_STATE_CLOSE,
  ESLINK_STATE_WRITE,     //
  ESLINK_STATE_READ,      //读主机发送的数据
  ESLINK_STATE_END,
  ESLINK_STATE_ERROR
} eslink_state_t;
static eslink_state_t cls_state;
static uint32_t cls_data_ops;
static uint32_t cls_wr_cmd_len;                   //主机写命令码数据长度
static uint32_t cls_rd_cmd_len;                   //主机读命令码数据长度
static volatile uint8_t  usb_receive_full;

uint8_t usb_es_write[ES_COMM_FRAME_MAX_LEN]; //写数据buff
uint8_t usb_es_read[ES_COMM_FRAME_MAX_LEN];  //读数据buff

//函数声明
uint32_t eslink_process_command(uint8_t *write_buf, uint8_t *read_buf);

void USBD_CLS_init(void)
{
  cls_wr_cmd_len = 0;
  cls_data_ops = 0;
  USBD_CLS_WriteBuf =  usb_es_write;
  USBD_CLS_ReadBuf = usb_es_read;
  cls_state =   ESLINK_STATE_CLOSE;
  usb_receive_full = 0;

}
void USBD_CLS_Reset_Event(void)
{
  cls_wr_cmd_len = 0;
  cls_rd_cmd_len = 0;
  cls_data_ops = 0;
  cls_state =   ESLINK_STATE_CLOSE;

}


//根据接收到的数据，确认长度
static uint32_t get_cmd_data_len(uint8_t *data)
{
  uint32_t header;
  uint32_t cmd_len;

  header = (data[0] << 24) |
           (data[1] << 16) |
           (data[2] <<  8) |
           (data[3] <<  0);

  if (header == DEBUG_FRAME_HEAD)
  {
    cmd_len = (data[4] << 8) | data[5];
  }
  else if (header == PROG_FRAME_HEAD)
  {
    if ((data[5] == ID_DL_USERHEX)                  |
        (data[5] == ID_DL_OFL_HEX)                 |
        (data[5] == ID_DL_TIMING_ING)               |
        (data[5] == ID_READ_FLASH)                  |
        (data[5] == ID_DL_OFFLINE_HEX))
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

//保存接收到的数据。
//data:接收到的数据
//len ：接收到数据的长度
//返回值： 1 接收到协议规定的数据长度，接收完成
//         0 未接收完成
uint8_t cls_packet_write(uint8_t *data, uint8_t len)
{
  uint32_t n;

  for (n = 0; n < len; n++)
  {
    usb_es_write[cls_data_ops + n] = data[n];
  }

  cls_data_ops += len;
  cls_wr_cmd_len -= len;

  if (cls_data_ops > ES_COMM_FRAME_MAX_LEN)
    USBD_CLS_Reset_Event();

  if ((cls_wr_cmd_len == 0) && (cls_data_ops != 0))
  {
    cls_data_ops = 0;
    return  1;
  }

  return 0;
}
// USB Callback: when data is received from the host
uint32_t USBD_CLS_DataOutTransfer(uint8_t *data, uint8_t len)
{
  switch (cls_state)
  {
    case ESLINK_STATE_CLOSE:

      if (len < FRAME_HEADER_LEN)
        break;

      cls_wr_cmd_len = get_cmd_data_len(data);

      if ((cls_wr_cmd_len > ES_COMM_FRAME_MAX_LEN) | (cls_wr_cmd_len == 0))    //包长渡错误
        USBD_CLS_Reset_Event();

      if (cls_packet_write(data, len) != 1)
        cls_state = ESLINK_STATE_WRITE;
      else
        cls_state = ESLINK_STATE_READ;

      break;

    case ESLINK_STATE_WRITE:
      if (cls_packet_write(data, len) != 1)
        cls_state = ESLINK_STATE_WRITE;
      else
        cls_state = ESLINK_STATE_READ;

      break;

    default:
      cls_state = ESLINK_STATE_CLOSE;
      USBD_CLS_Reset_Event();
      break;
  }

  if (cls_state ==  ESLINK_STATE_READ)
  {
//        usb_receive_full = 1;
    cls_rd_cmd_len = eslink_process_command(usb_es_write, usb_es_read);
    return cls_rd_cmd_len;

  }

  return 0;
}


uint32_t eslink_process_command(uint8_t *write_buf, uint8_t *read_buf)
{
  uint32_t header;
  uint32_t read_len = 0;

  header = (write_buf[0] << 24) |
           (write_buf[1] << 16) |
           (write_buf[2] <<  8) |
           (write_buf[3] <<  0);

  if (header == DEBUG_FRAME_HEAD)
  {
#ifdef HR_8BIT
    //调试命令处理
    read_len = debug_process_command(write_buf, read_buf);
#endif
  }
  else if (header == PROG_FRAME_HEAD)
  {
    //连接命令处理
    read_len = prog_process_command(write_buf, read_buf);
  }
  else
  {


  }

  return read_len;
}



