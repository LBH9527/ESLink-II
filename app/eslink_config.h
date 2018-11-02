#ifndef __ESLINK_CONFIG_H__
#define __ESLINK_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif    
    
#include "MK22f12810.h"
#include "eslink_gpio.h"
// Configure ISP I/O pins ------------------------------

/** Setup ISP pins: ISPSCK、ISPSDA and RST.
Configures the DAP Hardware I/O pins for Serial Wire Debug (SWD) mode:
 - SWCLK, SWDIO, nRESET to output mode and set to default high level.
 - TDI, TMS, nTRST to HighZ mode (pins are unused in SWD mode).
*/
static inline void PORT_ISP_SETUP(void)
{
    PIN_ISPCLK_GPIO->PSOR     = 1 << PIN_ISPCLK_BIT;
    PIN_ISPSDA_OUT_GPIO->PSOR = 1 << PIN_ISPSDA_OUT_BIT;    
    PIN_ISPSDA_NOE_GPIO->PCOR = 1 << PIN_ISPSDA_NOE_BIT;    
    PIN_ISPSCK_NOE_GPIO->PCOR = 1 << PIN_ISPSCK_NOE_BIT;    
    PIN_RST_GPIO->PSOR        = 1 << PIN_RST_BIT;    
    PIN_ISPSCK_NOE_GPIO->PDDR = PIN_ISPSCK_NOE_GPIO->PDDR | (1 << PIN_ISPSCK_NOE_BIT );   
    PIN_ISPSCK_NOE_GPIO->PCOR = 1 << PIN_ISPSCK_NOE_BIT;    
    PIN_ISPSDA_NOE_GPIO->PCOR = 1 << PIN_ISPSDA_NOE_BIT;    
    PIN_RST_GPIO->PSOR = PIN_RST;
    PIN_RST_GPIO->PDDR |= PIN_RST; //output
    PIN_RST_PORT->PCR[PIN_RST_BIT] = PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK | PORT_PCR_MUX(1);
}

/** Disable JTAG/SWD I/O Pins.
Disables the DAP Hardware I/O pins which configures:
 - TCK/SWCLK, TMS/SWDIO, TDI, TDO, nTRST, nRESET to High-Z mode.
*/
static inline void PORT_ISP_OFF(void)
{
    PIN_ISPSDA_NOE_GPIO->PSOR = 1 << PIN_ISPSDA_NOE_BIT;
    PIN_ISPSCK_NOE_GPIO->PSOR = 1 << PIN_ISPSCK_NOE_BIT;
    PIN_RST_GPIO->PSOR    = 1 << PIN_RST_BIT;
    PIN_RST_GPIO->PDDR &= ~PIN_RST; //input    
    PIN_RST_PORT->PCR[PIN_RST_BIT] |= PORT_PCR_ISF_MASK;
    PIN_RST_PORT->PCR[PIN_RST_BIT] = PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_PFE_MASK | PORT_PCR_MUX(1);
}


// ISPCLK I/O pin -------------------------------------

/** ISPCLK I/O pin: Set Output to High.
Set the SWCLK/TCK DAP hardware I/O pin to high level.
*/
static __forceinline void     PIN_ISPCLK_SET(void)
{
    PIN_ISPCLK_GPIO->PSOR = 1 << PIN_ISPCLK_BIT;
}

/** ISPCLK pin: Set Output to Low.
Set the ISPCLK hardware I/O pin to low level.
*/
static __forceinline void     PIN_ISPCLK_CLR(void)
{
    PIN_ISPCLK_GPIO->PCOR = 1 << PIN_ISPCLK_BIT;
}


// ISPSDA Pin I/O --------------------------------------

/** ISPSDA I/O pin: Get Input.
\return Current status of the ISP hardware I/O pin.
*/
static __forceinline uint32_t PIN_ISPSDA_IN(void)
{
    return ((PIN_ISPSDA_IN_GPIO->PDIR >> PIN_ISPSDA_IN_BIT) & 1);
}

