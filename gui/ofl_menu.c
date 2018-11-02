#include "es_common.h"
#include "oled.h"

#include "offline_file.h" 
#include "menu.h"
#include "fonts.h"
#include "main.h"
#include "update.h"
#include "settings_rom.h"
#include "cortex_m.h"
#include "ofl_prj_decoder.h"
#include "systick.h"

static uint8_t disp_init = 0;
typedef void (* menu_func)(void );

#define PAGE_DISP_NUM   3    //每屏显示菜单数

struct ofl_menu_state
{
//    uint8_t ExitMark;             // 退出菜单(0-不退出，1-退出)
    uint8_t disp_num;                // 每页显示的菜单项个数    
    uint8_t item_index;		        //当前选定菜单对应的索引号
    uint8_t cur_line ;              //当前选择的行
    uint8_t first_line;             //屏幕第一行显示的索引号

//    uint8_t MaxPage;               // 最大页数(最大有多少种显示页)    
    
} ;
struct ofl_menu_state ofl_state;

struct menu_item
{
    uint8_t item_num;           //待显示的条目数量
    char str[MAX_PAJ_NUM+1][OFL_FILE_NAME_MAX_LEN+1];   //包括方案名和退出栏
	uint8_t state;				//所选择的子菜单
} ;
struct menu_item  ofl_item ;   //脱机菜单显示项 

struct menu_t
{ 	
//	struct menu_item *items;          //菜单项目
	menu_func func;                     //菜单处理函数
	struct menu_t *next_menu;            //下级菜单
//    struct menu_t *child_menu;    //子菜单
////    struct menu_t *parent_menu;   //上级菜单 ,如果是顶级则为null
}  ; 
struct menu_t *cur_menu;				//当前菜单窗口  


struct ofl_update_flag{
     char *name;
     uint8_t select;
} ;

struct ofl_update_flag ofl_flag;


void ofl_select_display(void);
void logo_display(void );
void update_display(void );
void idle_func(void)
{
  /* Nothing to execute: return */
  return;
}
struct menu_t ofl_menu;

struct menu_t update_menu = 
{
     update_display,
     &ofl_menu,
    
} ;
struct menu_t ofl_menu = 
{
	ofl_select_display,
	&update_menu,
};  


void update_display(void)
{
//    uint8_t msg = MSG_NULL;
    FONT_T Font16;
    error_t ret;
	 Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
    Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
    Font16.Space = 0;			/* 文字间距，单位 = 像素 */	
    
	if(disp_init == 1)
	{  
        oled_clr_scr(0x00);	/* 清屏，0x00表示黑底； 0xFF 表示白底 */  			
		oled_display_str(0,12,"   updating     ", &Font16);       
        oled_display_str(0,32,"   please wait  ", &Font16);	
		disp_init = 0;
	}
    
    if( ofl_flag.select)
    {
        ofl_flag.select = 0;
        LED_YELLOW_ON();	
        ret = ofl_prj_update(ofl_flag.name);

        if(ret != ERROR_SUCCESS)
        {
            oled_clr_scr(0x00);	              
            oled_display_str(0,12,"  update error  ", &Font16);
            ofl_flag.select = 0;
            ofl_flag.name = 0;
            bsp_delay_ms(2000);
            cur_menu  = cur_menu-> next_menu;
            disp_init = 1;
        }
        else
        {
              //更新成功，跳转到link程序,
            set_app_update(UPDATE_LINK_APP);
            if(set_link_mode(LINK_OFFLINE_MODE) != TRUE )
            {
            
            }
            oled_clr_scr(0x00);						
            oled_display_str(0,12," update success ", &Font16);

            SystemSoftReset();           
        }             
    }   
}


//ofl字符串显示
static void ofl_diplay_str(uint8_t i, char *str)
{
    FONT_T Font16;
    
    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
    Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
    Font16.Space = 0;			/* 文字间距，单位 = 像素 */	
    oled_display_str(0, i, str, &Font16);  
}
//ofl反白显示
static void ofl_diplay_str_inv(uint8_t i, char *str)
{
    FONT_T Font16;
    
    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.FrontColor = 0;		/* 字体颜色 0 或 1 */
    Font16.BackColor = 1;		/* 文字背景颜色 0 或 1 */
    Font16.Space = 0;			/* 文字间距，单位 = 像素 */	
    oled_display_str(0, i, str, &Font16);  
} 

