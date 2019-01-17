
#ifndef ESLINK_GPIO_H
#define ESLINK_GPIO_H

#include "fsl_gpio.h" 
#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"

#ifdef __cplusplus
extern "C" {
#endif

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


// nRESET out Pin                PTC4
#define PIN_nRESET_O_PORT        PORTC
#define PIN_nRESET_O_GPIO        PTC
#define PIN_nRESET_O_BIT         4
#define PIN_nRESET_O             (1 << PIN_nRESET_O_BIT)

// nRESET in Pin                 PTC11
#define PIN_nRESET_I_PORT         PORTC
#define PIN_nRESET_I_GPIO         PTC
#define PIN_nRESET_I_BIT          11
#define PIN_nRESET_I              (1 << PIN_nRESET_I_BIT)

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

// RST out Pin                  PTC4
#define PIN_RST_O_PORT        PORTC
#define PIN_RST_O_GPIO        PTC
#define PIN_RST_O_BIT         4
#define PIN_RST_O             (1 << PIN_RST_O_BIT)

// RST in Pin                   PTC11
#define PIN_RST_I_PORT         PORTC
#define PIN_RST_I_GPIO         PTC
#define PIN_RST_I_BIT          11
#define PIN_RST_I              (1 << PIN_RST_I_BIT)

// Control Pin PTC10
#define PIN_CTL_PORT            PORTC
#define PIN_CTL_GPIO            PTC
#define PIN_CTL_BIT             10

//ES_SWO    PTD2
#define PIN_SWO_PORT            PORTD
#define PIN_SWO_GPIO            PTD
#define PIN_SWO_BIT             (2)
#define PIN_SWO                 (1<<PIN_SWO_BIT)      

/******************************************************************************/
// Debug Unit LEDs
// 机台信号和led
// PORTA5 (number 27), LED_G/PASS_OUT
#define LED_G_PASS_PORT     PORTA 
#define LED_G_PASS_GPIO     GPIOA 
#define LED_G_PASS_PIN      5U  

// PORTB3 (number 38), LED_Y/BUSY_OUT
#define LED_Y_BUSY_PORT     PORTB 
#define LED_Y_BUSY_GPIO     GPIOB 
#define LED_Y_BUSY_PIN      3U   

// ERROR OUT/ LED_RED    PTB2
#define LED_R_ERROR_PORT     PORTB 
#define LED_R_ERROR_GPIO     GPIOB 
#define LED_R_ERROR_PIN      2U  

// Connected LED   PORTA5 (number 27), LED_G/PASS_OUT
#define LED_CONNECTED_PORT      PORTA
#define LED_CONNECTED_GPIO      PTA
#define LED_CONNECTED_BIT       5U

// Target Running LED           Not available   //busy    

//START_IN Pin   PTB16
#define PIN_START_IN_PORT       PORTB
#define PIN_START_IN_GPIO       PTB
#define PIN_START_IN_BIT        (16)
#define PIN_START_IN            (1 << PIN_START_IN_BIT) 

//CTL IO Pin   PTA4
#define PIN_CTL_1_PORT            PORTA
#define PIN_CTL_1_GPIO            PTA
#define PIN_CTL_1_BIT             4 

//BEEP  PORTC1 (number 44) 
#define BEEP_PORT               PORTC 
#define BEEP_GPIO               GPIOC 
#define BEEP_PIN                1   

/******************************************************************************/
//KEY Port I/O Pins
// KEY1  PTD3  
#define PIN_KEY0_PORT           PORTD
#define PIN_KEY0_GPIO           PTD
#define PIN_KEY0_BIT            (3)
#define PIN_KEY0                (1<<PIN_KEY0_BIT)

/******************************************************************************/
// 5V Select pin 
#define PIN_SELECT_5V_PORT      PORTD
#define PIN_SELECT_5V_GPIO      PTD
#define PIN_SELECT_5V_BIT       0 

// 3.3V Select pin 
#define PIN_SELECT_3V3_PORT     PORTD
#define PIN_SELECT_3V3_GPIO     PTD
#define PIN_SELECT_3V3_BIT      1

/******************************************************************************/
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

/******************************************************************************/
/*
 * RTC OUT Pin
 */
// RTC_OUT  PTA12
#define PIN_RTC_OUT_PORT        PORTA
#define PIN_RTC_OUT_GPIO        PTA
#define PIN_RTC_OUT_BIT         (12)
#define PIN_RTC_OUT             (1<<PIN_PTC_OUT_BIT) 
 
/*******************************************************************************
*   LED SET       led灯逻辑错误
*******************************************************************************/ 
#define LED_GREEN_PASS_INIT(output)  \
    GPIO_PinWrite(LED_G_PASS_GPIO, LED_G_PASS_PIN, output); \
    LED_G_PASS_GPIO->PDDR |= (1U << LED_G_PASS_PIN) 
#define LED_GREEN_PASS_OFF() \
    GPIO_PortClear(LED_G_PASS_GPIO, 1U << LED_G_PASS_PIN) 
#define LED_GREEN_PASS_ON() \
    GPIO_PortSet(LED_G_PASS_GPIO, 1U << LED_G_PASS_PIN) 
#define LED_GREEN_PASS_TOGGLE() \
    GPIO_PortToggle(LED_G_PASS_GPIO, 1U << LED_G_PASS_PIN) 

#define LED_YELLOW_BUSY_INIT(output)    \
    GPIO_PinWrite(LED_Y_BUSY_GPIO, LED_Y_BUSY_PIN, output); \
    LED_Y_BUSY_GPIO->PDDR |= (1U << LED_Y_BUSY_PIN) 
#define LED_YELLOW_BUSY_OFF() \
    GPIO_PortClear(LED_Y_BUSY_GPIO, 1U << LED_Y_BUSY_PIN) 
#define LED_YELLOW_BUSY_ON()   \
    GPIO_PortSet(LED_Y_BUSY_GPIO, 1U << LED_Y_BUSY_PIN) 
#define LED_YELLOW_BUSY_TOGGLE() \
    GPIO_PortToggle(LED_Y_BUSY_GPIO, 1U << LED_Y_BUSY_PIN) 
 
#define LED_RED_ERROR_INIT(output)    \
    GPIO_PinWrite(LED_R_ERROR_GPIO, LED_R_ERROR_PIN, output);   \
    LED_Y_BUSY_GPIO->PDDR |= (1U << LED_R_ERROR_PIN)        

#define LED_RED_ERROR_OFF()  \
    GPIO_PortClear(LED_R_ERROR_GPIO, 1U << LED_R_ERROR_PIN) 
#define LED_RED_ERROR_ON() \
    GPIO_PortSet(LED_R_ERROR_GPIO, 1U << LED_R_ERROR_PIN) 
#define LED_RED_ERROR_TOGGLE() \
    GPIO_PortToggle(LED_R_ERROR_GPIO, 1U << LED_R_ERROR_PIN)     
 
 //绿灯亮     
#define LED_GREEN_ON() \
        LED_RED_ERROR_OFF();    \
        LED_YELLOW_BUSY_OFF();  \
        LED_GREEN_PASS_ON()  
        
//红灯亮             
#define LED_RED_ON()   \
        LED_RED_ERROR_ON();    \
        LED_YELLOW_BUSY_OFF();  \
        LED_GREEN_PASS_OFF()    
//黄灯亮   
#define LED_YELLOW_ON() \
        LED_RED_ERROR_OFF();    \
        LED_YELLOW_BUSY_ON();  \
        LED_GREEN_PASS_OFF()   

//#define LED_GREEN_TOGGLE()   \
//        LED_YELLOW_BUSY_OFF(); \
//        LED_GREEN_PASS_TOGGLE() 

        
/*******************************************************************************
*   BEEP PIN SET
*******************************************************************************/  
#define BEEP_INIT(output)   \
    GPIO_PinWrite(BEEP_GPIO, BEEP_PIN, output); \
    BEEP_GPIO->PDDR |= (1U << BEEP_PIN) 
#define BEEP_OFF() \
    GPIO_PortClear(BEEP_GPIO, 1U << BEEP_PIN)
#define BEEP_ON() \
    GPIO_PortSet(BEEP_GPIO, 1U << BEEP_PIN)

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
    GPIO_PortSet(PIN_SELECT_5V_GPIO, 1U << PIN_SELECT_5V_BIT) 
#define V5_OFF() \
    GPIO_PortClear(PIN_SELECT_5V_GPIO, 1U << PIN_SELECT_5V_BIT) 
    
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

void gpio_init(void);     
uint8_t gpio_get_reset_btn_no_fwrd(void); 
uint8_t gpio_key0_down(void);

#ifdef __cplusplus
}
#endif

#endif
