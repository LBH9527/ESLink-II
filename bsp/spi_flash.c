/**
  **************************************************************************************
  * @file    bsp_flash.c
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
//SPI FLASH信息    
//MX25L64  总容量 = 64M bit = 8M bytes      地址范围： 0 --- 7F FFFF    
 
#include "spi_flash.h"
#include "es_common.h"


/* 定义串行Flash ID */
enum
{
	MX25L64_ID      = 0xC22017,
	W25Q64BV_ID     = 0xEF4017
};

#define SF_STATUS_BUSY = (1 << 0),                  /**< busing  wip*/
#define SF_STATUS_WEL = (1 << 1),                   /**< write enable latch */


/* command list */
#define CMD_WRSR                    (0x01)  /* Write Status Register */
#define CMD_PP                      (0x02)  /* Page Program */
#define CMD_READ                    (0x03)  /* Read Data */
#define CMD_WRDI                    (0x04)  /* Write Disable */
#define CMD_RDSR                   (0x05)  /* Read Status Register-1 */
#define CMD_WREN                    (0x06)  /* Write Enable */
#define CMD_FAST_READ               (0x0B)  /* Fast Read */
#define CMD_ERASE_4K                (0x20)  /* Sector Erase:4K */
//#define CMD_RDSR2                   (0x35)  /* Read Status Register-2 */
//#define CMD_ERASE_32K               (0x52)  /* 32KB Block Erase */
#define CMD_JEDEC_ID                (0x9F)  /* Read JEDEC ID */
#define CMD_ERASE_full              (0xC7)  /* Chip Erase */
#define CMD_ERASE_64K               (0xD8)  /* 64KB Block Erase */
#define CMD_MANU_ID                 (0x90)

#define DUMMY                       (0xFF)

#define FLASH_ERASE_CMD             CMD_ERASE_4K
#define FLASH_BLOCK_SIZE            4096
#define FLASH_PAGE_SIZE             256


SPI_FLASH_t g_flash;

int sf_read_info(void);
static void sf_write_enable(void);
//static void sf_write_status(uint8_t _ucValue);

static void spi_delay_ms(uint32_t delay) 
{   
    delay *= ((SystemCoreClock/1000U) + (4-1U)) / 4;
    while (--delay);
}

static void sf_set_cs(uint8_t _level)
{
    if (_level == 0)
    {
        SF_CS_CLR(); 
    }
    else
    {
        SF_CS_SET();
    }
}
void spi_falsh_hw_init(void)
{   
    uint32_t srcClock_Hz;
    dspi_master_config_t masterConfig;
    
  /* Master config */
    masterConfig.whichCtar = kDSPI_Ctar0;
    masterConfig.ctarConfig.baudRate = FLASH_DSPI_BAUDRATE;
    masterConfig.ctarConfig.bitsPerFrame = 8U;
    masterConfig.ctarConfig.cpol = kDSPI_ClockPolarityActiveLow;
    masterConfig.ctarConfig.cpha = kDSPI_ClockPhaseSecondEdge;
    masterConfig.ctarConfig.direction = kDSPI_MsbFirst;
    masterConfig.ctarConfig.pcsToSckDelayInNanoSec        = 1000000000U / masterConfig.ctarConfig.baudRate ;
    masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec    = 1000000000U / masterConfig.ctarConfig.baudRate ;
    masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 1000000000U / masterConfig.ctarConfig.baudRate ;

//    masterConfig.whichPcs = FLASH_DSPI_MASTER_PCS_FOR_INIT;
//    masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

    masterConfig.enableContinuousSCK = false;
    masterConfig.enableRxFifoOverWrite = false;
    masterConfig.enableModifiedTimingFormat = false;
    masterConfig.samplePoint = kDSPI_SckToSin0Clock;

    srcClock_Hz = FLASH_DSPI_CLK_FREQ;
    DSPI_MasterInit(FLASH_DSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);   
    SF_CS_INIT(0);
    
}

 uint8_t flash_send_byte(uint8_t byte)
{    
    uint32_t SPITimeout = 0xFFFF;
     uint8_t data;
    
    SPI1->PUSHR =  SPI_PUSHR_TXDATA(byte);
    
//    /* waitting for complete */
//    if(!(SPI1->RSER & SPI_RSER_TCF_RE_MASK)) /* if it is polling mode */
//    {
//        while(!SPI1->SR & SPI_SR_TCF_MASK);
//        SPI1->SR |= SPI_SR_TCF_MASK;
//    }
    /* Wait till TCF sets */
//    while (!(SPI1->SR & SPI_SR_TCF_MASK))
//    {
//        if ((SPITimeout--) == 0) 
//            return SF_ERR_TIMEOUT;
//    }
//    SPI1->SR |= SPI_SR_TCF_MASK;
//    
//    return (uint8_t)SPI1->POPR;
    
    while( !( SPI1->SR & SPI_SR_TCF_MASK) )  
    {
         if ((SPITimeout--) == 0) 
            return SF_ERR_TIMEOUT;
    }
    SPI1->SR |= SPI_SR_TCF_MASK;    //Clears the DSPI status flag. 
    while( !( SPI1->SR & SPI_SR_RFDF_MASK) ) 
    {
         if ((SPITimeout--) == 0) 
            return SF_ERR_TIMEOUT;
    }
    data =  (uint8_t)SPI1->POPR;
    SPI1->SR |= SPI_SR_RFDF_MASK;
    return data;   
}

