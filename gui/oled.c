/*
//OLED的显存
//存放格式如下.
[0]0 1 2 3 ... 127	
[1]0 1 2 3 ... 127	
[2]0 1 2 3 ... 127	
[3]0 1 2 3 ... 127	
[4]0 1 2 3 ... 127	
[5]0 1 2 3 ... 127	
[6]0 1 2 3 ... 127	
[7]0 1 2 3 ... 127 	

-------------------> x
|
|
|
|
y
*/
#include "oled.h"
#include "gpio.h"
#include "es_common.h"
#include "fonts.h"

static void oled_delay_ms(uint32_t delay) 
{      
    delay *= ((SystemCoreClock/1000U) + (4-1U)) / 4;
    while (--delay);
}

/* 12864 OLED的显存镜像，占用1K字节. 共8行，每行128像素 */
static uint8_t OLED_Gram[8][128];

/* 为了避免刷屏拉幕感太强，引入刷屏标志。
0 表示显示函数只改写缓冲区，不写屏。1 表示直接写屏（同时写缓冲区） */
static uint8_t s_ucUpdateEn = 1;

static void oled_write_cmd(uint8_t _ucCmd);
static void oled_write_data(uint8_t _ucData);
static void OLED_BufToPanel(void);

/*******************************************************************************
*	函 数 名: OLED_DispOn
*	功能说明: 打开显示
*	形    参:  无
*	返 回 值: 无
*******************************************************************************/
void OLED_DispOn(void)
{
	oled_write_cmd(0x8D);	/* 设置充电泵（和下个命令结合使用） */
	oled_write_cmd(0x14);	/* 0x14 使能充电泵， 0x10 是关闭 */
	oled_write_cmd(0xAF);	/* 打开OLED面板 */
}

/*******************************************************************************
*	函 数 名: oled_disp_off
*	功能说明: 关闭显示
*	形    参:  无
*	返 回 值: 无
*******************************************************************************/
void OLED_DispOff(void)
{
	oled_write_cmd(0x8D);	/* 设置充电泵（和下个命令结合使用）*/
	oled_write_cmd(0x10);	/* 0x14 使能充电泵，0x10 是关闭 */
	oled_write_cmd(0xAE);	/* 打开OLED面板 */
}

/*******************************************************************************
*	函 数 名: OLED_SetDir
*	功能说明: 设置显示方向
*	形    参: _ucDir = 0 表示正常方向，1表示翻转180度
*	返 回 值: 无
*******************************************************************************/
void OLED_SetDir(uint8_t _ucDir)
{
	if (_ucDir == 0)
	{
       	oled_write_cmd(0xA0);	/* A0 ：列地址0映射到SEG0; A1 ：列地址127映射到SEG0 */
		oled_write_cmd(0xC0);	/* C0 ：正常扫描,从COM0到COM63;  C8 : 反向扫描, 从 COM63至 COM0 */
	}
	else
	{
		oled_write_cmd(0xA1);	/* A0 ：列地址0映射到SEG0; A1 ：列地址127映射到SEG0 */
		oled_write_cmd(0xC8);	/* C0 ：正常扫描,从COM0到COM63;  C8 : 反向扫描, 从 COM63至 COM0 */
	}
}

/*******************************************************************************
*	函 数 名: OLED_SetContrast
*	功能说明: 设置对比度
*	形    参:  无
*	返 回 值: 无
*******************************************************************************/
void OLED_SetContrast(uint8_t ucValue)
{
	oled_write_cmd(0x81);	/* 设置对比度命令(双字节命令），第1个字节是命令，第2个字节是对比度参数0-255 */
	oled_write_cmd(ucValue);	/* 设置对比度参数,缺省CF */
}

/*******************************************************************************
*	函 数 名: OLED_StartDraw
*	功能说明: 开始绘图。以后绘图函数只改写缓冲区，不改写面板显存
*	形    参:  无
*	返 回 值: 无
*******************************************************************************/
void OLED_StartDraw(void)
{
	s_ucUpdateEn = 0;
}

/*******************************************************************************
*	函 数 名: OLED_EndDraw
*	功能说明: 结束绘图。缓冲区的数据刷新到面板显存。 OLED_StartDraw() 和 OLED_EndDraw() 必须成对使用
*	形    参:  无
*	返 回 值: 无
*******************************************************************************/
void OLED_EndDraw(void)
{
	s_ucUpdateEn = 1;
	OLED_BufToPanel();
}

