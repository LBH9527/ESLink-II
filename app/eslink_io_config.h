#ifndef __ESLINK_IO_CONFIG_H__
#define __ESLINK_IO_CONFIG_H__

#include "pin_mux.h"

//KEY Port I/O Pins
/******************************************************************************/
// HID_LED PTA5
#define PIN_HID_LED_PORT        PORTA
#define PIN_HID_LED_GPIO        PTA
#define PIN_HID_LED_BIT         (5)
#define PIN_HID_LED             (1<<PIN_HID_LED_BIT)

// MSC_LED PTB3
#define PIN_MSC_LED_PORT        PORTB
#define PIN_MSC_LED_GPIO        PTB
#define PIN_MSC_LED_BIT         (3)
#define PIN_MSC_LED             (1<<PIN_MSC_LED_BIT)

// CDC_LED PTB2
#define PIN_CDC_LED_PORT        PORTB
#define PIN_CDC_LED_GPIO        PTB
#define PIN_CDC_LED_BIT         (2)
#define PIN_CDC_LED             (1<<PIN_CDC_LED_BIT)

//KEY Port I/O Pins
/******************************************************************************/
// KEY0 PTA1
#define PIN_KEY0_PORT       PORTA
#define PIN_KEY0_GPIO       PTA
#define PIN_KEY0_BIT        (1)
#define PIN_KEY0            (1<<PIN_KEY0_BIT)


//OLED Port I/O Pins 
/******************************************************************************/

// OLED_CS  PTB19
#define PIN_OLED_CS_PORT        PORTB
#define PIN_OLED_CS_GPIO        PTB
#define PIN_OLED_CS_BIT         (19)
#define PIN_OLED_CS             (1<<PIN_OLED_CS_BIT)
// OLED_RST  PTB18
#define PIN_OLED_RST_PORT       PORTB
#define PIN_OLED_RST_GPIO       PTB
#define PIN_OLED_RST_BIT        (18)
#define PIN_OLED_RST            (1<<PIN_OLED_RST_BIT)
// OLED_SCK  PTC2
#define PIN_OLED_SCK_PORT       PORTC
#define PIN_OLED_SCK_GPIO       PTC
#define PIN_OLED_SCK_BIT        (2)
#define PIN_OLED_SCK            (1<<PIN_OLED_SCK_BIT)
// OLED_SDA  PTC3
#define PIN_OLED_SDA_PORT       PORTC
#define PIN_OLED_SDA_GPIO       PTC
#define PIN_OLED_SDA_BIT        (3)
#define PIN_OLED_SDA            (1<<PIN_OLED_SDA_BIT)  

// Debug Port I/O Pins
/******************************************************************************/
/*
 * DAPLink IO define
 */
// SWCLK Pin                    PTC5
#define PIN_SWCLK_PORT          PORTC
#define PIN_SWCLK_GPIO          PTC
#define PIN_SWCLK_BIT           5

// SWDIO Out Pin                PTC6
#define PIN_SWDIO_OUT_PORT      PORTC
#define PIN_SWDIO_OUT_GPIO      PTC
#define PIN_SWDIO_OUT_BIT       6

// SWDIO In Pin                 PTC7
#define PIN_SWDIO_IN_PORT       PORTC
#define PIN_SWDIO_IN_GPIO       PTC
#define PIN_SWDIO_IN_BIT        7

// SWDIO Neg. Output Enable Pin PTC9
#define PIN_SWDIO_NOE_PORT      PORTC
#define PIN_SWDIO_NOE_GPIO      PTC
#define PIN_SWDIO_NOE_BIT       9

// SWD Neg. Enable Pin          PTC8
#define PIN_SWD_NOE_PORT        PORTC
#define PIN_SWD_NOE_GPIO        PTC
#define PIN_SWD_NOE_BIT         8

// nRESET Pin                   PTC4
#define PIN_nRESET_PORT         PORTC
#define PIN_nRESET_GPIO         PTC
#define PIN_nRESET_BIT          4
#define PIN_nRESET              (1 << PIN_nRESET_BIT)

// Power and fault detection

// PWR_REG_EN PTD2              PTD6
//#define PIN_POWER_EN_PORT       PORTD
//#define PIN_POWER_EN_GPIO       PTD
//#define PIN_POWER_EN_BIT        (6)
//#define PIN_POWER_EN            (1<<PIN_POWER_EN_BIT)

// VTRG_FAULT_B PTD7
//#define PIN_VTRG_FAULT_B_PORT   PORTD
//#define PIN_VTRG_FAULT_B_GPIO   PTD
//#define PIN_VTRG_FAULT_B_BIT    (7)

// Debug Unit LEDs

// Connected LED                PTD4
#define LED_CONNECTED_PORT      PORTD
#define LED_CONNECTED_GPIO      PTD
#define LED_CONNECTED_BIT       4

// Target Running LED           Not available

// Debug Unit LEDs





    
/******************************************************************************/
/*
 * ESSEMI-ISP IO define
 */
// ISPCLK Pin                   PTC5
#define PIN_ISPCLK_PORT         PORTC
#define PIN_ISPCLK_GPIO         PTC
#define PIN_ISPCLK_BIT          5

// ISPSDA Out Pin               PTC6
#define PIN_ISPSDA_OUT_PORT     PORTC
#define PIN_ISPSDA_OUT_GPIO     PTC
#define PIN_ISPSDA_OUT_BIT      6

// ISPSDA In Pin                PTC7
#define PIN_ISPSDA_IN_PORT      PORTC
#define PIN_ISPSDA_IN_GPIO      PTC
#define PIN_ISPSDA_IN_BIT       7

// ISPSDA_EN  Output Enable Pin PTC9
#define PIN_ISPSDA_NOE_PORT     PORTC
#define PIN_ISPSDA_NOE_GPIO     PTC
#define PIN_ISPSDA_NOE_BIT      9

// ISPSCK_EN  Enable Pin        PTC8
#define PIN_ISPSCK_NOE_PORT     PORTC
#define PIN_ISPSCK_NOE_GPIO     PTC
#define PIN_ISPSCK_NOE_BIT      8

// RST Pin                      PTC4
#define PIN_RST_PORT            PORTC
#define PIN_RST_GPIO            PTC
#define PIN_RST_BIT             4
#define PIN_RST                 (1 << PIN_RST_BIT)

// Control Pin PTC10
#define PIN_CTL_PORT            PORTC
#define PIN_CTL_GPIO            PTC
#define PIN_CTL_BIT             10

// 5V Select pin 
#define PIN_SELECT_5V_PORT      PORTD
#define PIN_SELECT_5V_GPIO      PTD
#define PIN_SELECT_5V_BIT       0


// 3.3V Select pin 
#define PIN_SELECT_3V3_PORT     PORTD
#define PIN_SELECT_3V3_GPIO     PTD
#define PIN_SELECT_3V3_BIT      1


#endif
