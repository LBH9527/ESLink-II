/**
  **************************************************************************************
  * @file    bsp_flash.h
  * @brief   flash File.
  *
  * @version V0.01
  * @data    4/3/2018
  * @author  Eastsoft AE Team
  * @note
  *
  * Copyright (C) 2018 Shanghai Eastsoft Microelectronics Co., Ltd. ALL rights reserved.
  *
  **************************************************************************************
  */

#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#include "stdint.h"
#include "board.h"

/* Control Pin */
#define SF_CS_INIT(output)                                                 \
    GPIO_PinWrite(FLASH_DSPI_INT_GPIO, FLASH_DSPI_INT_PIN, output); \
    FLASH_DSPI_INT_GPIO->PDDR |= (1U << FLASH_DSPI_INT_PIN) /*!< Enable target LED_RED */
#define SF_CS_CLR() \
    GPIO_PortClear(FLASH_DSPI_INT_GPIO, 1U << FLASH_DSPI_INT_PIN) /*!< Turn on target LED_RED */
#define SF_CS_SET() \
    GPIO_PortSet(FLASH_DSPI_INT_GPIO, 1U << FLASH_DSPI_INT_PIN) /*!< Turn off target LED_RED */
    
/**
 * error code
 */
typedef enum {
    SF_SUCCESS = 0,                                      /**< success */
    SF_ERR_WRITE = 1,                                    /**< write error */
    SF_ERR_READ = 2,                                     /**< read error */
    SF_ERR_TIMEOUT = 3,                                  /**< timeout error */
    SF_ERR_ADDR_OUT_BOUND = 4,                        /**< address is out of flash bound */
} sf_err;

typedef struct
{
	uint32_t chip_id;		/* 芯片ID */
	char ChipName[16];		/* 芯片型号字符串，主要用于显示 */
	uint32_t capacity;		/* 总容量 */
    uint32_t block_size;			/* The Block size in the flash */
	uint32_t block_start;		/* The start of available block*/
	uint32_t block_end;			/* The end of available block */
}SPI_FLASH_t;

extern SPI_FLASH_t g_flash;

/*******************************************************************************
  函数声明
********************************************************************************/
void spi_flash_init(void);
//void sf_read_info(void);
sf_err sf_erase_chip(void);
sf_err sf_erase_sector(uint32_t offset, uint32_t length);
sf_err sf_erase_block(uint32_t offset, uint32_t length);
sf_err spi_flash_read( uint32_t _uiReadAddr, uint8_t * _pBuf,uint32_t _uiSize );
sf_err spi_flash_write( uint32_t _uiWriteAddr, const uint8_t* _pBuf, uint32_t _usWriteSize);
sf_err sf_page_write(uint32_t addr,const uint8_t * _pBuf, uint32_t _usSize);

uint8_t sf_CmpData(uint32_t _uiSrcAddr, const uint8_t *_ucpTar, uint32_t _uiSize);
#endif