/*******************************************************************************
*	函 数 名: OLED_ClrScr
*	功能说明: 清屏
*	形    参:  _ucMode : 0 表示全黑； 0xFF表示全亮
*	返 回 值: 无
*******************************************************************************/
void OLED_ClrScr(uint8_t _ucMode)
{
	uint8_t i,j;

	for (i = 0 ; i < 8; i++)
	{
		for (j = 0 ; j < 128; j++)
		{
			OLED_Gram[i][j] = _ucMode;
		}
	}

	if (s_ucUpdateEn == 1)
	{
		OLED_BufToPanel();
	}
}

/*******************************************************************************
*	函 数 名: OLED_BufToPanel
*	功能说明: 将缓冲区中的点阵数据写入面板
*	形    参:  无
*	返 回 值: 无
*******************************************************************************/
static void OLED_BufToPanel(void)
{
	uint8_t i,j;

	for (i = 0 ; i< 8; i++)
	{
		oled_write_cmd (0xB0 + i);	/* 设置页地址（0~7） */
		oled_write_cmd (0x00);		/* 设置列地址的低地址 */
		oled_write_cmd (0x10);		/* 设置列地址的高地址 */

		for (j = 0 ; j < 128; j++)
		{
			oled_write_data(OLED_Gram[i][j]);
		}
	}
}

/*******************************************************************************
*	函 数 名: OLED_DispStr
*	功能说明: 在屏幕指定坐标（左上角为0，0）显示一个字符串
*	形    参:
*		_usX : X坐标，对于12864屏，范围为【0 - 127】
*		_usY : Y坐标，对于12864屏，范围为【0 - 63】
*		_ptr  : 字符串指针
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数
*	返 回 值: 无
*******************************************************************************/
void OLED_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont)
{
	uint32_t i;
	uint8_t code1;
	uint8_t code2;
	uint32_t address = 0;
	uint8_t buf[24 * 24 / 8];	/* 最大支持24点阵汉字 */
	uint8_t m, width;
	uint8_t font_width,font_height, font_bytes;
	uint16_t x, y;
	const uint8_t *pAscDot;		
	const uint8_t *pHzDot;

    OLED_StartDraw();
//	/* 如果字体结构为空指针，则缺省按16点阵 */
//	if (_tFont->FontCode == FC_ST_12)
//	{
//		font_height = 12;
//		font_width = 12;
//		font_bytes = 24;
//		pAscDot = g_Ascii12;	
//		pHzDot = g_Hz12;
//		
//	}
//	else
	{
		/* 缺省是16点阵 */
		font_height = 16;
		font_width = 16;
		font_bytes = 32;
		pAscDot = g_Ascii16;	
		pHzDot = g_Hz16;
	
	}

	/* 开始循环处理字符 */
	while (*_ptr != 0)
	{
		code1 = *_ptr;	/* 读取字符串数据， 该数据可能是ascii代码，也可能汉字代码的高字节 */
		if (code1 < 0x80)
		{
			/* 将ascii字符点阵复制到buf */
			memcpy(buf, &pAscDot[code1 * (font_bytes / 2)], (font_bytes / 2));
			width = font_width / 2;
		}
		else
		{
			code2 = *++_ptr;
			if (code2 == 0)
			{
				break;
			}

			/* 计算16点阵汉字点阵地址
				ADDRESS = [(code1-0xa1) * 94 + (code2-0xa1)] * 32
				;
			*/			
				m = 0;
				while(1)
				{
					address = m * (font_bytes + 2);
					m++;
					if ((code1 == pHzDot[address + 0]) && (code2 == pHzDot[address + 1]))
					{
						address += 2;
						memcpy(buf, &pHzDot[address], font_bytes);
						break;
					}
					else if ((pHzDot[address + 0] == 0xFF) && (pHzDot[address + 1] == 0xFF))
					{
						/* 字库搜索完毕，未找到，则填充全FF */
						memset(buf, 0xFF, font_bytes);
						break;
					}
				}			
				width = font_width;
		}

		y = _usY;
		/* 开始刷LCD */
		for (m = 0; m < font_height; m++)	/* 字符高度 */
		{
			x = _usX;
			for (i = 0; i < width; i++)	/* 字符宽度 */
			{
				if ((buf[m * ((2 * width) / font_width) + i / 8] & (0x80 >> (i % 8 ))) != 0x00)
				{
					OLED_PutPixel(x, y, _tFont->FrontColor);	/* 设置像素颜色为文字色 */
                    
				}
				else
				{
					OLED_PutPixel(x, y, _tFont->BackColor);	/* 设置像素颜色为文字背景色 */
				}

				x++;
			}
			y++;
		}

		if (_tFont->Space > 0)
		{
			/* 如果文字底色按_tFont->usBackColor，并且字间距大于点阵的宽度，那么需要在文字之间填充(暂时未实现) */
		}
		_usX += width + _tFont->Space;	/* 列地址递增 */
		_ptr++;			/* 指向下一个字符 */
	}
    OLED_EndDraw();
}

