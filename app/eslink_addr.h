#ifndef ESLINK_ADDR_H
#define ESLINK_ADDR_H

#define  ESLINK_BL1      
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
#define ESLINK_ROM_BL_SIZE              0x00005000

#define ESLINK_ROM_OFFLINE_START        0x00000000          
#define ESLINK_ROM_OFFLINE_SIZE         0x00020000          

#define ESLINK_ROM_LINK_START           0x0000A000
#define ESLINK_ROM_LINK_SIZE            0x00015000
//#define ESLINK_ROM_LINK_START           0x00000000
//#define ESLINK_ROM_LINK_SIZE            0x00020000

//保存芯片信息和iap标志
#define ESLINK_ROM_UPDATE_START    0x0001F800
#define ESLINK_ROM_UPDATE_SIZE     0x00000800

 
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