void ofl_display (void)
{
	uint8_t i;
    char display_temp[16+1] = {'\0'};
	for(i=0; i<ofl_state.disp_num; i++)
    {
        sprintf(display_temp,"%-16s",ofl_item.str[i+ofl_state.first_line]);
        if(i== ofl_state.cur_line)
            ofl_diplay_str_inv((i+1)*16,display_temp); 
        else
            ofl_diplay_str((i+1)*16,display_temp); 
    } 	
}
/*******************************************************************************
*	函 数 名: ofl_select_display
*	功能说明: 脱机工程选择菜单
*	形    参: 
*	返 回 值: None
*******************************************************************************/
void ofl_select_display(void)
{
    uint8_t msg = MSG_NULL;
    char *exit_item = "EXIT";
    char display_temp[2] = {'\0'};
	//    char data[13] = {0xD1,0xA1,0xD4,0xF1,0xCD,0xD1,0xBB,0xFA,0xB7,0xBD,0xB0,0xB8,0};//“选择脱机方案”
	char data[9] = {0xD1,0xA1,0xD4,0xF1,0xB7,0xBD,0xB0,0xB8,0};//“选择方案” 
//    uint8_t  SelectLine_L = 1;                    //原来选中行
    FONT_T Font16;
    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
	Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
    
	if(disp_init == 1)
	{
		oled_clr_scr(0x00);	/* 清屏，0x00表示黑底； 0xFF 表示白底 */
		Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
		Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
		Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
		oled_display_str(0,0,data, &Font16);  
//		oled_display_str(0,0,"方案选择", &Font16);      	
		oled_display_str(64,0,"(00/  )", &Font16);
        sprintf(display_temp,"%02d",ofl_item.item_num-1);  
        oled_display_str(96,0,display_temp, &Font16); 
        sprintf(display_temp,"%02d",ofl_state.item_index) ; 
        oled_display_str(72,0,display_temp, &Font16); 
		disp_init = 0;	
        ofl_display();
	}          
    
    msg_read_data(&msg);
	if(msg != MSG_NULL)        //有按键按下 
    {
		switch (msg)
		{
			case MSG_KEY_DOWN:
				if(ofl_state.item_index >= ofl_item.item_num )    //如果当前是表单最后一个索引
				{
                    if( ofl_item.item_num != 0)
                        ofl_state.item_index = 1;
                    else
                        ofl_state.item_index = 0;

                    ofl_state.cur_line = 0;
                    ofl_state.first_line = 0; 
				}               
				else	//如果不是最后的索引
				{
					if(ofl_state.cur_line != PAGE_DISP_NUM-1)     //如果当前不是屏幕最底行
					{
						ofl_state.item_index++;       //选择索引自加 
//						SelectLine_L = ofl_state.cur_line;        //记录原来行
						ofl_state.cur_line++;       //选择行下移 						
					}                       
					else
					{
						ofl_state.first_line++;     //第一行显示下移
						ofl_state.item_index++;       //选择索引自加   						
					}					
				}
                if(ofl_state.item_index == ofl_item.item_num)
                {                  
                     sprintf(display_temp,"%02d",ofl_state.item_index-1);
                } 
                else
                {
                    sprintf(display_temp,"%02d",ofl_state.item_index);
                }                   
                oled_display_str(72,0,display_temp, &Font16);    
		        ofl_display();
			break;
			case  MSG_KEY_ENTER:          // 确认
//                memcpy(ofl_name, ofl_item.str[ofl_state.item_index],sizeof(ofl_name));
                ofl_flag.select = 1;
                ofl_flag.name = ofl_item.str[ofl_state.item_index-1];
                if(strcmp(ofl_flag.name, exit_item) != 0)   //选择方案
                {
                    cur_menu  = cur_menu-> next_menu;
                    disp_init = 1; 
                }
                else    //退出
                {
                    set_app_update(UPDATE_LINK_APP);  
                    SystemSoftReset();                
                }

			break; 	
			
		}
    }
}    
/*******************************************************************************
*	函 数 名: get_ofl_menu
*	功能说明: 
*	形    参: 
*	返 回 值: None
*******************************************************************************/
void get_ofl_menu(void)
{
    uint8_t i;
    char *exit_item = "EXIT";
    //获取待显示的方案名
    get_ofl_file_num(&ofl_item.item_num);
    for(i = 0; i < ofl_item.item_num; i++)
    {
        get_ofl_file_name (i, ofl_item.str[i]); 
    } 
    //添加退出item       
    memcpy( ofl_item.str[ofl_item.item_num], exit_item, strlen(exit_item));
    ofl_item.item_num++;
    //设置参数
    if( ofl_item.item_num >= PAGE_DISP_NUM)
			 ofl_state.disp_num = PAGE_DISP_NUM;
		else
			 ofl_state.disp_num =  ofl_item.item_num;
    if( ofl_item.item_num != 0)
        ofl_state.item_index = 1;
    else
        ofl_state.item_index = 0;
    ofl_state.cur_line = 0;
    ofl_state.first_line = 0;        
}
/*******************************************************************************
*	函 数 名: menu_init
*	功能说明: 菜单初始化
*	形    参: None
*	返 回 值: None
*******************************************************************************/
void menu_init(void)
{
	cur_menu  = &ofl_menu;
    disp_init = 1;
	get_ofl_menu();
	msg_init();
}
/*******************************************************************************
*	函 数 名: menu_display
*	功能说明: 菜单显示
*	形    参: None
*	返 回 值: None
*******************************************************************************/
void menu_display(void)
{   
    cur_menu->func(); 	
}