//static uint8_t sf_read_status(void)
//{
//    uint8_t status;
//    
//	sf_set_cs(0);									/* 使能片选 */
//	flash_send_byte(CMD_RDSR);						/* 发送命令， 读状态寄存器 */  
//    status = flash_send_byte(0xff);       
//	sf_set_cs(1);									/* 禁能片选 */   
//    return status;   
//    
//}

static void sf_wait_busy(void)
{
    #if 1
    uint8_t status;
    uint32_t retry = 1000;   //全擦耗时60s
    uint32_t SPITimeout;
    
    
    while(retry--)
    {
        sf_set_cs(0);									/* 使能片选 */
        SPITimeout =  50;
        flash_send_byte(CMD_RDSR);						/* 发送命令， 读状态寄存器 */
        do{
            status = flash_send_byte(0xff);            
            spi_delay_ms(2);
        }while( ((status & 0x01) == 1) && SPITimeout-- );  
        
        sf_set_cs(1);	
        if((status & 0x01) != 1)
            break;
    }
    if(!retry)
        while(1);
    #else
        uint8_t status;
    uint32_t SPITimeout = SPI_TIMEOUT;
    
	sf_set_cs(0);									/* 使能片选 */
	flash_send_byte(CMD_RDSR);						/* 发送命令， 读状态寄存器 */
    do{
        status = flash_send_byte(0xff);
        spi_delay_ms(2);
        
    }while( (status & 0x01) == 1);
    
	sf_set_cs(1);									/* 禁能片选 */
	#endif
}

