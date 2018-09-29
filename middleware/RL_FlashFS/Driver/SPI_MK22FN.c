/*----------------------------------------------------------------------------
 *      RL-ARM - FlashFS
 *----------------------------------------------------------------------------
 *      Name:    SPI_STM32F103.c
 *      Purpose: Serial Peripheral Interface Driver for ST STM32F103
 *      Rev.:    V4.22
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2011 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <File_Config.h>
#include "spi_flash.h"

/*----------------------------------------------------------------------------
  SPI Driver instance definition
   spi0_drv: First SPI driver
   spi1_drv: Second SPI driver
 *---------------------------------------------------------------------------*/

#define __DRV_ID  spi0_drv
#define __FPCLK   FLASH_DSPI_CLK_FREQ

/* SPI Driver Interface functions */
static BOOL Init (void);
static BOOL UnInit (void);
static U8   Send (U8 outb);
static BOOL SendBuf (U8 *buf, U32 sz);
static BOOL RecBuf (U8 *buf, U32 sz);
static BOOL BusSpeed (U32 kbaud);
static BOOL SetSS (U32 ss);
//static U32  CheckMedia (void);        /* Optional function for SD card check */

/* SPI Device Driver Control Block */
SPI_DRV __DRV_ID = {
  Init,
  UnInit,
  Send,
  SendBuf,
  RecBuf,
  BusSpeed,
  SetSS,
  NULL,                          /* Can be NULL if not existing         */
};

/* SPI_SR - bit definitions. */
#define TCF     (0x80000000U)               /*!< Transfer Complete Flag. */
#define TXRXS   (0x40000000U)               /*!< The module is in Stopped/Running state.*/
#define TFUF    (0x8000000U)                /*!< Transmit FIFO Underflow Flag.*/
#define TFFF    (0x2000000U)                /*!< Transmit FIFO Fill Flag.*/
#define RFOF    (0x80000U)                  /*!< Receive FIFO Overflow Flag.*/
#define RFDF    (0x20000U)                  /*!< Receive FIFO Drain Flag.*/  

/*--------------------------- Init ------------------------------------------*/
static BOOL Init (void) {
    /* Initialize and enable the SSP Interface module. */
    SF_CS_CLR();	            //使能片选


    return (__TRUE);
}


/*--------------------------- UnInit ----------------------------------------*/

static BOOL UnInit (void) {
    /* Return SSP interface to default state. */ 
    //由于有两个地方同时使用spi flash，不把spi flash设为default状态，仅仅禁止片选
    SF_CS_SET();							/* 禁能片选 */
    return (__TRUE);
}


/*--------------------------- Send ------------------------------------------*/    
static U8 Send (U8 outb) {
    /* Write and Read a byte on SPI interface. */

    uint8_t data;
    
    SPI1->PUSHR =  SPI_PUSHR_TXDATA(outb);

//    while( !( SPI1->SR & RFDF) )  ;
//    DSPI_ClearStatusFlags(SPI1, kDSPI_RxFifoDrainRequestFlag);
    while( !( SPI1->SR & TCF) )  ;
    SPI1->SR |= TCF;    //Clears the DSPI status flag. 
    while( !( SPI1->SR & RFDF) )  ;
    data =  (uint8_t)SPI1->POPR;
    SPI1->SR |= RFDF;
    
    return data;
}


/*--------------------------- SendBuf ---------------------------------------*/

static BOOL SendBuf (U8 *buf, U32 sz) {
  /* Send buffer to SPI interface. */
    U32 i;

    for (i = 0; i < sz; i++) 
    {
        SPI1->PUSHR =  SPI_PUSHR_TXDATA(buf[i]);
        /* Wait if Tx FIFO is full. */
//        while( !( SPI1->SR & TFFF) )  ;
//        DSPI_ClearStatusFlags(SPI1, kDSPI_TxFifoFillRequestFlag);
        while( !( SPI1->SR & TCF) )  ;
        SPI1->SR |= TCF;    //Clears the DSPI status flag. 
        SPI1->POPR;
    }
    /* Wait until Tx finished, drain Rx FIFO. */
    while (SPI1->SR & RFDF)
    {          
        SPI1->POPR;
        SPI1->SR |=  RFDF;
    }     
    
    return (__TRUE);
}


/*--------------------------- RecBuf ----------------------------------------*/

static BOOL RecBuf (U8 *buf, U32 sz) {
  /* Receive SPI data to buffer. */
  U32 i;

  for (i = 0; i < sz; i++) {
       SPI1->PUSHR =  0xFF;
        /* Wait while Rx FIFO is empty. */
        while( !( SPI1->SR & RFDF) )  ;
       
        buf[i] = (uint8_t)SPI1->POPR;
        SPI1->SR |= RFDF ;
  }
  return (__TRUE);
}


/*--------------------------- BusSpeed --------------------------------------*/

static BOOL BusSpeed (U32 kbaud) {
  /* Set an SPI clock to required baud rate. */
    DSPI_MasterSetBaudRate(SPI1,kDSPI_Ctar0,kbaud,__FPCLK);
    return (__TRUE);
}       
/*--------------------------- SetSS -----------------------------------------*/
/* Enable/Disable SPI Chip Select (drive it high or low). */    
static BOOL SetSS (U32 ss) {
  /* Enable/Disable SPI Chip Select (drive it high or low). */
  if(ss)      /* SSEL is GPIO, output set to high. */  
  {
      FLASH_DSPI_INT_GPIO->PSOR = 1U << FLASH_DSPI_INT_PIN;
  }
  else        /* SSEL is GPIO, output set to low. */  
  {
      FLASH_DSPI_INT_GPIO->PCOR = 1U << FLASH_DSPI_INT_PIN;
  }
  return (__TRUE);
}




/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
