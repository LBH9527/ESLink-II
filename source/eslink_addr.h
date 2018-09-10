#ifndef ESLINK_ADDR_H
#define ESLINK_ADDR_H

#define  ESLINK_BL1      
/* Device sizes */
//芯片地址信息
//0x00000000----->0x00020000 = 128K ROM
#define ESLINK_ROM_START               0x00000000
#define ESLINK_ROM_SIZE                0x00020000
// 0x1FFFE000---->0x20004000 = 24K RAM
#define ESLINK_RAM_START               0x1FFFE000
#define ESLINK_RAM_SIZE                0x00006000

/* ROM sizes */     
#define ESLINK_ROM_BL1_START            0x00000000          //boot1 起始地址
#define ESLINK_ROM_BL1_SIZE             0x00010000          //boot1 长度

#define ESLINK_ROM_BL2_START          0x00008000
#define ESLINK_ROM_BL2_SIZE           0x00008000

//#define ESLINK_ROM_APP_START          0x00017C00
//#define ESLINK_ROM_APP_SIZE           0x00017C00

//保存芯片信息和iap标志
#define ESLINK_ROM_UPDATE_USER_START   0x0001FC00
#define ESLINK_ROM_UPDATE_USER_SIZE    0x00000400
 
/* RAM sizes */   
#define ESLINK_RAM_APP_START           0x1FFFE000
#define ESLINK_RAM_APP_SIZE            0x00006000

//#define ESLINK_RAM_SHARED_START        0x20001F00
//#define ESLINK_RAM_SHARED_SIZE         0x00000100

/* Flash Programming Info */
//调试器芯片信息
#define ESLINK_SECTOR_SIZE             0x00000800           //扇区大小 2K
#define ESLINK_MIN_WRITE_SIZE          0x00000100

/* Current build */

#if defined(ESLINK_BL1)

#define ESLINK_APP_START            ESLINK_ROM_BL1_START
#define ESLINK_APP_SIZE             ESLINK_ROM_BL1_SIZE
#define ESLINK_UPDATE_START         ESLINK_ROM_BL2_START
#define ESLINK_UPDATE_SIZE          ESLINK_ROM_BL2_SIZE

#elif defined(ESLINK_BL2)

#define ESLINK_APP_START            ESLINK_ROM_BL2_START
#define ESLINK_APP_SIZE             ESLINK_ROM_BL2_START
#define ESLINK_UPDATE_START         ESLINK_ROM_APP_START
#define ESLINK_UPDATE_SIZE          ESLINK_ROM_APP_SIZE

#elif defined(ESLINK_APP)
#define ESLINK_APP_START            ESLINK_ROM_APP_START
#define ESLINK_APP_SIZE             ESLINK_ROM_APP_SIZE
#define ESLINK_UPDATE_START         0
#define ESLINK_UPDATE_SIZE          0

#else

#error "Build must be either bootloader or interface"

#endif

#endif
