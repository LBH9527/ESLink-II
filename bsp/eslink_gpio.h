
#ifndef ESLINK_GPIO_H
#define ESLINK_GPIO_H

#include "fsl_gpio.h" 
#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"

#ifdef __cplusplus
extern "C" {
#endif


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
// KEY0 PTA4
#define PIN_KEY0_PORT       PORTA
#define PIN_KEY0_GPIO       PTA
#define PIN_KEY0_BIT        (4)
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

//EEPROM 
// EEPROM SCL Pin                PTB0
#define PIN_IIC_SCL_PORT         PORTB
#define PIN_IIC_SCL_GPIO         PTB
#define PIN_IIC_SCL_BIT          0
#define PIN_IIC_SCL              (1 << PIN_IIC_SCL_BIT)
// EEPROM SDA Pin                PTB1
#define PIN_IIC_SDA_PORT         PORTB
#define PIN_IIC_SDA_GPIO         PTB
#define PIN_IIC_SDA_BIT          1
#define PIN_IIC_SDA              (1 << PIN_IIC_SDA_BIT)

/*******************************************************************************
*   POWER SET
*******************************************************************************/  
/* 3.3V Select */
#define V33_SELECT_INIT(output)    \
    GPIO_PinWrite(PIN_SELECT_3V3_GPIO, PIN_SELECT_3V3_BIT, output); \
    PIN_SELECT_3V3_GPIO->PDDR |= (1U << PIN_SELECT_3V3_BIT) 
#define V33_ON() \
    GPIO_PortClear(PIN_SELECT_3V3_GPIO, 1U << PIN_SELECT_3V3_BIT) 
#define V33_OFF() \
    GPIO_PortSet(PIN_SELECT_3V3_GPIO, 1U << PIN_SELECT_3V3_BIT)

/* 5V Select */
#define V5_SELECT_INIT(output)  \
    GPIO_PinWrite(PIN_SELECT_5V_GPIO, PIN_SELECT_5V_BIT, output); \
    PIN_SELECT_5V_GPIO->PDDR |= (1U << PIN_SELECT_5V_BIT) 
#define V5_ON() \
    GPIO_PortClear(PIN_SELECT_5V_GPIO, 1U << PIN_SELECT_5V_BIT) 
#define V5_OFF() \
    GPIO_PortSet(PIN_SELECT_5V_GPIO, 1U << PIN_SELECT_5V_BIT) 
/*******************************************************************************
*   ES_LINK ٦ŜIO
*******************************************************************************/
/* Control Pin */
#define CLT_INIT(output)      \
    GPIO_PinWrite(PIN_CTL_GPIO, PIN_CTL_BIT, output); \
    PIN_CTL_GPIO->PDDR |= (1U << PIN_CTL_BIT) 
#define CTL_SET() \
    GPIO_PortClear(PIN_CTL_GPIO, 1U << PIN_CTL_BIT) 
#define CTL_CLR() \
    GPIO_PortSet(PIN_CTL_GPIO, 1U << PIN_CTL_BIT) 
#define CTL_TOGGLE() \
    GPIO_PortToggle(PIN_CTL_GPIO, 1U << PIN_CTL_BIT) 
    
/*******************************************************************************
*   LED SET
*******************************************************************************/  
#define LED_GREEN_INIT(output)                                                   \
    GPIO_PinWrite(BOARD_LED_G_PASS_GPIO, BOARD_LED_G_PASS_GPIO_PIN, output); \
    BOARD_LED_G_PASS_GPIO->PDDR |= (1U << BOARD_LED_G_PASS_GPIO_PIN) 
#define LED_GREEN_ON() \
    GPIO_PortClear(BOARD_LED_G_PASS_GPIO, 1U << BOARD_LED_G_PASS_GPIO_PIN) 
#define LED_GREEN_OFF() \
    GPIO_PortSet(BOARD_LED_G_PASS_GPIO, 1U << BOARD_LED_G_PASS_GPIO_PIN) 
#define LED_GREEN_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_G_PASS_GPIO, 1U << BOARD_LED_G_PASS_GPIO_PIN) 

#define LED_RED_INIT(output)    \
    GPIO_PinWrite(BOARD_LED_Y_BUSY_GPIO, BOARD_LED_Y_BUSY_GPIO_PIN, output); \
    BOARD_LED_Y_BUSY_GPIO->PDDR |= (1U << BOARD_LED_Y_BUSY_GPIO_PIN) 
#define LED_RED_ON() \
    GPIO_PortClear(BOARD_LED_Y_BUSY_GPIO, 1U << BOARD_LED_Y_BUSY_GPIO_PIN) 
#define LED_RED_OFF() \
    GPIO_PortSet(BOARD_LED_Y_BUSY_GPIO, 1U << BOARD_LED_Y_BUSY_GPIO_PIN) 
#define LED_RED_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_Y_BUSY_GPIO, 1U << BOARD_LED_Y_BUSY_GPIO_PIN) 
    
#define LED_YELLOW_ON() \
        LED_RED_ON(); \
        LED_GREEN_ON()
#define LED_YELLOW_OFF() \
         LED_RED_OFF(); \
        LED_GREEN_OFF()   
   
/*******************************************************************************
*   BEEP PIN SET
*******************************************************************************/  
#define BEEP_INIT(output)   \
    GPIO_PinWrite(BOARD_BEEP_GPIO, BOARD_BEEP_GPIO_PIN, output); \
    BOARD_BEEP_GPIO->PDDR |= (1U << BOARD_BEEP_GPIO_PIN) 
#define BEEP_ON() \
    GPIO_PortClear(BOARD_BEEP_GPIO, 1U << BOARD_BEEP_GPIO_PIN)
#define BEEP_OFF() \
    GPIO_PortSet(BOARD_BEEP_GPIO, 1U << BOARD_BEEP_GPIO_PIN)
    
/*******************************************************************************
*   OLED PIN SET
*******************************************************************************/    
#define OLED_RST_INIT(output)  \
    GPIO_PinWrite(PIN_OLED_RST_GPIO, PIN_OLED_RST_BIT, output); \
    PIN_OLED_RST_GPIO->PDDR |= (1U << PIN_OLED_RST_BIT) /*!< Enable target LED_RED */
#define OLED_RST_Clr() \
    GPIO_PortClear(PIN_OLED_RST_GPIO, 1U << PIN_OLED_RST_BIT) /*!< Turn on target LED_RED */
#define OLED_RST_Set() \
    GPIO_PortSet(PIN_OLED_RST_GPIO, 1U << PIN_OLED_RST_BIT) /*!< Turn off target LED_RED */

#define OLED_CS_INIT(output)   \
    GPIO_PinWrite(PIN_OLED_CS_GPIO, PIN_OLED_CS_BIT, output); \
    PIN_OLED_CS_GPIO->PDDR |= (1U << PIN_OLED_CS_BIT) /*!< Enable target LED_RED */
#define OLED_CS_Clr() \
    GPIO_PortClear(PIN_OLED_CS_GPIO, 1U << PIN_OLED_CS_BIT) /*!< Turn on target LED_RED */
#define OLED_CS_Set() \
    GPIO_PortSet(PIN_OLED_CS_GPIO, 1U << PIN_OLED_CS_BIT) /*!< Turn off target LED_RED */

#define OLED_SCK_INIT(output)  \
    GPIO_PinWrite(PIN_OLED_SCK_GPIO, PIN_OLED_SCK_BIT, output); \
    PIN_OLED_SCK_GPIO->PDDR |= (1U << PIN_OLED_SCK_BIT) 
#define OLED_SCK_Clr() \
    GPIO_PortClear(PIN_OLED_SCK_GPIO, 1U << PIN_OLED_SCK_BIT) 
#define OLED_SCK_Set() \
    GPIO_PortSet(PIN_OLED_SCK_GPIO, 1U << PIN_OLED_SCK_BIT) 

#define OLED_SDA_INIT(output)   \
    GPIO_PinWrite(PIN_OLED_SDA_GPIO, PIN_OLED_SDA_BIT, output); \
    PIN_OLED_SDA_GPIO->PDDR |= (1U << PIN_OLED_SDA_BIT)
#define OLED_SDA_Clr() \
    GPIO_PortClear(PIN_OLED_SDA_GPIO, 1U << PIN_OLED_SDA_BIT) 
#define OLED_SDA_Set() \
    GPIO_PortSet(PIN_OLED_SDA_GPIO, 1U << PIN_OLED_SDA_BIT) 

/*******************************************************************************
*   SPI FLASH PIN SET
*******************************************************************************/ 
#define SF_CS_INIT(output)  \
    GPIO_PinWrite(FLASH_DSPI_INT_GPIO, FLASH_DSPI_INT_PIN, output); \
    FLASH_DSPI_INT_GPIO->PDDR |= (1U << FLASH_DSPI_INT_PIN) 
#define SF_CS_CLR() \
    GPIO_PortClear(FLASH_DSPI_INT_GPIO, 1U << FLASH_DSPI_INT_PIN) 
#define SF_CS_SET() \
    GPIO_PortSet(FLASH_DSPI_INT_GPIO, 1U << FLASH_DSPI_INT_PIN)
/*******************************************************************************
*   EEPROM PIN SET
*******************************************************************************/  
#define IIC_SCL_INIT(output)  \
    GPIO_PinWrite(PIN_IIC_SCL_GPIO, PIN_IIC_SCL_BIT, output); \
    PIN_IIC_SCL_GPIO->PDDR |= (1U << PIN_IIC_SCL_BIT) 
#define IIC_SCL_CLR() \
    GPIO_PortClear(PIN_IIC_SCL_GPIO, 1U << PIN_IIC_SCL_BIT)
#define IIC_SCL_SET() \
    GPIO_PortSet(PIN_IIC_SCL_GPIO, 1U << PIN_IIC_SCL_BIT)   
  
#define IIC_SDA_INIT(output)  \
    GPIO_PinWrite(PIN_IIC_SDA_GPIO, PIN_IIC_SDA_BIT, output); \
    PIN_IIC_SDA_GPIO->PDDR |= (1U << PIN_IIC_SDA_BIT)    
#define IIC_SDA_CLR() \
    GPIO_PortClear(PIN_IIC_SDA_GPIO, 1U << PIN_IIC_SDA_BIT)
#define IIC_SDA_SET() \
    GPIO_PortSet(PIN_IIC_SDA_GPIO, 1U << PIN_IIC_SDA_BIT)  
    
#define IIC_SDA_OUT()  \
    PIN_IIC_SDA_GPIO->PDDR |= (1U << PIN_IIC_SDA_BIT) 
#define IIC_SDA_IN() \
	PIN_IIC_SDA_GPIO->PDDR &= ~(1 << PIN_IIC_SDA_BIT)       
    
static __forceinline uint8_t IIC_SDA_READ(void)
{
    return (((PIN_IIC_SDA_GPIO->PDIR) >> PIN_IIC_SDA_BIT) & 0x01U);
}
 /* the trget powerr set */
typedef enum {
    TRGET_POWER_DISABLE,
    TRGET_POWER_ENABLE,
    TRGET_POWER_3V3,
    TRGET_POWER_5V,
}trget_power_t;   
    
//// wiring on PCB is unknown so implementations may vary
//typedef enum led_state {
//    GPIO_LED_OFF = 0,
//    GPIO_LED_ON
//} gpio_led_state_t;

void gpio_init(void);
//void gpio_set_board_power(bool powerEnabled);
//void gpio_set_hid_led(gpio_led_state_t state);
//void gpio_set_cdc_led(gpio_led_state_t state);
//void gpio_set_msc_led(gpio_led_state_t state);
uint8_t gpio_get_reset_btn_no_fwrd(void);
//uint8_t gpio_get_reset_btn_fwrd(void);

//static inline uint8_t gpio_get_reset_btn(void)
//{
//    return gpio_get_reset_btn_no_fwrd() || gpio_get_reset_btn_fwrd();
//}

uint8_t gpio_key0_down(void);
uint8_t gpio_key1_down(void);
uint8_t gpio_key2_down(void);
void es_set_trget_power(trget_power_t power);


#ifdef __cplusplus
}
#endif

#endif
