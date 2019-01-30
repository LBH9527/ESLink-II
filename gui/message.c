/*
****************************************************************************
*
*	模块名称 : 消息处理模块
*	文件名称 : bsp_msg.c
*	版    本 : V1.0
*	说    明 : 消息处理机制。
*
**************************************************************************
*/
#include "message.h"
#include "circ_buf.h"


#define BUFFER_SIZE   10
circ_buf_t msg_buffer;
uint8_t msg_buffer_data[BUFFER_SIZE];


void msg_clear_buffers(void)
{
    circ_buf_init(&msg_buffer, msg_buffer_data, sizeof(msg_buffer_data));
}

uint8_t gui_msg_write_data(uint8_t *value)
{
	uint8_t cnt;
    
    cnt = circ_buf_write(&msg_buffer, value, 1);
    
    return cnt;
}

/*******************************************************************************
*	函 数 名: gui_msg_read_data
*	功能说明: 读按键值
*	形    参: 按键值指针
*	返 回 值: 0：没有按键值 1 有按键值
*******************************************************************************/
uint8_t gui_msg_read_data(uint8_t *value)
{
    return circ_buf_read(&msg_buffer, value, 1);
}

//注册环形缓冲区，保存gui msg
void gui_msg_init(void)
{
    circ_buf_init(&msg_buffer, msg_buffer_data, sizeof(msg_buffer_data));

}
