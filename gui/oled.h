
#ifndef __OLED_H
#define __OLED_H			  	 

#include "stdint.h"

#define SIZE 16
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	   
 		     
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据
/* 字体代码 */
typedef enum
{
	FC_ST_12 = 0,		/* 宋体12x12点阵 （宽x高） */
	FC_ST_16,			/* 宋体16x16点阵 （宽x高） */   
    FC_ST_24,			/* 宋体24x24点阵 （宽x高） */
}FONT_CODE_E;

/* 字体属性结构, 用于LCD_DispStr() */
typedef struct
{
	FONT_CODE_E FontCode;	/* 字体代码 FONT_CODE_E  */
	uint16_t FrontColor;/* 字体颜色 */
	uint16_t BackColor;	/* 文字背景颜色，透明 */
	uint16_t Space;		/* 文字间距，单位 = 像素 */
}FONT_T;


/* 可供外部模块调用的函数 */
void oled_init(void);
void OLED_DispOn(void);
void OLED_DispOff(void);

void oled_clr_scr(uint8_t _ucMode);
void oled_display_str(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont);
#endif  
	 



