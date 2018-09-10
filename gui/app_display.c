
#include "oled.h"
#include "gpio.h"
#include "es_common.h"
#include "fonts.h"


void eslink_display(void)
{
    FONT_T  tFont16;  
    
    OLED_InitHard();
    
    /* 设置字体参数 */
	{
		tFont16.FontCode = FC_ST_16;/* 字体代码 16点阵 */
		tFont16.FrontColor = 1;		/* 字体颜色 0 或 1 */
		tFont16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
		tFont16.Space = 0;			/* 文字间距，单位 = 像素 */
	}  
    OLED_DispStr(0,0,"es_link", &tFont16);
    while(1);
    
}