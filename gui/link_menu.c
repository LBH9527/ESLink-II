// 联机/脱机显示界面
#include "es_common.h"
#include "oled.h"

#include "menu.h"
#include "fonts.h"

#include "update.h"
#include "settings_rom.h"
#include "sflash_port.h"
#include "cortex_m.h"
#include "eeprom_port.h"
#include "offline_def.h"

//static char ofl_name[OFL_FILE_NAME_MAX_LEN+1] = { '\0'};
typedef void (* menu_func)(void );

#define PAGE_DISP_NUM   3    //每屏显示菜单数

//struct ofl_menu_state
//{
//    uint8_t disp_num;                // 每页显示的菜单项个数    
//    uint8_t item_index;		        //当前选定菜单对应的索引号
//    uint8_t cur_line ;              //当前选择的行
//    uint8_t first_line;             //屏幕第一行显示的索引号   
////    uint8_t MaxPage;               // 最大页数(最大有多少种显示页)    
//    
//} ;
//struct ofl_menu_state ofl_state;

struct menu_item
{
    uint8_t item_num;           //待显示的条目数量
    char str[MAX_PAJ_NUM][OFL_FILE_NAME_MAX_LEN+1]; 
	uint8_t state;				//所选择的子菜单
} ;
struct menu_item  ofl_item ;   //脱机菜单显示项 

struct menu_t
{ 	
//	struct menu_item *items;          //菜单项目
	menu_func func;                     //菜单处理函数
	struct menu_t *next_menu;            //下级菜单
//    struct menu_t *child_menu;    //子菜单
//    struct menu_t *parent_menu;   //上级菜单 ,如果是顶级则为null
}  ; 
struct menu_t *cur_menu;				//当前菜单窗口  

void ofl_select_display(void);
void logo_display(void );
void update_display(void );
void ofl_program_display(void);
void idle_func(void)
{
  /* Nothing to execute: return */
  return;
}
//脱机编程窗口
struct menu_t ofl_prog_menu = 
{
    ofl_program_display,
    NULL,
}; 

 
//logo 菜单窗口
struct menu_t logo_menu = 
{  
	logo_display,
	NULL,
} ;

//8位机序列号 显示
static void serial_number_8bit_display(uint16_t y,uint8_t *buf, uint8_t size)
{
    uint8_t i;
    FONT_T Font16;
    char disp_temp[2+1] = {'\0'};
    
    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
    Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
    Font16.Space = 0;			/* 文字间距，单位 = 像素 */	 

    for(i=0; i< size/2; i++)
    {
         sprintf(disp_temp,"%02X",buf[i*2]);    
         oled_display_str( i*16, y, disp_temp, &Font16);  
    } 
}
//32位机序列号 显示
static void serial_number_32bit_display(uint16_t y,uint8_t *buf, uint8_t size)
{
    uint8_t i;
    FONT_T Font16;
    char disp_temp[2+1] = {'\0'};
    
    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
    Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */     
    Font16.Space = 0;			/* 文字间距，单位 = 像素 */	   

    for(i=0; i< 4; i++)
    {
        sprintf(disp_temp,"%02X",buf[3-i]);    
        oled_display_str( i*16, y, disp_temp, &Font16);  
    }          
    if(size == 0x08)
    {
        for(i=4; i< size; i++)
        {
            sprintf(disp_temp,"%02X",buf[7 + 4 -i]);    
            oled_display_str( i*16, y, disp_temp, &Font16);  
        }   
    }
}
//脱机序列号显示
static void ofl_sn_display(uint8_t state)
{
    char display_temp[16+1] = {'\0'};
    ofl_serial_number_t sn_info;                //序列号信息
    FONT_T Font16;
    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.Space = 0;
    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
	Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */

    get_offline_serial_number((uint8_t*)&sn_info, sizeof(ofl_serial_number_t));
    if( state & 0x01)
    {
        if(sn_info.state != OFL_SERIALNUM_DISABLE)
        {
            if(sn_info.read_mode  == OFL_SERIALNUM_READ_USE_IAP)  //32位机 IAP方式
            {
                serial_number_32bit_display( 32 , sn_info.sn.data, sn_info.sn.size);
            }
            else        //8位机
            {
                serial_number_8bit_display( 32 , sn_info.sn.data, sn_info.sn.size);
            } 
        }   
    } 
    if(state & 0x02)
    {
        //烧录成功个数
        oled_display_str(0,48,"OK:             ", &Font16);        
        sprintf(display_temp,"%08d", sn_info.success_count);         
        oled_display_str(24,48, display_temp  , &Font16);      
    }       
} 
 