static void sf_write_enable(void)
{  	
    sf_set_cs(0);									/* 使能片选 */
	flash_send_byte(CMD_WREN);								/* 发送命令 */
	sf_set_cs(1);   	
}
static void sf_write_disable(void)
{  	
    sf_set_cs(0);	
	flash_send_byte(CMD_WRDI);								/* 发送命令 */
    sf_set_cs(1);	
}
/*******************************************************************************
*	函 数 名: spi_flash_read
*	功能说明: 连续读取若干字节。字节个数不能超出芯片容量。
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiReadAddr ：首地址
*				_usSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 1 成功
*******************************************************************************/
sf_err spi_flash_read( uint32_t _uiReadAddr, uint8_t * _pBuf,uint32_t _uiSize )
{
	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_flash.block_end * FLASH_BLOCK_SIZE)
	{
		return SF_ERR_ADDR_OUT_BOUND;
	}

	sf_set_cs(0);									
	flash_send_byte(CMD_READ);							
	flash_send_byte((_uiReadAddr & 0xFF0000) >> 16);	
	flash_send_byte((_uiReadAddr & 0xFF00) >> 8);		
	flash_send_byte(_uiReadAddr & 0xFF);				
	while (_uiSize--)
	{
		*_pBuf++ = flash_send_byte(0xFF);			
	}
	sf_set_cs(1);	

    return SF_SUCCESS;	/* 成功 */    
}
/*******************************************************************************
*	函 数 名: sf_page_write
*	功能说明: 向一个page内写入若干字节。字节个数不能超出页面大小（4K)
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiWriteAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 无
*******************************************************************************/
sf_err sf_page_write(uint32_t addr,const uint8_t * _pBuf, uint32_t _usSize)
{
	uint32_t i;
    
    if( (addr&0xFF) != 0 )
        return SF_ERR_ADDR_OUT_BOUND;
    sf_write_enable();		
    /* 发送写使能命令 */
    sf_wait_busy();	
    sf_set_cs(0);									/* 使能片选 */ 
    flash_send_byte(CMD_PP);								/* 发送CP命令(地址自动增加编程) */
    flash_send_byte((addr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
    flash_send_byte((addr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
    flash_send_byte(addr & 0xFF);				/* 发送扇区地址低8bit */ 
    
    for (i = 0; i < FLASH_PAGE_SIZE; i++)
    {
        flash_send_byte(*_pBuf);					/* 发送数据 */
        _pBuf++;
    }  
    sf_set_cs(1);								/* 禁止片选 */
    sf_wait_busy();					/* 等待串行Flash内部写操作完成 */
    sf_write_disable();
    return SF_SUCCESS;
}

/*******************************************************************************
*	函 数 名: sf_erase_sector
*	功能说明: 擦除指定的扇区
*	形    参:  _uiSectorAddr : 扇区地址
*	返 回 值: 无
*******************************************************************************/
sf_err sf_erase_sector(uint32_t offset, uint32_t length)
{
    uint32_t erase_size = 0;
    
    //offset must be ROUND_DOWN to BLOCKSIZE
    if(offset != ROUND_DOWN(offset,FLASH_BLOCK_SIZE))
        return SF_ERR_ADDR_OUT_BOUND;
    if((offset + length) > g_flash.block_end * FLASH_BLOCK_SIZE)
        return SF_ERR_ADDR_OUT_BOUND;
    if(length %  g_flash.block_size != 0)
    {          
        return SF_ERR_ADDR_OUT_BOUND;
    }
    while (erase_size < length)
    {
        sf_write_enable();	    
        sf_set_cs(0);
        flash_send_byte(CMD_ERASE_4K);								/* 发送擦除命令 */
        flash_send_byte((offset & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
        flash_send_byte((offset & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
        flash_send_byte(offset & 0xFF);				/* 发送扇区地址低8bit */   
        sf_set_cs(1);        
        erase_size += 4096;
        offset += 4096;       
        sf_wait_busy();		
    }      
    sf_write_disable();    
    return SF_SUCCESS    ;       
}


sf_err sf_erase_block(uint32_t offset, uint32_t length)
{
    uint32_t erase_size = 0;
    
    //offset must be ROUND_DOWN to BLOCKSIZE
    if(offset != ROUND_DOWN(offset,FLASH_BLOCK_SIZE))
        return SF_ERR_ADDR_OUT_BOUND;
    if((offset + length) > g_flash.block_end * FLASH_BLOCK_SIZE)
        return SF_ERR_ADDR_OUT_BOUND;
    if(length %  g_flash.block_size != 0)
    {          
        return SF_ERR_ADDR_OUT_BOUND;
    }
    while (erase_size < length)
    {
        sf_write_enable();	    
        sf_set_cs(0);
        flash_send_byte(CMD_ERASE_64K);								/* 发送擦除命令 */
        flash_send_byte((offset & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
        flash_send_byte((offset & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
        flash_send_byte(offset & 0xFF);				/* 发送扇区地址低8bit */   
        sf_set_cs(1);        
        erase_size += 0x10000;
        offset += 0x10000;       
        sf_wait_busy();		
    }      
    sf_write_disable();    
    return SF_SUCCESS    ;      
    
}
/*******************************************************************************
*	函 数 名: spi_flash_write
*	功能说明: 对FLASH写入数据，函数自动完成擦除工作
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiWrAddr ：目标区域首地址
*				_usSize ：数据个数，不能超过页面大小
*	返 回 值: 1 : 成功， 0 ： 失败
*******************************************************************************/
sf_err spi_flash_write( uint32_t _uiWriteAddr, const uint8_t* _pBuf,uint32_t _usWriteSize)
{
	uint32_t NumOfPage = 0, NumOfSingle = 0;

	if(_uiWriteAddr % FLASH_PAGE_SIZE != 0)     /* 起始地址是页面首地址  */
        return SF_ERR_ADDR_OUT_BOUND;
	NumOfPage =  _usWriteSize / FLASH_PAGE_SIZE;
    NumOfSingle = _usWriteSize % FLASH_PAGE_SIZE;
    if (NumOfPage == 0) /* 数据长度小于页面大小 */
    {
        if (sf_page_write( _uiWriteAddr,_pBuf, _usWriteSize ) != SF_SUCCESS)
            return SF_ERR_WRITE;
    }
    else 	/* 数据长度大于等于页面大小 */
    {
        while (NumOfPage--)
        {
            if (sf_page_write( _uiWriteAddr, _pBuf, FLASH_PAGE_SIZE) != SF_SUCCESS)
                return SF_ERR_WRITE;
            _uiWriteAddr +=  FLASH_PAGE_SIZE;
            _pBuf += FLASH_PAGE_SIZE;
        }
        if( NumOfSingle != 0)
        {
            if (sf_page_write( _uiWriteAddr, _pBuf, NumOfSingle) != SF_SUCCESS)
                return SF_ERR_WRITE;
        }
        
    }
	
	return SF_SUCCESS;	/* 成功 */
}


/*
********************************************************************************
*	函 数 名: bsp_InitSpiFlash
*	功能说明: 初始化串行Flash硬件接口（配置STM32的SPI时钟、GPIO)
*	形    参:  无
*	返 回 值: 无
********************************************************************************
*/
void spi_flash_init(void)
{
    spi_falsh_hw_init();
    
//    flash_delay();   
	sf_set_cs(0);				/* 软件方式，使能串行Flash片选 */
	sf_wait_busy();					/* 等待串行Flash内部写操作完成 */
    sf_write_disable();
	sf_set_cs(1);				/* 软件方式，禁能串行Flash片选 */

//    sf_erase_chip();
//	sf_write_status(0);			/* 解除所有BLOCK的写保护 */
//    sf_read_info();				/* 自动识别芯片型号 */
}

///*
//********************************************************************************
//*	函 数 名: sf_erase_sector
//*	功能说明: 擦除指定的扇区
//*	形    参:  _uiSectorAddr : 扇区地址
//*	返 回 值: 无
//********************************************************************************
//*/
//sf_err sf_erase_sector(uint32_t offset, uint32_t length)
//{
//    uint32_t erase_size = 0;
//    
//    //offset must be ROUND_DOWN to BLOCKSIZE
//    if(offset != ROUND_DOWN(offset,FLASH_BLOCK_SIZE))
//        return SF_ERR_ADDR_OUT_BOUND;
//    if((offset + length) > g_flash.block_end * FLASH_BLOCK_SIZE)
//        return SF_ERR_ADDR_OUT_BOUND;
//    if(length %  g_flash.block_size != 0)
//    {          
//        return SF_ERR_ADDR_OUT_BOUND;
//    }
//	sf_write_enable();								/* 发送写使能命令 */
//    sf_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */ 
//	/* 擦除扇区操作 */
//	sf_set_cs(0);									/* 使能片选 */
//    while (erase_size < length)
//    {
//        flash_send_byte(CMD_ERASE_4K);								/* 发送擦除命令 */
//        flash_send_byte((offset & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
//        flash_send_byte((offset & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
//        flash_send_byte(offset & 0xFF);				/* 发送扇区地址低8bit */        
//        erase_size += 4096;
//        offset += 4096;       
//    } 
//	sf_set_cs(1);									/* 禁能片选 */ 
//	sf_WaitForWriteEnd();							/* 等待串行Flash内部写操作完成 */ 
//    return SF_SUCCESS    ;
//    
//}

/*
********************************************************************************
*	函 数 名: sf_erase_chip
*	功能说明: 擦除整个芯片
*	形    参:  无
*	返 回 值: 无
********************************************************************************
*/
sf_err sf_erase_chip(void)
{
	sf_write_enable();	
    sf_set_cs(0);				    

	flash_send_byte(CMD_ERASE_full);							/* 发送整片擦除命令 */ 

    sf_set_cs(1);
    sf_wait_busy();		
    sf_write_disable();    
    return SF_SUCCESS    ;
}


///*
//*********************************************************************************************************
//*	函 数 名: spi_flash_read
//*	功能说明: 连续读取若干字节。字节个数不能超出芯片容量。
//*	形    参:  	_pBuf : 数据源缓冲区；
//*				_uiReadAddr ：首地址
//*				_usSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
//*	返 回 值: 1 成功
//*********************************************************************************************************
//*/
//sf_err spi_flash_read( uint32_t _uiReadAddr, uint8_t * _pBuf,uint32_t _uiSize )
//{
//	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
//	if ((_uiSize == 0) ||(_uiReadAddr + _uiSize) > g_flash.block_end * FLASH_BLOCK_SIZE)
//	{
//		return SF_ERR_ADDR_OUT_BOUND;
//	}

//	sf_set_cs(0);									
//	flash_send_byte(CMD_READ);							
//	flash_send_byte((_uiReadAddr & 0xFF0000) >> 16);	
//	flash_send_byte((_uiReadAddr & 0xFF00) >> 8);		
//	flash_send_byte(_uiReadAddr & 0xFF);				
//	while (_uiSize--)
//	{
//		*_pBuf++ = flash_send_byte(0xFF);			
//	}
//	sf_set_cs(1);	

//    return SF_SUCCESS;	/* 成功 */    
//}


/*
*********************************************************************************************************
*	函 数 名: sf_CmpData
*	功能说明: 比较Flash的数据.
*	形    参:  	_ucpTar : 数据缓冲区
*				_uiSrcAddr ：Flash地址
*				_uiSize ：数据个数, 可以大于PAGE_SIZE,但是不能超出芯片总容量
*	返 回 值: 0 = 相等, 1 = 不等
*********************************************************************************************************
*/
uint8_t sf_CmpData(uint32_t _uiSrcAddr, const uint8_t *_ucpTar, uint32_t _uiSize)
{
	uint8_t ucValue;

	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if ((_uiSrcAddr + _uiSize) > g_flash.block_end * FLASH_BLOCK_SIZE)
	{
		return 1;
	}

	if (_uiSize == 0)
	{
		return 0;
	}

	sf_set_cs(0);									/* 使能片选 */
	flash_send_byte(CMD_READ);							/* 发送读命令 */
	flash_send_byte((_uiSrcAddr & 0xFF0000) >> 16);		/* 发送扇区地址的高8bit */
	flash_send_byte((_uiSrcAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	flash_send_byte(_uiSrcAddr & 0xFF);					/* 发送扇区地址低8bit */
	while (_uiSize--)
	{
		/* 读一个字节 */
		ucValue = flash_send_byte(0xff);
		if (*_ucpTar++ != ucValue)
		{
			sf_set_cs(1);
			return 1;
		}
	}
	sf_set_cs(1);
    
	return 0;
}

///*
//*********************************************************************************************************
//*	函 数 名: sf_need_erase
//*	功能说明: 判断写PAGE前是否需要先擦除。
//*	形    参:   _ucpOldBuf ： 旧数据
//*			   _ucpNewBuf ： 新数据
//*			   _uiLen ：数据个数，不能超过页面大小
//*	返 回 值: 0 : 不需要擦除， 1 ：需要擦除
//*********************************************************************************************************
//*/
////static uint8_t sf_need_erase(uint8_t * _ucpOldBuf, uint8_t *_ucpNewBuf, uint16_t _usLen)
////{
////	uint16_t i;
////	uint8_t ucOld;

////	/*
////	算法第1步：old 求反, new 不变
////	      old    new
////		  1101   0101
////	~     1111
////		= 0010   0101

////	算法第2步: old 求反的结果与 new 位与
////		  0010   old
////	&	  0101   new
////		 =0000

////	算法第3步: 结果为0,则表示无需擦除. 否则表示需要擦除
////	*/

////	for (i = 0; i < _usLen; i++)
////	{
////		ucOld = *_ucpOldBuf++;
////		ucOld = ~ucOld;

////		/* 注意错误的写法: if (ucOld & (*_ucpNewBuf++) != 0) */
////		if ((ucOld & (*_ucpNewBuf++)) != 0)
////		{
////			return 1;
////		}
////	}
////	return 0;
////}

/////*
////*********************************************************************************************************
////*	函 数 名: sf_AutoWritePage
////*	功能说明: 写1个扇区并校验,如果不正确则再重写两次。本函数自动完成擦除操作。
////*	形    参:  	_pBuf : 数据源缓冲区；
////*				_uiWriteAddr ：目标区域首地址
////*				_usSize ：数据个数，不能超过页面大小
////*	返 回 值: 0 : 错误， 1 ： 成功
////*********************************************************************************************************
////*/
////static uint8_t sf_AutoWritePage(uint32_t _uiWrAddr, uint16_t _usWrLen, uint8_t *_ucpSrc)
////{
////	uint16_t i;
////	uint16_t j;					/* 用于延时 */
////	uint32_t uiFirstAddr;		/* 扇区首址 */
////	uint8_t ucNeedErase;		/* 1表示需要擦除 */
////	uint8_t cRet;

////	/* 长度为0时不继续操作,直接认为成功 */
////	if (_usWrLen == 0)
////	{
////		return 1;
////	}

////	/* 如果偏移地址超过芯片容量则退出 */
////	if (_uiWrAddr >= g_flash.total_size)
////	{
////		return 0;
////	}

////	/* 如果数据长度大于扇区容量，则退出 */
////	if (_usWrLen > g_flash.sector_size)
////	{
////		return 0;
////	}

////	/* 如果FLASH中的数据没有变化,则不写FLASH */
////	spi_flash_read( _uiWrAddr, _usWrLen ,s_spiBuf);
////	if (memcmp(s_spiBuf, _ucpSrc, _usWrLen) == 0)
////	{
////		return 1;
////	}

////	/* 判断是否需要先擦除扇区 */
////	/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
////	ucNeedErase = 0;
////	if (sf_need_erase(s_spiBuf, _ucpSrc, _usWrLen))
////	{
////		ucNeedErase = 1;
////	}

////	uiFirstAddr = _uiWrAddr & (~(g_flash.sector_size - 1));

////	if (_usWrLen == g_flash.sector_size)		/* 整个扇区都改写 */
////	{
////		for	(i = 0; i < g_flash.sector_size; i++)
////		{
////			s_spiBuf[i] = _ucpSrc[i];
////		}
////	}
////	else						/* 改写部分数据 */
////	{
////		/* 先将整个扇区的数据读出 */
////		spi_flash_read(uiFirstAddr, g_flash.sector_size, s_spiBuf);

////		/* 再用新数据覆盖 */
////		i = _uiWrAddr & (g_flash.sector_size - 1);
////		memcpy(&s_spiBuf[i], _ucpSrc, _usWrLen);
////	}

////	/* 写完之后进行校验，如果不正确则重写，最多3次 */
////	cRet = 0;
////	for (i = 0; i < 3; i++)
////	{

////		/* 如果旧数据修改为新数据，所有位均是 1->0 或者 0->0, 则无需擦除,提高Flash寿命 */
////		if (ucNeedErase == 1)
////		{
////			sf_erase_sector(uiFirstAddr);		/* 擦除1个扇区 */
////		}

////		/* 编程一个SECTOR */
////		sf_page_write(s_spiBuf, uiFirstAddr, g_flash.sector_size);

////		if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
////		{
////			cRet = 1;
////			break;
////		}
////		else
////		{
////			if (sf_CmpData(_uiWrAddr, _ucpSrc, _usWrLen) == 0)
////			{
////				cRet = 1;
////				break;
////			}

////			/* 失败后延迟一段时间再重试 */
////			for (j = 0; j < 10000; j++);
////		}
////	}

////	return cRet;
////}

///*
//*********************************************************************************************************
//*	函 数 名: spi_flash_write
//*	功能说明: 对FLASH写入数据，函数自动完成擦除工作
//*	形    参:  	_pBuf : 数据源缓冲区；
//*				_uiWrAddr ：目标区域首地址
//*				_usSize ：数据个数，不能超过页面大小
//*	返 回 值: 1 : 成功， 0 ： 失败
//*********************************************************************************************************
//*/
//sf_err spi_flash_write( uint32_t _uiWriteAddr, const uint8_t* _pBuf,uint32_t _usWriteSize)
//{
//	uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

//	Addr = _uiWriteAddr % FLASH_PAGE_SIZE;
//	count = FLASH_PAGE_SIZE - Addr;
//	NumOfPage =  _usWriteSize / FLASH_PAGE_SIZE;
//	NumOfSingle = _usWriteSize % FLASH_PAGE_SIZE;

//	if (Addr == 0) /* 起始地址是页面首地址  */
//	{
//		if (NumOfPage == 0) /* 数据长度小于页面大小 */
//		{
//			if (sf_page_write( _uiWriteAddr,_pBuf, _usWriteSize ) != SF_SUCCESS)
//			{
//				return SF_ERR_WRITE;
//			}
//		}
//		else 	/* 数据长度大于等于页面大小 */
//		{
//			while (NumOfPage--)
//			{
//				if (sf_page_write( _uiWriteAddr, _pBuf, FLASH_PAGE_SIZE) != SF_SUCCESS)
//				{
//					return SF_ERR_WRITE;
//				}
//				_uiWriteAddr +=  FLASH_PAGE_SIZE;
//				_pBuf += FLASH_PAGE_SIZE;
//			}
//			if (sf_page_write( _uiWriteAddr, _pBuf, NumOfSingle) != SF_SUCCESS)
//			{
//				return SF_ERR_WRITE;
//			}
//		}
//	}
//	else  /* 起始地址不是页面首地址  */
//	{
//		if (NumOfPage == 0) /* 数据长度小于页面大小 */
//		{
//			if (NumOfSingle > count) /* (_usWriteSize + _uiWriteAddr) > SPI_FLASH_sector_size */
//			{
//				temp = NumOfSingle - count;

//				if (sf_page_write( _uiWriteAddr,  _pBuf,count) != SF_SUCCESS)
//				{
//					return SF_ERR_WRITE;
//				}

//				_uiWriteAddr +=  count;
//				_pBuf += count;

//				if (sf_page_write( _uiWriteAddr, _pBuf,temp) != SF_SUCCESS)
//				{
//					return SF_ERR_WRITE;
//				}
//			}
//			else
//			{
//				if (sf_page_write( _uiWriteAddr,_pBuf, _usWriteSize ) != SF_SUCCESS)
//				{
//					return SF_ERR_WRITE;
//				}
//			}
//		}
//		else	/* 数据长度大于等于页面大小 */
//		{
//			_usWriteSize -= count;
//			NumOfPage =  _usWriteSize / FLASH_PAGE_SIZE;
//			NumOfSingle = _usWriteSize % FLASH_PAGE_SIZE;

//			if (sf_page_write( _uiWriteAddr, _pBuf,count) != SF_SUCCESS)
//			{
//				return SF_ERR_WRITE;
//			}

//			_uiWriteAddr +=  count;
//			_pBuf += count;

//			while (NumOfPage--)
//			{
//				if (sf_page_write( _uiWriteAddr, _pBuf, FLASH_PAGE_SIZE) != SF_SUCCESS)
//				{
//					return SF_ERR_WRITE;
//				}
//				_uiWriteAddr +=  FLASH_PAGE_SIZE;
//				_pBuf += FLASH_PAGE_SIZE;
//			}

//			if (NumOfSingle != 0)
//			{
//				if (sf_page_write( _uiWriteAddr,_pBuf, NumOfSingle) != SF_SUCCESS)
//				{
//					return SF_ERR_WRITE;
//				}
//			}
//		}
//	}
//	return SF_SUCCESS;	/* 成功 */
//}

/*
********************************************************************************
*	函 数 名: sf_read_id
*	功能说明: 读取器件ID
*	形    参:  无
*	返 回 值: 32bit的器件ID (最高8bit填0，有效ID位数为24bit 25Q32BV_ID    = 0xEF4016
********************************************************************************
*/
static uint32_t sf_read_id(void)
{
	uint32_t uiID;
	uint8_t id1, id2, id3;

	sf_set_cs(0);									/* ???? */
	flash_send_byte(CMD_JEDEC_ID);								/* ???ID?? */
	id1 = flash_send_byte(0xFF);					/* ?ID??1??? */
	id2 = flash_send_byte(0xFF);					/* ?ID??2??? */
	id3 = flash_send_byte(0xFF);					/* ?ID??3??? */
	sf_set_cs(1);									/* ???? */

	uiID = ((uint32_t)id1 << 16) | ((uint32_t)id2 << 8) | id3;

	return uiID;
}

/*
********************************************************************************
*	函 数 名: sf_ReadInfo
*	功能说明: 读取器件ID,并填充器件参数
*	形    参:  无
*	返 回 值: 无
********************************************************************************
*/
int sf_read_info(void)
{
    uint32_t chip_id;
	/* 自动识别串行Flash型号 */
    chip_id = sf_read_id();	        /* 芯片ID */
//    g_flash.total_size = 4 * 1024 * 1024;	/* 总容量 = 4M */
//    g_flash.sector_size = 4 * 1024;			/* 扇区大小 = 4K */
//    g_flash.page_size = 256;			/* 页大小 = 256*/

    g_flash.chip_id =  chip_id;
    g_flash.block_start = 0;
    g_flash.block_size = FLASH_BLOCK_SIZE;
    g_flash.block_end = 2048;
    g_flash.capacity = g_flash.block_size * g_flash.block_end ;
    if( chip_id == MX25L64_ID)
        return 0;
    return -1;
}


//int spiflash_test(uint32_t begin, uint32_t end)
//{
//    uint32_t i, total_sector, j;
//    static uint8_t buf[FLASH_BLOCK_SIZE];
//    
////    printf("spi flash id:0x%X\r\n", w25qxx_get_id());
//    
//    total_sector = (end - begin)/FLASH_BLOCK_SIZE;
//    
//    /* erase chip */
////    printf("erase all chips...\r\n");
//    sf_erase_chip();
////    printf("erase complete\r\n");
//    spi_flash_read(0, buf, FLASH_BLOCK_SIZE);
//    for(i=begin/FLASH_BLOCK_SIZE; i<total_sector; i++)
//    {
////        printf("verify addr:0x%X(%d)...\r\n", i*FLASH_BLOCK_SIZE, i);
//        for(j=0;j<sizeof(buf);j++)
//        {
//            buf[j] = j % 0xFF;
//        }
//        spi_flash_write(i*FLASH_BLOCK_SIZE, buf, FLASH_BLOCK_SIZE);
//        memset(buf, 0, FLASH_BLOCK_SIZE);
//        spi_flash_read(i*FLASH_BLOCK_SIZE, buf, FLASH_BLOCK_SIZE);
//        
//        /* varify */
//        for(j=0;j<sizeof(buf);j++)
//        {
//            if(buf[j] != (j%0xFF))
//            {
////                printf("%d error\r\n", j);
//                while(1);
//            }
//        }
//    }
//    return 0;
//}


