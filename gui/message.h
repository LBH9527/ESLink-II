#ifndef __MESSAGE_H
#define __MESSAGE_H	

#include <stdint.h>
/* Null message */
#define MSG_NULL                    0x00

/* Quit  message */
#define MSG_QUIT                    0x01

/* key  message */
#define MSG_KEY_DOWN                0x02
#define MSG_KEY_ENTER               0x03

/* Offline proj update message*/
#define MSG_UPDATE_SUCCESS          0x04
#define MSG_UPDATE_FAILE            0x05

/* Offline program message*/
#define MSG_PROG_MODE_CHECK         0x06
#define MSG_PROG_ING                0x07
#define MSG_PROG_OK                 0x08
#define MSG_ERR_COUNT_FULL          0x09            //烧录计数溢出

#define MSG_ERR_CHIPID_CHECK        0x0B            //ID检测失败
#define MSG_ERR_ERASE               0x0c            //擦除失败
#define MSG_ERR_CHECK_EMPTY         0x0D            //查空
#define MSG_ERR_PROG                0x0E            //编程失败
#define MSG_ERR_VERIFY              0x0F            //校验
#define MSG_ERR_ENCRYPT             0x10            //加密
#define MSG_ERR_ENTRY_MODE          0x11
#define MSG_ERR_PROG_INTF           0x0A            //编程接口设置失败

#define MSG_ERR                    0xFF            //编程失败，未知错误
void gui_msg_init(void);
uint8_t gui_msg_read_data(uint8_t *value);
uint8_t gui_msg_write_data(uint8_t *value);


#endif
