/*
 * The Clear BSD License
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_gpio.h"
#include "fsl_dspi.h"
//#include "fsl_i2c.h"
#include "pin_mux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* The board name */

/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE DEBUG_CONSOLE_DEVICE_TYPE_UART
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) UART0
#define BOARD_DEBUG_UART_INSTANCE 0U
#define BOARD_DEBUG_UART_CLKSRC SYS_CLK
#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetCoreSysClkFreq()
#define BOARD_UART_IRQ UART0_RX_TX_IRQn
#define BOARD_UART_IRQ_HANDLER UART0_RX_TX_IRQHandler

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE 115200
#endif /* BOARD_DEBUG_UART_BAUDRATE */


/* Board beep */
#define BEEP_INIT(output)                                                 \
    GPIO_PinWrite(BOARD_BEEP_GPIO, BOARD_BEEP_GPIO_PIN, output); \
    BOARD_BEEP_GPIO->PDDR |= (1U << BOARD_BEEP_GPIO_PIN) /*!< Enable target LED_RED */
#define BEEP_ON() \
    GPIO_PortClear(BOARD_BEEP_GPIO, 1U << BOARD_BEEP_GPIO_PIN) /*!< Turn on target LED_RED */
#define BEEP_OFF() \
    GPIO_PortSet(BOARD_BEEP_GPIO, 1U << BOARD_BEEP_GPIO_PIN) /*!< Turn off target LED_RED */


 
/*******************************************************************************
*   ES_LINK ¹¦ÄÜIO
*******************************************************************************/
/* Control Pin */
#define CLT_INIT(output)                                                 \
    GPIO_PinWrite(PIN_CTL_GPIO, PIN_CTL_BIT, output); \
    PIN_CTL_GPIO->PDDR |= (1U << PIN_CTL_BIT) /*!< Enable target LED_RED */
#define CTL_SET() \
    GPIO_PortClear(PIN_CTL_GPIO, 1U << PIN_CTL_BIT) /*!< Turn on target LED_RED */
#define CTL_CLR() \
    GPIO_PortSet(PIN_CTL_GPIO, 1U << PIN_CTL_BIT) /*!< Turn off target LED_RED */
#define CTL_TOGGLE() \
    GPIO_PortToggle(PIN_CTL_GPIO, 1U << PIN_CTL_BIT) /*!< Toggle on target LED_BLUE */
   
/* SDHC base address, clock and card detection pin */
#define BOARD_SDHC_BASEADDR SDHC
#define BOARD_SDHC_CLKSRC kCLOCK_CoreSysClk
#define BOARD_SDHC_CLK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)
#define BOARD_SDHC_IRQ SDHC_IRQn
#define BOARD_SDHC_CD_GPIO_BASE GPIOE
#ifndef BOARD_SDHC_CD_GPIO_PIN
#define BOARD_SDHC_CD_GPIO_PIN 6U
#endif
#define BOARD_SDHC_CD_PORT_BASE PORTE
#define BOARD_SDHC_CD_PORT_IRQ PORTE_IRQn
#define BOARD_SDHC_CD_PORT_IRQ_HANDLER PORTE_IRQHandler
#define BOARD_SDHC_CD_LOGIC_RISING

#define BOARD_ACCEL_I2C_BASEADDR I2C0

/* FLASH DSPI configuration */
#define FLASH_DSPI_MASTER_BASEADDR SPI1
#define FLASH_DSPI_BAUDRATE 10000000U           //10M
#define FLASH_DSPI_CLKSRC DSPI1_CLK_SRC
#define FLASH_DSPI_CLK_FREQ CLOCK_GetFreq(DSPI1_CLK_SRC)

#define FLASH_DSPI_MASTER_PCS_FOR_INIT kDSPI_Pcs0
#define FLASH_DSPI_MASTER_PCS_FOR_TRANSFER kDSPI_MasterPcs0
#define FLASH_DSPI_INT_GPIO GPIOD
#define FLASH_DSPI_INT_PORT PORTD
#define FLASH_DSPI_INT_PIN 4U
//#define FLASH_DSPI_INT_PIN_IRQ PORTB_IRQn
//#define FLASH_DSPI_INT_PIN_IRQ_HANDLER PORTB_IRQHandler

/* DAC base address */
#define BOARD_DAC_BASEADDR DAC0

/* Board accelerometer driver */
#define BOARD_ACCEL_FXOS

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
void BOARD_InitDebugConsole(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