/*******************************************************************************
*	函 数 名: OLED_PutPixel
*	功能说明: 画1个像素
*	形    参:
*			_usX,_usY : 像素坐标
*			_ucColor  ：像素颜色
*	返 回 值: 无
*******************************************************************************/
void OLED_PutPixel(uint16_t _usX, uint16_t _usY, uint8_t _ucColor)

{
	uint8_t ucValue;
	uint8_t ucPageAddr;
	uint8_t ucColAddr;

	const uint8_t aOrTab[8]  = {0x01, 0x02, 0x04, 0x08,0x10,0x20,0x40,0x80};
	const uint8_t aAndTab[8] = {0xFE, 0xFD, 0xFB, 0xF7,0xEF,0xDF,0xBF,0x7F};

	ucPageAddr = _usY / 8;
	ucColAddr = _usX;

	ucValue = OLED_Gram[ucPageAddr][ucColAddr];
	if (_ucColor == 0)
	{
		ucValue &= aAndTab[_usY % 8];
	}
	else
	{
		ucValue |= aOrTab[_usY % 8];
	}
	OLED_Gram[ucPageAddr][ucColAddr] = ucValue;

	if (s_ucUpdateEn == 1)
	{
		oled_write_cmd (0xB0 + ucPageAddr);					/* 设置页地址（0~7） */
		oled_write_cmd (0x00 + (ucColAddr & 0x0F));			/* 设置列地址的低地址 */
        oled_write_cmd (0x10 + ((ucColAddr >> 4) & 0x0F));	/* 设置列地址的高地址 */
		oled_write_data(ucValue);
	}
}

/**
* @brief 设置坐标点
* @param x:x坐标
* @param y:y坐标
*/

static void oled_set_pos(uint8_t x,uint8_t y) 
{ 
    oled_write_cmd(0xb0 + (7 - y / 8));
    oled_write_cmd(((x & 0xf0) >> 4) | 0x10);
    oled_write_cmd((x & 0x0f)); 
} 

static void oled_refresh_gram(void)
{
	uint8_t i,j;

	for (i = 0 ; i< 8; i++)
	{
		oled_write_cmd (0xB0 + i);	/* 设置页地址（0~7） */
		oled_write_cmd (0x00);		/* 设置列地址的低地址 */
		oled_write_cmd (0x10);		/* 设置列地址的高地址 */

		for (j = 0 ; j < 128; j++)
		{
			oled_write_data(OLED_Gram[i][j]);
		}
	}
}
/**
* @brief 画点
* @param x:x坐标
* @param y:y坐标
* @param show:0:不显示,1:显示
*/
void oled_draw_pixel(uint8_t x,uint8_t y,uint8_t show)
{
    uint8_t pos = 0;
    uint8_t bx = 0;
    uint8_t temp = 0;

    //设置坐标点
    oled_set_pos(x,y);
    //p岸段是否超出范围
    if (x > 127 || y > 64)
    {
        return;
    }

    pos = 7 - y / 8;
    bx = y % 8;
    temp = 1 << (7 - bx);
    if (show)
    {
        OLED_Gram[x][pos] |= temp;
    }
    else
    {
        OLED_Gram[x][pos] &= ~temp;
    }       

    //设置显示位置—列高地址   
    oled_write_data(OLED_Gram[x][pos]);     
    
}



