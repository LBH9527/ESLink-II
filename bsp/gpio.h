
#ifndef GPIO_H
#define GPIO_H
#include "eslink_io_config.h"
#include "fsl_gpio.h"
#include "pin_mux.h"

#ifdef __cplusplus
extern "C" {
#endif

 /* the trget powerr set */
typedef enum {
    TRGET_POWER_DISABLE,
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
void gpio_set_trget_power(trget_power_t power);
/*******************************************************************************
*   POWER SET
*******************************************************************************/  
/* 3.3V Select */
#define V33_SELECT_INIT(output)                                                 \
    GPIO_PinWrite(PIN_SELECT_3V3_GPIO, PIN_SELECT_3V3_BIT, output); \
    PIN_SELECT_3V3_GPIO->PDDR |= (1U << PIN_SELECT_3V3_BIT) 
#define V33_ON() \
    GPIO_PortClear(PIN_SELECT_3V3_GPIO, 1U << PIN_SELECT_3V3_BIT) 
#define V33_OFF() \
    GPIO_PortSet(PIN_SELECT_3V3_GPIO, 1U << PIN_SELECT_3V3_BIT)

/* 5V Select */
#define V5_SELECT_INIT(output)                                                 \
    GPIO_PinWrite(PIN_SELECT_5V_GPIO, PIN_SELECT_5V_BIT, output); \
    PIN_SELECT_5V_GPIO->PDDR |= (1U << PIN_SELECT_5V_BIT) 
#define V5_ON() \
    GPIO_PortClear(PIN_SELECT_5V_GPIO, 1U << PIN_SELECT_5V_BIT) 
#define V5_OFF() \
    GPIO_PortSet(PIN_SELECT_5V_GPIO, 1U << PIN_SELECT_5V_BIT) 
    
/*******************************************************************************
*   LED SET
*******************************************************************************/  
/* Board led  */
#define LED_RED_INIT(output)                                                 \
    GPIO_PinWrite(BOARD_LED_R_ERROR_GPIO, BOARD_LED_R_ERROR_GPIO_PIN, output); \
    BOARD_LED_R_ERROR_GPIO->PDDR |= (1U << BOARD_LED_R_ERROR_GPIO_PIN) /*!< Enable target LED_RED */
#define LED_RED_ON() \
    GPIO_PortClear(BOARD_LED_R_ERROR_GPIO, 1U << BOARD_LED_R_ERROR_GPIO_PIN) /*!< Turn on target LED_RED */
#define LED_RED_OFF() \
    GPIO_PortSet(BOARD_LED_R_ERROR_GPIO, 1U << BOARD_LED_R_ERROR_GPIO_PIN) /*!< Turn off target LED_RED */
#define LED_RED_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_R_ERROR_GPIO, 1U << BOARD_LED_R_ERROR_GPIO_PIN) /*!< Toggle on target LED_RED */

#define LED_GREEN_INIT(output)                                                   \
    GPIO_PinWrite(BOARD_LED_G_PASS_GPIO, BOARD_LED_G_PASS_GPIO_PIN, output); \
    BOARD_LED_G_PASS_GPIO->PDDR |= (1U << BOARD_LED_G_PASS_GPIO_PIN) /*!< Enable target LED_GREEN */
#define LED_GREEN_ON() \
    GPIO_PortClear(BOARD_LED_G_PASS_GPIO, 1U << BOARD_LED_G_PASS_GPIO_PIN) /*!< Turn on target LED_GREEN */
#define LED_GREEN_OFF() \
    GPIO_PortSet(BOARD_LED_G_PASS_GPIO, 1U << BOARD_LED_G_PASS_GPIO_PIN) /*!< Turn off target LED_GREEN */
#define LED_GREEN_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_G_PASS_GPIO, 1U << BOARD_LED_G_PASS_GPIO_PIN) /*!< Toggle on target LED_GREEN */

#define LED_YELLOW_INIT(output)                                                  \
    GPIO_PinWrite(BOARD_LED_Y_BUSY_GPIO, BOARD_LED_Y_BUSY_GPIO_PIN, output); \
    BOARD_LED_Y_BUSY_GPIO->PDDR |= (1U << BOARD_LED_Y_BUSY_GPIO_PIN) /*!< Enable target LED_BLUE */
#define LED_YELLOW_ON() \
    GPIO_PortClear(BOARD_LED_Y_BUSY_GPIO, 1U << BOARD_LED_Y_BUSY_GPIO_PIN) /*!< Turn on target LED_BLUE */
#define LED_YELLOW_OFF() \
    GPIO_PortSet(BOARD_LED_Y_BUSY_GPIO, 1U << BOARD_LED_Y_BUSY_GPIO_PIN) /*!< Turn off target LED_BLUE */
#define LED_YELLOW_TOGGLE() \
    GPIO_PortToggle(BOARD_LED_Y_BUSY_GPIO, 1U << BOARD_LED_Y_BUSY_GPIO_PIN) /*!< Toggle on target LED_BLUE */

/*******************************************************************************
*   OLED PIN SET
*******************************************************************************/    
#define OLED_RST_INIT(output)                                                 \
    GPIO_PinWrite(PIN_OLED_RST_GPIO, PIN_OLED_RST_BIT, output); \
    PIN_OLED_RST_GPIO->PDDR |= (1U << PIN_OLED_RST_BIT) /*!< Enable target LED_RED */
#define OLED_RST_Clr() \
    GPIO_PortClear(PIN_OLED_RST_GPIO, 1U << PIN_OLED_RST_BIT) /*!< Turn on target LED_RED */
#define OLED_RST_Set() \
    GPIO_PortSet(PIN_OLED_RST_GPIO, 1U << PIN_OLED_RST_BIT) /*!< Turn off target LED_RED */

#define OLED_CS_INIT(output)                                                 \
    GPIO_PinWrite(PIN_OLED_CS_GPIO, PIN_OLED_CS_BIT, output); \
    PIN_OLED_CS_GPIO->PDDR |= (1U << PIN_OLED_CS_BIT) /*!< Enable target LED_RED */
#define OLED_CS_Clr() \
    GPIO_PortClear(PIN_OLED_CS_GPIO, 1U << PIN_OLED_CS_BIT) /*!< Turn on target LED_RED */
#define OLED_CS_Set() \
    GPIO_PortSet(PIN_OLED_CS_GPIO, 1U << PIN_OLED_CS_BIT) /*!< Turn off target LED_RED */

#define OLED_SCK_INIT(output)                                                 \
    GPIO_PinWrite(PIN_OLED_SCK_GPIO, PIN_OLED_SCK_BIT, output); \
    PIN_OLED_SCK_GPIO->PDDR |= (1U << PIN_OLED_SCK_BIT) /*!< Enable target LED_RED */
#define OLED_SCK_Clr() \
    GPIO_PortClear(PIN_OLED_SCK_GPIO, 1U << PIN_OLED_SCK_BIT) /*!< Turn on target LED_RED */
#define OLED_SCK_Set() \
    GPIO_PortSet(PIN_OLED_SCK_GPIO, 1U << PIN_OLED_SCK_BIT) /*!< Turn off target LED_RED */

#define OLED_SDA_INIT(output)                                                 \
    GPIO_PinWrite(PIN_OLED_SDA_GPIO, PIN_OLED_SDA_BIT, output); \
    PIN_OLED_SDA_GPIO->PDDR |= (1U << PIN_OLED_SDA_BIT) /*!< Enable target LED_RED */
#define OLED_SDA_Clr() \
    GPIO_PortClear(PIN_OLED_SDA_GPIO, 1U << PIN_OLED_SDA_BIT) /*!< Turn on target LED_RED */
#define OLED_SDA_Set() \
    GPIO_PortSet(PIN_OLED_SDA_GPIO, 1U << PIN_OLED_SDA_BIT) /*!< Turn off target LED_RED */


#ifdef __cplusplus
}
#endif

#endif
