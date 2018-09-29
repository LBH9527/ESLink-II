#include "es_common.h"
#include "oled.h"
#include "key.h"  
#include "offline_file.h" 
#include "menu.h"
#include "fonts.h"



//////菜单参数
////struct menu_t        
////{   
////    uint8_t select_item;            //当前选中项
////    uint8_t start_item;             //当前显示的项开始项
////    uint8_t total_item;             //每层菜单显示的项目总数
////    uint8_t select_item;            //当前选中项
////    uint8_t start_item;             //当前显示的项开始项
////    
////    struct menu_t *child_menu;    //子菜单
////    struct menu_t *parent_menu;   //上级菜单 ,如果是顶级则为null
////    
////    	uint8_t X;
////	uint8_t Top;
////	uint8_t Height;
////	uint8_t Width;
////    //
////}menu_param_t;




///**************************************************************************************
//* Variable definition                                               
//**************************************************************************************/
// enum {
//    STATE_MENU_LOGO ,    
//    STATE_MENU_OFL,
//    STATE_MENU_OFL_PRJ,
//} menu_state_t;

//// 菜单参数
//typedef struct  
//{
//     uint8_t ExitMark;             // 退出菜单(0-不退出，1-退出)
//     uint8_t Cursor;             // 光标值(当前光标位置)
//     uint8_t PageNo;             // 菜单页(显示开始项)
//     uint8_t Index;             // 菜单索引(当前选择的菜单项)
//     uint8_t DispNum;             // 显示项数(每页可以现在菜单项)
//     uint8_t MaxPage;             // 最大页数(最大有多少种显示页)
//     struct menu_param_t *next_menu;    //子菜单
//} menu_param_t;

//static menu_param_t ofl_param;      //脱机方案显示菜单

//typedef struct  
//{
//    uint8_t key_down;
//    uint8_t key_enter;
//    void (*operation) ();
//} menu_tag;

////待显示菜单项信息
//#define ITEM_LIST_MAX_NUM    10      
//typedef struct 
//{
//    uint8_t item_num; //待显示的条目数量
//    char *item[ITEM_LIST_MAX_NUM]; 
//} menu_table_t;
//menu_table_t  ofl_menu;
////每屏显示菜单数
//#define PAGE_DISP_NUM   3


///**************************************************************************************
//* FunctionName   : Menu_PrmtInit()
//* Description    : 初始化菜单参数
//* EntryParameter : param - 当前菜单参数, scr_num ：每屏最多显示项目数,  -item_num 当前菜单的数目
//* ReturnValue    : None
//**************************************************************************************/
//void menu_param_set(menu_param_t *param, uint8_t scr_num, uint8_t item_num)
//{
//    
//    param->ExitMark = 0;                     // 清除退出菜单标志
//    param->Cursor   = 0;                     // 光标清零
//    param->PageNo   = 0;                     // 页清零
//    param->Index    = 0;                     // 索引清零
//    param->DispNum  = scr_num;                   // 页最多显示项目数
//    if(item_num > scr_num)
//        param->MaxPage = item_num - scr_num + 1;  // 最多页数
//    else
//        param->MaxPage = 1;                          
//}
void menu_display_logo(void)
{
    FONT_T Font16;
    
    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
    Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
    Font16.Space = 2;			/* 文字间距，单位 = 像素 */	
    oled_display_str(35,12,"ESSEMI", &Font16);
//    Font16.FrontColor = 0;		/* 字体颜色 0 或 1 */
//    Font16.BackColor = 1;		/* 文字背景颜色 0 或 1 */
//    Font16.Space = 0;	
//    oled_display_str(28,32,"EsLink_II", &Font16);
    oled_display_str(0,32,"选择脱机方案", &Font16);

}
//void ofl_prj_diplay(uint8_t i, char *str)
//{
//    FONT_T Font16;
//    
//    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
//    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
//    Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
//    Font16.Space = 0;			/* 文字间距，单位 = 像素 */	
//    oled_display_str(0, i*16, str, &Font16);  
//}
//void ofl_prj_diplay_inv(uint8_t i, char *str)
//{
//    FONT_T Font16;
//    
//    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
//    Font16.FrontColor = 0;		/* 字体颜色 0 或 1 */
//    Font16.BackColor = 1;		/* 文字背景颜色 0 或 1 */
//    Font16.Space = 0;			/* 文字间距，单位 = 像素 */	
//    oled_display_str(0, i*16, str, &Font16); 

