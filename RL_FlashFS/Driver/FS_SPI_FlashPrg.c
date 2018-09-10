/*
*********************************************************************************************************
*
*	模块名称 : SPI Flash驱动
*	文件名称 : FS_SPI_FlashPrg.c
*	版    本 : V1.0
*	说    明 : 本文件是FlashFS所需的SPI Flash接口文件，使用芯片W25Q128FV，容量16MB。
*              多个函数做了修改，以支持SW25Q128BV。
*              
*	修改记录 :
*		版本号  日期        作者     说明

*
*********************************************************************************************************
*/
/*----------------------------------------------------------------------------
 *      RL-ARM - FlashFS
 *----------------------------------------------------------------------------
 *      Name:    FS_SPI_FLASHPRG.C 
 *      Purpose: Flash Programming Functions - Intel Serial Flash S33 (SPI)
 *      Rev.:    V4.20
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2011 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <File_Config.h>

/*----------------------------------------------------------------------------
  SPI Flash Driver instance definition
   sf0_drv: First  SPI Flash drive [S0:]
   sf1_drv: Second SPI Flash drive [S1:]
 *----------------------------------------------------------------------------*/

#define __DRV_ID  sf0_drv

/*----------------------------------------------------------------------------
  Local SPI Driver instance definition
   spi0_drv: no MC-SPI mode drives or MC drives in Native mode used
   spi1_drv: M0 used in SPI mode (spi0_drv is reserved for M0:)
 *----------------------------------------------------------------------------*/

#define __SPI     spi0_drv

/* Embedded Flash Driver Interface functions */
static BOOL Init        (U32 adr, U32 clk);
static BOOL UnInit      (void);
static BOOL ReadData    (U32 adr, U32 sz, U8 *buf);
static BOOL ProgramPage (U32 adr, U32 sz, U8 *buf);
static BOOL EraseSector (U32 adr);
static BOOL EraseChip   (void);

/* Embedded Flash Device Driver Control Block */
EFS_DRV __DRV_ID = {
  Init,
  UnInit,
  ReadData,
  ProgramPage,
  EraseSector,
  EraseChip
};


/* External SPI driver */
extern SPI_DRV    __SPI;
#define spi     (&__SPI)

/* Local definitions */
#define PAGE_SZ                 256     /* Page Size */

/* SPI Flash Commands */
#define SPI_WRITE_SR            0x01
#define SPI_PAGE_PROGRAM        0x02
#define SPI_READ_DATA           0x03
#define SPI_WRITE_DISABLE       0x04
#define SPI_READ_SR             0x05
#define SPI_WRITE_ENABLE        0x06
#define SPI_CLEAR_SR_FLAGS      0x30
#define SPI_SECTOR_ERASE        0xD8
#define SPI_CHIP_ERASE          0xC7	

/* Status Register Bits */
#define SR_WIP                  0x01
#define SR_E_FAIL               0x20
#define SR_P_FAIL               0x40


/*--------------------------- Init ------------------------------------------*/

static BOOL Init (U32 adr, U32 clk)  {
  /* Initialize flash programming functions. */
	
  if (spi->Init () == __FALSE) {
    /* SPI driver init failed. */
    return (__FALSE);
  }

  /* Set the clock rate to 10 MHz. */
  if (spi->BusSpeed (10000000) == __FALSE) {
    return (__FALSE);
  }

//  No need, the default congfig is 0x00
//  spi->SetSS (0);
//  spi->Send (SPI_WRITE_SR);    
//  spi->Send (0x00);
//  spi->SetSS (1);

//  W25Q128 have no this config..  
//  spi->SetSS (0);
//  spi->Send (SPI_CLEAR_SR_FLAGS); 
//  spi->SetSS (1);

  return (__TRUE);
}


/*--------------------------- UnInit ----------------------------------------*/

static BOOL UnInit (void)  {
   /* Uninitialize flash programming functions. */

   return (spi->UnInit ());
}


/*--------------------------- ReadData --------------------------------------*/

