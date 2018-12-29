#ifndef _ESLINK_APP_ADDR_H
#define _ESLINK_APP_ADDR_H
 
/* Device sizes */
//芯片地址信息
//0x00000000----->0x00020000 = 128K ROM
#define ESLINK_ROM_START                0x00000000
#define ESLINK_ROM_SIZE                 0x00020000
// 0x1FFFE000---->0x20004000 = 24K RAM
#define ESLINK_RAM_START                0x1FFFE000
#define ESLINK_RAM_SIZE                 0x00006000

/* ROM sizes */    
#define ESLINK_ROM_BL_START             0x00000000
#define ESLINK_ROM_BL_SIZE              0x00004000         //16K

#define ESLINK_ROM_OFFLINE_START        0x00004000          
#define ESLINK_ROM_OFFLINE_SIZE         0x00008000         //32K  
//#define ESLINK_ROM_OFFLINE_START           0x00000000
//#define ESLINK_ROM_OFFLINE_SIZE            0x00020000

#define ESLINK_ROM_LINK_START           0x0000C000
#define ESLINK_ROM_LINK_SIZE            0x00013000         //76K
//#define ESLINK_ROM_LINK_START           0x00000000
//#define ESLINK_ROM_LINK_SIZE            0x00020000   

//保存目标芯片信息 
#define ESLINK_ROM_CFG_START        0x0001F000  //2K
#define ESLINK_ROM_CFG_SIZE         0x00000800
//ES_LINK版本信息和iap标志
#define ESLINK_ROM_INFO_START       0x0001F800  //2K
#define ESLINK_ROM_INFO_SIZE        0x00000800
 

///* RAM sizes */   
//#define ESLINK_RAM_APP_START           0x1FFFE000
//#define ESLINK_RAM_APP_SIZE            0x00006000

//#define ESLINK_RAM_SHARED_START        0x20001F00
//#define ESLINK_RAM_SHARED_SIZE         0x00000100

/* Flash Programming Info */
//调试器芯片信息
#define ESLINK_SECTOR_SIZE             0x00000800           //扇区大小 2K
#define ESLINK_MIN_WRITE_SIZE          0x00000100


#endif