/*******************************************************************************
*	函 数 名: OLED_DrawLine
*	功能说明: 采用 Bresenham 算法，在2点间画一条直线。
*	形    参:
*			_usX1, _usY1 ：起始点坐标
*			_usX2, _usY2 ：终止点Y坐标
*			_ucColor     ：颜色
*	返 回 值: 无
*******************************************************************************/
void OLED_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint8_t _ucColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* 采用 Bresenham 算法，在2点间画一条直线 */

	OLED_PutPixel(_usX1 , _usY1 , _ucColor);

	/* 如果两点重合，结束后面的动作。*/
	if ( _usX1 == _usX2 && _usY1 == _usY2 )
	{
		return;
	}

	iTag = 0 ;
	/* dx = abs ( _usX2 - _usX1 ); */
	if (_usX2 >= _usX1)
	{
		dx = _usX2 - _usX1;
	}
	else
	{
		dx = _usX1 - _usX2;
	}

	/* dy = abs ( _usY2 - _usY1 ); */
	if (_usY2 >= _usY1)
	{
		dy = _usY2 - _usY1;
	}
	else
	{
		dy = _usY1 - _usY2;
	}

	if ( dx < dy )   /*如果dy为计长方向，则交换纵横坐标。*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* 确定是增1还是减1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* 循环画点 */
	{
		if ( d < 0 )
		{
			d += inc1 ;
		}
		else
		{
			y += ty ;
			d += inc2 ;
		}
		if ( iTag )
		{
			OLED_PutPixel ( y , x , _ucColor) ;
		}
		else
		{
			OLED_PutPixel ( x , y , _ucColor) ;
		}
		x += tx ;
	}
}

/*******************************************************************************
*	函 数 名: OLED_DrawPoints
*	功能说明: 采用 Bresenham 算法，绘制一组点，并将这些点连接起来。可用于波形显示。
*	形    参:
*			x, y     ：坐标数组
*			_ucColor ：颜色
*	返 回 值: 无
*******************************************************************************/
void OLED_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint8_t _ucColor)
{
	uint16_t i;

	for (i = 0 ; i < _usSize - 1; i++)
	{
		OLED_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _ucColor);
	}
}

/*******************************************************************************
*	函 数 名: OLED_DrawRect
*	功能说明: 绘制矩形。
*	形    参:
*			_usX,_usY：矩形左上角的坐标
*			_usHeight ：矩形的高度
*			_usWidth  ：矩形的宽度
*	返 回 值: 无
*******************************************************************************/
void OLED_DrawRect(uint16_t _usX, uint16_t _usY, uint8_t _usHeight, uint16_t _usWidth, uint8_t _ucColor)
{
	/*
	 ---------------->---
	|(_usX，_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
	*/

	OLED_DrawLine(_usX, _usY, _usX + _usWidth - 1, _usY, _ucColor);	/* 顶 */
	OLED_DrawLine(_usX, _usY + _usHeight - 1, _usX + _usWidth - 1, _usY + _usHeight - 1, _ucColor);	/* 底 */

	OLED_DrawLine(_usX, _usY, _usX, _usY + _usHeight - 1, _ucColor);	/* 左 */
	OLED_DrawLine(_usX + _usWidth - 1, _usY, _usX + _usWidth - 1, _usY + _usHeight, _ucColor);	/* 右 */
}



/*******************************************************************************
*	函 数 名: OLED_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
*	形    参:
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：单色图片点阵指针，每个像素占用1个字节
*	返 回 值: 无
*******************************************************************************/
void OLED_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint8_t *_ptr)
{
	uint16_t x, y;

	for (x = 0; x < _usWidth; x++)
	{
		for (y = 0; y < _usHeight; y++)
		{
			OLED_PutPixel(_usX + x, _usY + y, *_ptr);
		}
	}
}

/*******************************************************************************
*	函 数 名: oled_write_cmd
*	功能说明: 向SSD1306发送一字节命令
*	形    参:  命令字
*	返 回 值: 无
*******************************************************************************/
static void oled_write_cmd(uint8_t cmd)
{
	uint8_t i;

	OLED_CS_Clr();
	OLED_SCK_Clr();
	OLED_SDA_Clr();	/* 0 表示后面传送的是命令 1表示后面传送的数据 */
	OLED_SCK_Set();

	for (i = 0; i < 8; i++)
	{
		if (cmd & 0x80)
		{
			OLED_SDA_Set();
		}
		else
		{
			OLED_SDA_Clr();
		}
		OLED_SCK_Clr();
		cmd <<= 1;
		OLED_SCK_Set();
	}

	OLED_CS_Set();
}