static BOOL ReadData (U32 adr, U32 sz, U8 *buf)  {
  /* Read a block of Data from Flash Memory. */ 

  spi->SetSS (0);
  spi->Send (SPI_READ_DATA);
  spi->Send ((U8)(adr >> 16));
  spi->Send ((U8)(adr >>  8));
  spi->Send ((U8)(adr >>  0));
  spi->RecBuf (buf, sz);
  spi->SetSS (1);

  return (__TRUE);
}


/*--------------------------- ProgramPage -----------------------------------*/

static BOOL ProgramPage (U32 adr, U32 sz, U8 *buf) {
  /* Program Page in Flash Memory. */ 
  U32 cnt;
  U8  sr;

  while (sz) {
    cnt = PAGE_SZ - (adr & (PAGE_SZ - 1));
    if (cnt > sz) cnt = sz;

    /* Write Enable */
    spi->SetSS (0);
    spi->Send (SPI_WRITE_ENABLE);
    spi->SetSS (1);

    /* Program Page */
    spi->SetSS (0);
    spi->Send (SPI_PAGE_PROGRAM);
    spi->Send ((U8)(adr >> 16));
    spi->Send ((U8)(adr >>  8));
    spi->Send ((U8)(adr >>  0));
    spi->SendBuf (buf, cnt);
    spi->SetSS (1);

    /* Wait until done */
    spi->SetSS (0);
    spi->Send (SPI_READ_SR);
    do {
      sr = spi->Send (0xFF);
    } while (sr & SR_WIP);
    spi->SetSS (1);

//  W25Q128 have no this config..
//    /* Check for Error */
//    if (sr & SR_P_FAIL) {
//      spi->SetSS (0);
//      spi->Send (SPI_CLEAR_SR_FLAGS);
//      spi->SetSS (1);
//      return (__FALSE);
//    }

    adr += cnt;
    buf += cnt;
    sz  -= cnt;
  }
  return (__TRUE);
}

/*--------------------------- EraseSector -----------------------------------*/

static BOOL EraseSector (U32 adr) {
  /*  Erase Sector in Flash Memory. */
  U8  sr;

  /* Write Enable */
  spi->SetSS (0);
  spi->Send (SPI_WRITE_ENABLE);
  spi->SetSS (1);

  /* Erase Sector */
  spi->SetSS (0);
  spi->Send (SPI_SECTOR_ERASE);
  spi->Send ((U8)(adr >> 16));
  spi->Send ((U8)(adr >>  8));
  spi->Send ((U8)(adr >>  0));
  spi->SetSS (1);

  /* Wait until done */
  spi->SetSS (0);
  spi->Send (SPI_READ_SR);
  do {
    sr = spi->Send (0xFF);
  } while (sr & SR_WIP);
  spi->SetSS (1);

//  W25Q128 have no this config..
//  /* Check for Error */
//  if (sr & SR_E_FAIL) {
//    spi->SetSS (0);
//    spi->Send (SPI_CLEAR_SR_FLAGS);
//    spi->SetSS (1);
//      return (__FALSE);
//  }
  
  return (__TRUE);
}

/*--------------------------- EraseChip -----------------------------------*/

static BOOL EraseChip (void)  
{
/*  Erase Chip in Flash Memory. */
    U8  sr;
	
	/* Write Enable */
	spi->SetSS (0);
	spi->Send (SPI_WRITE_ENABLE);
	spi->SetSS (1);

	/* Erase Chip */
	spi->SetSS (0);
	spi->Send (SPI_CHIP_ERASE);
	spi->SetSS (1);

	/* Wait until done */
	spi->SetSS (0);
	spi->Send (SPI_READ_SR);
	do {
	sr = spi->Send (0xFF);
	} while (sr & SR_WIP);
	spi->SetSS (1);

//  W25Q128 have no this config..
//	/* Check for Error */
//	if (sr & SR_E_FAIL) {
//	spi->SetSS (0);
//	spi->Send (SPI_CLEAR_SR_FLAGS);
//	spi->SetSS (1);
//	  return (__FALSE);
//	}
	
	return (__TRUE);
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