/*******************************************************************************
*	函 数 名: ofl_program_display
*	功能说明: 
*	形    参: 
*	返 回 值: None
*******************************************************************************/ 
void ofl_program_display(void)
{
    static uint8_t disp_init = 0 ;  	
    static ofl_prj_info_t ofl_prj_info;         //脱机方案信息
    
    char display_temp[16+1] = {'\0'};
    uint8_t msg = MSG_NULL;
    FONT_T Font16;
    Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
    Font16.Space = 0;
    Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
	Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
    
    if(disp_init == 0)
    {
        //获取方案信息
        online_file_read(OFL_PROG_INFO, 0,(uint8_t*) &ofl_prj_info, sizeof(ofl_prj_info_t));
        oled_clr_scr(0x00);	
        sprintf(display_temp,"%s", (char*)ofl_prj_info.chip_name);
        oled_display_str(0,0,display_temp  , &Font16);             //芯片名称  
        
        oled_display_str(0,16,"S:      C:    ", &Font16);        //配置字和flash数据累加和 与 CRC校验和
        Font16.FrontColor = 0;		/* 字体颜色 0 或 1 */
        Font16.BackColor = 1;		/* 文字背景颜色 0 或 1 */
        sprintf(display_temp,"%04X", ofl_prj_info.checksum);
        oled_display_str(20,16,display_temp  , &Font16);             //芯片累加和
        ofl_prj_info.crc &= 0x0000ffff;                             //显示低字节
        sprintf(display_temp,"%04X", ofl_prj_info.crc);
        oled_display_str(84,16,display_temp  , &Font16);             //芯片CRC校验和           
        Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
        Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */          
        ofl_sn_display(0x03);      
        disp_init = 1;
    }     
    
    gui_msg_read_data(&msg);
	if(msg != MSG_NULL)         
    switch(msg)
    {      
        case MSG_PROG_MODE_CHECK:
            oled_clr_scr(0x00);	
           //获取方案信息
//            online_file_read(OFL_PROG_INFO, 0,(uint8_t*) &ofl_prj_info, sizeof(ofl_prj_info_t));
            oled_clr_scr(0x00);	
            sprintf(display_temp,"%s", (char*)ofl_prj_info.chip_name);
            oled_display_str(0,0,display_temp  , &Font16);             //芯片名称             
            oled_display_str(0,16,"S:      C:      ", &Font16);        //配置字和flash数据累加和 与 CRC校验和
            Font16.FrontColor = 0;		/* 字体颜色 0 或 1 */
            Font16.BackColor = 1;		/* 文字背景颜色 0 或 1 */
            sprintf(display_temp,"%04X", ofl_prj_info.checksum);
            oled_display_str(20,16,display_temp  , &Font16);             //芯片累加和
            ofl_prj_info.crc &= 0x0000ffff;                             //显示低字节
            sprintf(display_temp,"%04X", ofl_prj_info.crc);
            oled_display_str(84,16,display_temp  , &Font16);             //芯片CRC校验和           
            Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
            Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
            
            ofl_sn_display(0x03);      
            break ;
        case MSG_PROG_ING:
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"  programing  ", &Font16); 
            ofl_sn_display(0x01);             
            break;
        case MSG_PROG_OK:
            oled_clr_scr(0x00);	            
            oled_display_str(0,16,"      OK        ", &Font16); 
            ofl_sn_display(0x02); 
            break;
        case MSG_ERR:
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"00 Error        ", &Font16); 
        case MSG_ERR_COUNT_FULL:
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"01 Count Full   ", &Font16); 
            break;
        case MSG_ERR_CHIPID_CHECK:
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"02 Chipid Check ", &Font16); 
            break;
        case MSG_ERR_ERASE:                         //擦除失败
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"03 Erase        ", &Font16);    
            break;
        case MSG_ERR_CHECK_EMPTY:                   //查空
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"04 Check Empty  ", &Font16);    
            break;            
        case MSG_ERR_PROG:                          //编程失败
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"05 Program      ", &Font16);    
            break;
        case MSG_ERR_VERIFY:                        //校验
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"06 Verify       ", &Font16);    
            break;
        case MSG_ERR_ENCRYPT:                       //加密
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"07 Encrypt      ", &Font16);    
            break;
        
