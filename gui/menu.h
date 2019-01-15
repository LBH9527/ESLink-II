#ifndef __MENU_H
#define __MENU_H	

#include <stdint.h> 
#include "message.h"

#define MENU_ONLINE_MODE            0x00    //联机模式
#define MENU_OFFLINE_MODE           0x01    //脱机模式

void menu_init(uint8_t mode );
void menu_display(void );
//void menu_display_logo(void);


#endif