//}

///**************************************************************************************
//* FunctionName   : Menu_Display()
//* Description    : 显示菜单
//* EntryParameter : page - 显示页，dispNum - 每一页的显示项，cursor - 光标位置
//* ReturnValue    : None
//**************************************************************************************/
//void menu_display(menu_table_t *item, uint8_t page, uint8_t dispNum, uint8_t cursor)
//{
//    uint8_t i;
//    for (i=0; i<dispNum; i++)
//    {
//        if (cursor == i)
//        {
//            ofl_prj_diplay_inv((i+1)*2, item->list[page+i]);            // 反白显示菜单项
//        }
//        else
//        {
//            ofl_prj_diplay((i+1)*2, item->list[page+i]);             // 显示菜单项  
//        }
//    }
//}
///*******************************************************************************
//*	函 数 名: get_ofl_menu
//*	功能说明: 设置菜单显示
//*	形    参: num - 页菜单显示项，page - 最多页数
//*	返 回 值: None
//*******************************************************************************/
//void get_ofl_menu(void)
//{
//    uint8_t i;
//    //获取带显示的表项
//    get_ofl_file_num(&ofl_menu.item_num);
//    for(i = 0; i < ofl_menu.item_num; i++)
//    {
//        get_ofl_file_name (i,ofl_menu.list[i]); 
//    } 
//    //设置参数
//    menu_param_set(&ofl_param,PAGE_DISP_NUM, item_tbl.item_num);
//}
///**************************************************************************************
//* FunctionName   : menu_refrsh()
//* Description    : 菜单移动
//* EntryParameter : prmt - 菜单参数, key - 按键值
//* ReturnValue    : 有确认返回0，否则返回1
//**************************************************************************************/
//uint8_t menu_refrsh(struct menu_param_t *param, uint8_t key)
//{  
//    switch (key) 
//    {  
//        case KEY_DOWN:
//            if (param->Cursor < param->DispNum-1)                          // 光标没有到底，移动光标
//            {
//                param->Cursor++;                        // 光标向下移动
//            }
//            else                                                         // 光标到底
//            {
//                if (param->PageNo < param->MaxPage-1)                      // 页面没有到底，页面移动
//                {
//                    param->PageNo++;                       // 下翻一页
//                }
//                else                                                     // 页面和光标都到底，返回开始页
//                {
//                    param->Cursor = 0;
//                    param->PageNo = 0;
//                }
//            }
//            break;
//        case KEY_ENTER:          // 确认
//            param->Index = param->Cursor + param->PageNo;                   // 计算执行项的索引
//            menu_init();
//            break;
//        default:
//            break;
//    } 
//    menu_display(struct menu_table_t *item, uint8_t page, uint8_t dispNum, uint8_t cursor)
//}

//void menu_app(void)
//{
//    menu_state_t state;
//    uint8_t key_value;
//    state = get_prj_state() ;
//    if(key_read_data(&key_value) != 0)        //有按键按下
//    {
//        switch(state)
//        {
//             case STATE_MENU_LOGO :
//             
//                menu_display_logo();
//                break;
//             case STATE_MENU_OFL:
//                break;
//             case STATE_MENU_OFL_PRJ:
//                break;
//        }
//        (KEY_ENTER == key_value)
//        {
//            state ++;
//        
//        }
//        


//    }


//}