/** ISPSDA I/O pin: Set Output to High.
Set the ISPSDA hardware I/O pin to high level.
*/
static __forceinline void     PIN_ISPSDA_SET(void)
{
    PIN_ISPSDA_OUT_GPIO->PSOR = 1 << PIN_ISPSDA_OUT_BIT;
}

/** ISPSDA I/O pin: Set Output to Low.
Set the ISPSDA hardware I/O pin to low level.
*/
static __forceinline void     PIN_ISPSDA_CLR(void)
{
    PIN_ISPSDA_OUT_GPIO->PCOR = 1 << PIN_ISPSDA_OUT_BIT;
}

/** ISPSDA I/O pin: Get Input (used in SWD mode only).
\return Current status of the SWDIO DAP hardware I/O pin.
*/
//static __forceinline uint32_t PIN_ISPSDA_IN(void)
//{
//    return (BITBAND_REG(PIN_ISPSDA_IN_GPIO->PDIR, PIN_ISPSDA_IN_BIT));
//}

/** ISPSDA I/O pin: Set Output (used in SWD mode only).
\param bit Output value for the SWDIO DAP hardware I/O pin.
*/
static __forceinline void     PIN_ISPSDA_OUT(uint32_t bit)
{
    BITBAND_REG(PIN_ISPSDA_OUT_GPIO->PDOR, PIN_ISPSDA_OUT_BIT) = bit;
}

/** ISPSDA I/O pin: Switch to Output mode (used in SWD mode only).
Configure the ISPSDA hardware I/O pin to output mode. This function is
called prior \ref PIN_SWDIO_OUT function calls.
*/
static __forceinline void     PIN_ISPSDA_OUT_ENABLE(void)
{
    PIN_ISPSDA_NOE_GPIO->PCOR = 1 << PIN_ISPSDA_NOE_BIT;
}

/** ISPSDA I/O pin: Switch to Input mode (used in SWD mode only).
Configure the SWDIO DAP hardware I/O pin to input mode. This function is
called prior \ref PIN_SWDIO_IN function calls.
*/
static __forceinline void     PIN_ISPSDA_OUT_DISABLE(void)
{
    PIN_ISPSDA_NOE_GPIO->PSOR = 1 << PIN_ISPSDA_NOE_BIT;
}

// RST Pin I/O------------------------------------------

/** RST I/O pin: Get Input.
\return Current status of the RST hardware I/O pin.
*/
static __forceinline uint32_t PIN_RST_IN(void)
{
    return ((PIN_RST_GPIO->PDIR >> PIN_RST_BIT) & 1);
}

/** nRESET I/O pin: Set Output.
\param bit target device hardware reset pin status:
           - 0: issue a device hardware reset.
           - 1: release device hardware reset.
*/
static __forceinline void     PIN_RST_OUT(uint32_t bit)
{
    BITBAND_REG(PIN_RST_GPIO->PDOR, PIN_RST_BIT) = bit;
}

//CMSIS-DAP Hardware I/O and LED Pins are initialized with the function \ref DAP_SETUP.


