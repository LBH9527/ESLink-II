/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"			/* FatFs lower layer API */
#include "spi_flash.h"      
#include "sflash_port.h"

/* Definitions of physical drive number for each media */
#define FS_SPI_FLASH  0

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{
//    DSTATUS stat;
	switch (pdrv) {

		case FS_SPI_FLASH :
            if( sf_read_info() != SF_SUCCESS)
                return STA_NOINIT;

            // translate the reslut code here

			return RES_OK;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber (0..) */
)
{
	DSTATUS stat;

	switch (pdrv)
	{
		case FS_SPI_FLASH :
			stat = 0;
			return stat;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{

	switch (pdrv) {

		case FS_SPI_FLASH :
             /* 扇区偏移2MB，外部Flash文件系统空间放在SPI Flash后面6MB空间 */ 
            sector += 512;
			spi_flash_read(sector<<12, buff, count <<12);
			return RES_OK;

	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{

	switch (pdrv) {
		
		case FS_SPI_FLASH :
			{
                /* 扇区偏移2MB，外部Flash文件系统空间放在SPI Flash后面6MB空间 */ 
                sector += SF_FATFS_OFFSET;
                sf_erase_sector(sector<<12, 4096);
                spi_flash_write(sector<<12, buff, count<<12 );
                // translate the reslut code here

				return RES_OK;
			}
	
	}
	
	return RES_PARERR;
}
#endif


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
//	int result;

	switch (pdrv) {
	case FS_SPI_FLASH :

            // Process of the command for the RAM drive
                switch (cmd) {
            case GET_SECTOR_COUNT:
                *(DWORD * )buff = SF_FATFS_SECTOR_COUNT;		//sector˽   1536*4096/1024/1024=6(MB)
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
DWORD get_fattime (void)
{
	
	return	  ((DWORD)(2013 - 1980) << 25)	/* Year = 2013 */
			| ((DWORD)1 << 21)				/* Month = 1 */
			| ((DWORD)1 << 16)				/* Day_m = 1*/
			| ((DWORD)0 << 11)				/* Hour = 0 */
			| ((DWORD)0 << 5)				/* Min = 0 */
			| ((DWORD)0 >> 1);				/* Sec = 0 */
//    return 0;
}