/*******************************************************************************
*	函 数 名: oled_write_data
*	功能说明: 向SSD1306发送一字节数据
*	形    参:  命令字
*	返 回 值: 无
*******************************************************************************/
static void oled_write_data(uint8_t data)
{
	uint8_t i;

	OLED_CS_Clr();

	OLED_SCK_Clr();
	OLED_SDA_Set();	/* 0 表示后面传送的是命令 1表示后面传送的数据 */
	OLED_SCK_Set();

	for (i = 0; i < 8; i++)
	{
		if (data & 0x80)
		{
			OLED_SDA_Set();
		}
		else
		{
			OLED_SDA_Clr();
		}
		OLED_SCK_Clr();
		data <<= 1;
		OLED_SCK_Set();
	}

	OLED_CS_Set();    
    
}

/*******************************************************************************
*	函 数 名: OLED_InitHard
*	功能说明: 初始化OLED屏
*	形    参:  无
*	返 回 值: 无
*******************************************************************************/
void OLED_InitHard(void)
{
	/* 上电延迟 */
    OLED_RST_Set();
    oled_delay_ms(100);
	OLED_RST_Clr();
    oled_delay_ms(100);
	OLED_RST_Set();  

	 /* 模块厂家提供初始化代码 */
	oled_write_cmd(0xAE);	/* 关闭OLED面板显示(休眠) */
	oled_write_cmd(0x00);	/* 设置列地址低4bit */
	oled_write_cmd(0x10);	/* 设置列地址高4bit */
	oled_write_cmd(0x40);	/* 设置起始行地址（低5bit 0-63）， 硬件相关*/
	oled_write_cmd(0x81);	/* 设置对比度命令(双字节命令），第1个字节是命令，第2个字节是对比度参数0-255 */
	oled_write_cmd(0xCF);	/* 设置对比度参数,缺省CF */    
	oled_write_cmd(0xA1);	/* A0 ：列地址0映射到SEG0; A1 ：列地址127映射到SEG0 */
	oled_write_cmd(0xC8);	/* C0 ：正常扫描,从COM0到COM63;  C8 : 反向扫描, 从 COM63至 COM0 */
	oled_write_cmd(0xA6);	/* A6 : 设置正常显示模式; A7 : 设置为反显模式 */
	oled_write_cmd(0xA8);	/* 设置COM路数 */
	oled_write_cmd(0x3F);	/* 1 ->（63+1）路 */
	oled_write_cmd(0xD3);	/* 设置显示偏移（双字节命令）*/
	oled_write_cmd(0x00);	/* 无偏移 */
	oled_write_cmd(0xD5);	/* 设置显示时钟分频系数/振荡频率 */
	oled_write_cmd(0x80);	/* 设置分频系数,高4bit是分频系数，低4bit是振荡频率 */
	oled_write_cmd(0xD9);	/* 设置预充电周期 */
	oled_write_cmd(0xF1);	/* [3:0],PHASE 1; [7:4],PHASE 2; */
	oled_write_cmd(0xDA);	/* 设置COM脚硬件接线方式 */
	oled_write_cmd(0x12);
	oled_write_cmd(0xDB);	/* 设置 vcomh 电压倍率 */
	oled_write_cmd(0x40);	/* [6:4] 000 = 0.65 x VCC; 0.77 x VCC (RESET); 0.83 x VCC  */
	oled_write_cmd(0x20);    //-Set Page Addressing Mode (0x00/0x01/0x02)
	oled_write_cmd(0x02);    //    
	oled_write_cmd(0x8D);	/* 设置充电泵（和下个命令结合使用） */
	oled_write_cmd(0x14);	/* 0x14 使能充电泵， 0x10 是关闭 */    
	oled_write_cmd(0xA4);    // Disable Entire Display On (0xa4/0xa5)
	oled_write_cmd(0xA6);    // Disable Inverse Display On (0xa6/a7) 
	oled_write_cmd(0xAF);	/* 打开OLED面板 */  
    OLED_ClrScr(0x00);
}
