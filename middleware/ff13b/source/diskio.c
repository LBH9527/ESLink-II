/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "spi_flash.h"

/* Definitions of physical drive number for each drive */
#define DEV_FLASH_SPI		0	



/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_FLASH_SPI :
        return 0;

	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_FLASH_SPI :
		spi_flash_init();       //初始化
		if( sf_read_info() != 0)
           stat = STA_NOINIT;

		// translate the reslut code here
        stat &= ~STA_NOINIT;;
		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_FLASH_SPI :
		// translate the arguments here
        result = spi_flash_read(sector<<12, buff, count <<12);
		// translate the reslut code here
        res = RES_OK;
		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;
    
	switch (pdrv) {
	case DEV_FLASH_SPI :
		// translate the arguments here

        sf_erase_sector(sector<<12, 4096);
        result = spi_flash_write(sector<<12, buff, count<<12 );
		// translate the reslut code here
        res = RES_OK;
		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_FLASH_SPI :

            // Process of the command for the RAM drive
                switch (cmd) {
            case GET_SECTOR_COUNT:
                *(DWORD * )buff = 2048;		//sector˽   2048*4096/1024/1024=8(MB)
            break;
            case GET_SECTOR_SIZE :     // Get R/W sector size (WORD)

                *(WORD * )buff = 4096;		//flash扇区大小
            break;
            case GET_BLOCK_SIZE :      // Get erase block size in unit of sector (DWORD)
                *(DWORD * )buff = 1;		//flashӔ1ٶsectorΪخСӁԽեλ
            break;
        }
        res =  RES_OK;
		return res;
	}

	return RES_PARERR;
}