/** Setup of the Debug Unit I/O pins and LEDs (called when Debug Unit is initialized).
This function performs the initialization of the CMSIS-DAP Hardware I/O Pins and the
Status LEDs. In detail the operation of Hardware I/O and LED pins are enabled and set:
 - I/O clock system enabled.
 - all I/O pins: input buffer enabled, output pins are set to HighZ mode.
 - for nTRST, nRESET a weak pull-up (if available) is enabled.
 - LED output pins are enabled and LEDs are turned off.
*/
static inline void ISP_SETUP(void)
{
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK |  /* Enable Port A Clock */
                  SIM_SCGC5_PORTB_MASK |  /* Enable Port B Clock */
                  SIM_SCGC5_PORTC_MASK |  /* Enable Port C Clock */
                  SIM_SCGC5_PORTD_MASK;   /* Enable Port D Clock */
    /* Configure I/O pin ISPCLK */
    PIN_ISPCLK_PORT->PCR[PIN_ISPCLK_BIT]         = PORT_PCR_MUX(1) |   /* GPIO */
             PORT_PCR_DSE_MASK; /* High drive strength */
    PIN_ISPCLK_GPIO->PSOR  = 1 << PIN_ISPCLK_BIT;                      /* High level */
    PIN_ISPCLK_GPIO->PDDR |= 1 << PIN_ISPCLK_BIT;                      /* Output */
    /* Configure I/O pin ISPSDA_OUT */
    PIN_ISPSDA_OUT_PORT->PCR[PIN_ISPSDA_OUT_BIT] = PORT_PCR_MUX(1) |   /* GPIO */
             PORT_PCR_DSE_MASK; /* High drive strength */
    PIN_ISPSDA_OUT_GPIO->PSOR  = 1 << PIN_ISPSDA_OUT_BIT;              /* High level */
    PIN_ISPSDA_OUT_GPIO->PDDR |= 1 << PIN_ISPSDA_OUT_BIT;              /* Output */
    /* Configure I/O pin ISPSDA In */
    PIN_ISPSDA_IN_PORT->PCR[PIN_ISPSDA_IN_BIT]   = PORT_PCR_MUX(1)  |  /* GPIO */
            PORT_PCR_PE_MASK |  /* Pull enable */
            PORT_PCR_PS_MASK;   /* Pull-up */
    PIN_ISPSDA_IN_GPIO->PDDR &= ~(1 << PIN_ISPSDA_IN_BIT);             /* Input */
    /* Configure I/O pin SWDIO_NOE */
    PIN_ISPSDA_NOE_PORT->PCR[PIN_ISPSDA_NOE_BIT] = PORT_PCR_MUX(1) |   /* GPIO */
             PORT_PCR_DSE_MASK; /* High drive strength */
    PIN_ISPSDA_NOE_GPIO->PSOR  = 1 << PIN_SWDIO_NOE_BIT;              /* High level */
    PIN_ISPSDA_NOE_GPIO->PDDR |= 1 << PIN_SWDIO_NOE_BIT;              /* Output */
    /* Configure I/O pin SWD_NOE */
    PIN_ISPSCK_NOE_PORT->PCR[PIN_SWD_NOE_BIT]     = PORT_PCR_MUX(1) |   /* GPIO */
             PORT_PCR_DSE_MASK; /* High drive strength */
    PIN_ISPSCK_NOE_GPIO->PSOR  = 1 << PIN_ISPSCK_NOE_BIT;                  /* High level */
    PIN_ISPSCK_NOE_GPIO->PDDR |= 1 << PIN_ISPSCK_NOE_BIT;                  /* Output */
    /* Configure I/O pin nRESET */
    PIN_RST_PORT->PCR[PIN_RST_BIT]       = PORT_PCR_MUX(1)  |  /* GPIO */
            PORT_PCR_PE_MASK |  /* Pull enable */
            PORT_PCR_PS_MASK |  /* Pull-up */
            PORT_PCR_ODE_MASK;  /* Open-drain */
    PIN_RST_GPIO->PSOR  = 1 << PIN_RST_BIT;                    /* High level */
    PIN_RST_GPIO->PDDR &= ~(1 << PIN_RST_BIT);                    /* Input */
//    /* Configure LED */
//    LED_CONNECTED_PORT->PCR[LED_CONNECTED_BIT] = PORT_PCR_MUX(1)  |  /* GPIO */
//            PORT_PCR_ODE_MASK;  /* Open-drain */
//    LED_CONNECTED_GPIO->PCOR  = 1 << LED_CONNECTED_BIT;              /* Turned on */
//    LED_CONNECTED_GPIO->PDDR |= 1 << LED_CONNECTED_BIT;              /* Output */
}

#ifdef __cplusplus
}
#endif


#endif