//        case MSG_ERR_PROG_INTF: 
//            oled_clr_scr(0x00);	
//            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
//            oled_display_str(0,32,"08         ", &Font16);    
//            break;
        case MSG_ERR_ENTRY_MODE:
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"08 Entry Mode   ", &Font16);   
            break;
#if ESLINK_RTC_ENABLE             
        case MSG_ERR_RTC_CALI:
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"09 RTC Calibrate", &Font16);   
            break;
        case MSG_ERR_RTC_VERIFY:
            oled_clr_scr(0x00);	
            oled_display_str(0,16,"   PROG FAILE   ", &Font16); 
            oled_display_str(0,32,"10 RTC Verify   ", &Font16);   
            break;
#endif       
        case MSG_KEY_ENTER:                         //长按回到联机模式      
            if(set_link_mode(ESLINK_ONLINE_MODE) != TRUE )
            {
                 //TODO：设置失败
            }
            else
            {                    
                SystemSoftReset(); 
            }
        default:
            break;
    }     
}  

/*******************************************************************************
*	函 数 名: logo_display
*	功能说明: 联机时显示logo
*	形    参: 
*	返 回 值: None
*******************************************************************************/
void logo_display(void)
{
	uint8_t msg = MSG_NULL;
	FONT_T Font16;
	static uint8_t disp_init = 0 ;
	uint32_t version = get_hardware_version();
    char display_temp[8+1] = {'\0'};
	if(disp_init == 0)
	{  
        oled_clr_scr(0x00);	/* 清屏，0x00表示黑底； 0xFF 表示白底 */
		Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
		Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
		Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
		Font16.Space = 2;			/* 文字间距，单位 = 像素 */	
		oled_display_str(35,8,"ESSEMI", &Font16);
        Font16.FrontColor = 0;		/* 字体颜色 0 或 1 */
        Font16.BackColor = 1;		/* 文字背景颜色 0 或 1 */
        Font16.Space = 0;	
        oled_display_str(0,28,"   ESLink-II    ", &Font16);	
        Font16.FrontColor = 1;		/* 字体颜色 0 或 1 */
        Font16.BackColor = 0;		/* 文字背景颜色 0 或 1 */
        sprintf(display_temp,"%08x", version);
//        display_temp[0] =  version >> 
        display_temp[2] = '.';
        display_temp[4] = '.';
        display_temp[6] = '.';
        oled_display_str(0,48,"        V", &Font16);	
        oled_display_str(72,48,&display_temp[1], &Font16);	
		disp_init = 1;
	}
    gui_msg_read_data(&msg);
	if(msg ==  MSG_KEY_ENTER)       //有按键按下
    {
        cur_menu  = cur_menu-> next_menu;	
        set_app_update(UPDATE_OFFLINE_APP);
        SystemSoftReset();  
    }
} 
/*******************************************************************************
*	函 数 名: 初始化界面
*	功能说明: 联机时，初始化设备
*	形    参: 
*	返 回 值: None
*******************************************************************************/
void init_display(void)
{
//	uint8_t msg = MSG_NULL;
	FONT_T Font16;
	static uint8_t disp_init = 0 ;
	
	if(disp_init == 0)
	{  
        oled_clr_scr(0x00);	/* 清屏，0x00表示黑底； 0xFF 表示白底 */
		Font16.FontCode = FC_ST_16;	/* 字体代码 16点阵 */
        Font16.FrontColor = 0;		/* 字体颜色 0 或 1 */
        Font16.BackColor = 1;		/* 文字背景颜色 0 或 1 */
        Font16.Space = 0;	
        oled_display_str(0,16,"Initializing....", &Font16);	
		disp_init = 1;
	}
//    gui_msg_read_data(&msg);
//	if(msg ==  MSG_KEY_ENTER)       //有按键按下
//    {
//        cur_menu  = cur_menu-> next_menu;	
//        set_app_update(UPDATE_OFFLINE_APP);
//        SystemSoftReset();  
//    }
} 
/*******************************************************************************
*	函 数 名: menu_init
*	功能说明: 菜单初始化
*	形    参: None
*	返 回 值: None
*******************************************************************************/
void menu_init(uint8_t mode)
{         
    if (mode == MENU_ONLINE_MODE)
        cur_menu  = &logo_menu;
    else if (mode == MENU_OFFLINE_MODE)
        cur_menu  = &ofl_prog_menu;
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

