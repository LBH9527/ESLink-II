#ifndef __MESSAGE_H
#define __MESSAGE_H	

#include <stdint.h>
/* Null message */
#define  MSG_NULL                   0x00

/* Quit  message */
#define  MSG_QUIT                   0x01

/* key  message */
#define  MSG_KEY_DOWN               0x02
#define  MSG_KEY_ENTER              0x03

/* Offline proj update message*/
#define  MSG_UPDATE_SUCCESS         0x04
#define  MSG_UPDATE_FAILE           0x05

/* Offline program message*/
#define  MSG_PROG_MODE_CHECK        0x06
#define  MSG_PROG_ING               0x07
#define  MSG_PROG_OK               0x08
#define  MSG_PROG_FAILE               0x09
#define MSG_PROG_COUNT_FULL         0x0A

void msg_init(void);
uint8_t msg_read_data(uint8_t *value);
uint8_t msg_write_data(uint8_t *value);


#endif
