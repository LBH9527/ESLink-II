/*
********************************************************************************
*
*	模块名称 : SPI Flash驱动
*	文件名称 : FS_SPI_FlashDev.h
*	版    本 : V1.0
*	说    明 : 本文件是FlashFS所需的SPI Flash头文件，使用芯片W25Q128FV，容量16MB。
*              这里将16MB分为256块，每块大小64KB。
*                       
*	修改记录 :
*		版本号  日期        作者     说明
*
*
********************************************************************************
*/
/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.H:  Device Description for Intel 64MBit Serial Flash      */
/*                                                   Memory (S33)      */
/*                                                                     */
/***********************************************************************/

#define SPI_FLASH_DEVICE                             \
  DSB(0x10000, 0x000000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x010000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x020000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x030000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x040000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x050000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x060000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x070000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x080000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x090000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0A0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0B0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0C0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0D0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0E0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0F0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x100000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x110000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x120000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x130000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x140000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x150000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x160000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x170000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x180000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x190000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x1A0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x1B0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x1C0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x1D0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x1E0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x1F0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x200000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x210000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x220000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x230000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x240000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x250000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x260000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x270000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x280000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x290000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x2A0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x2B0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x2C0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x2D0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x2E0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x2F0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x300000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x310000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x320000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x330000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x340000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x350000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x360000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x370000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x380000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x390000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x3A0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x3B0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x3C0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x3D0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x3E0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x3F0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x400000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x410000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x420000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x430000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x440000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x450000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x460000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x470000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x480000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x490000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x4A0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x4B0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x4C0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x4D0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x4E0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x4F0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x500000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x510000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x520000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x530000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x540000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x550000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x560000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x570000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x580000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x590000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x5A0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x5B0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x5C0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x5D0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x5E0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x5F0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x600000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x610000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x620000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x630000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x640000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x650000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x660000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x670000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x680000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x690000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x6A0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x6B0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x6C0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x6D0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x6E0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x6F0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x700000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x710000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x720000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x730000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x740000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x750000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x760000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x770000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x780000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x790000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x7A0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x7B0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x7C0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x7D0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x7E0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x7F0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x800000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x810000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x820000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x830000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x840000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x850000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x860000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x870000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x880000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x890000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x8A0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x8B0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x8C0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x8D0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x8E0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x8F0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x900000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x910000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x920000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x930000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x940000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x950000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x960000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x970000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x980000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x990000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x9A0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x9B0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x9C0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x9D0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x9E0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0x9F0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA00000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA10000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA20000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA30000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA40000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA50000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA60000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA70000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA80000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xA90000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xAA0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xAB0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xAC0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xAD0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xAE0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xAF0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB00000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB10000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB20000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB30000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB40000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB50000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB60000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB70000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB80000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xB90000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xBA0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xBB0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xBC0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xBD0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xBE0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xBF0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC00000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC10000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC20000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC30000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC40000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC50000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC60000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC70000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC80000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xC90000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xCA0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xCB0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xCC0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xCD0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xCE0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xCF0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD00000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD10000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD20000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD30000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD40000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD50000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD60000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD70000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD80000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xD90000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xDA0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xDB0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xDC0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xDD0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xDE0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xDF0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE00000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE10000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE20000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE30000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE40000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE50000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE60000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE70000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE80000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xE90000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xEA0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xEB0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xEC0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xED0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xEE0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xEF0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF00000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF10000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF20000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF30000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF40000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF50000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF60000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF70000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF80000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xF90000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xFA0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xFB0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xFC0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xFD0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xFE0000),     /* Sector Size 64kB */ \
//  DSB(0x10000, 0xFF0000),     /* Sector Size 64kB */ \

#define SF_NSECT    32
