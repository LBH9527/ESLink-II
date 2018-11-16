


#include "eslink_gpio.h"


void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

void BOARD_InitPins(void)
{
    /* Port A Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Port B Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortB);
    /* Port C Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortC);
    /* Port D Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortD);
    /* Port E Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortE);

    /* PORTA0 (pin 22) is configured as JTAG_TCLK */
    PORT_SetPinMux(PORTA, 0U, kPORT_MuxAlt7);

    /* PORTA1 (pin 23) is configured as PTA1 */
    PORT_SetPinMux(PIN_KEY0_PORT, PIN_KEY0_BIT, kPORT_MuxAsGpio);

    PORTA->PCR[1] = ((PORTA->PCR[1] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Enable: Internal pullup or pulldown resistor is not enabled on the corresponding pin. */
                     | PORT_PCR_PE(kPORT_PullDisable));

    /* PORTA12 (pin 28) is configured as FTM1_CH0 */
    PORT_SetPinMux(PORTA, 12U, kPORT_MuxAlt3);

    /* PORTA13 (pin 29) is configured as FTM1_CH1 */
    PORT_SetPinMux(PORTA, 13U, kPORT_MuxAlt3);

    /* PORTA18 (pin 32) is configured as EXTAL0 */
    PORT_SetPinMux(PORTA, 18U, kPORT_PinDisabledOrAnalog);

    /* PORTA19 (pin 33) is configured as XTAL0 */
    PORT_SetPinMux(PORTA, 19U, kPORT_PinDisabledOrAnalog);



    PORTA->PCR[2] = ((PORTA->PCR[2] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Enable: Internal pullup or pulldown resistor is not enabled on the corresponding pin. */
                     | PORT_PCR_PE(kPORT_PullDisable));

    /* PORTA3 (pin 25) is configured as JTAG_TMS */
    PORT_SetPinMux(PORTA, 3U, kPORT_MuxAlt7);

//    /* PORTA4 (pin 26) is configured as PTA4 */
//    PORT_SetPinMux(BOARD_KEY1_PORT, BOARD_KEY1_PIN, kPORT_MuxAsGpio);

    PORTA->PCR[4] = ((PORTA->PCR[4] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Enable: Internal pullup or pulldown resistor is not enabled on the corresponding pin. */
                     | PORT_PCR_PE(kPORT_PullDisable));

    /* PORTA5 (pin 27) is configured as PTA5 */
    PORT_SetPinMux(LED_G_PASS_PORT, LED_G_PASS_PIN, kPORT_MuxAsGpio);

    PORTA->PCR[5] = ((PORTA->PCR[5] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Enable: Internal pullup or pulldown resistor is enabled on the corresponding pin. */
                     | (uint32_t)(PORT_PCR_PE_MASK));

//    /* PORTB0 (pin 35) is configured as I2C0_SCL */
//    PORT_SetPinMux(PORTB, 0U, kPORT_MuxAlt2);
//    /* PORTB1 (pin 36) is configured as I2C0_SDA */
//    PORT_SetPinMux(PORTB, 1U, kPORT_MuxAlt2);
    /* PORTB19 (pin 42) is configured as PTB19 */
    PORT_SetPinMux(PIN_OLED_CS_PORT, PIN_OLED_CS_BIT, kPORT_MuxAsGpio);

    /* PORTB18 (pin 41) is configured as PTB18 */
    PORT_SetPinMux(PIN_OLED_RST_PORT, PIN_OLED_RST_BIT, kPORT_MuxAsGpio);

    /* PORTB2 (pin 37) is configured as PTB2 */
    PORT_SetPinMux(LED_R_ERROR_PORT, LED_R_ERROR_PIN, kPORT_MuxAsGpio);
    /* PORTB3 (pin 38) is configured as PTB3 */
    PORT_SetPinMux(LED_Y_BUSY_PORT, LED_Y_BUSY_PIN, kPORT_MuxAsGpio);

    /* PORTB16 (pin 39) is configured as FTM_CLKIN0 */
    PORT_SetPinMux(PORTB, 16U, kPORT_MuxAlt4);
    
    
    /* PORTC0 (pin 43) is configured as ADC0_SE14 */
    PORT_SetPinMux(PORTC, 0U, kPORT_PinDisabledOrAnalog);

    /* PORTC10 (pin 55) is configured as PTC10 */
    PORT_SetPinMux(BOARD_ES_CTL_IO_PORT, BOARD_ES_CTL_IO_PIN, kPORT_MuxAsGpio);

    /* PORTC2 (pin 45) is configured as PTC2 */
    PORT_SetPinMux(PIN_OLED_SCK_PORT, PIN_OLED_SCK_BIT, kPORT_MuxAsGpio);

    /* PORTC3 (pin 46) is configured as PTC3 */
    PORT_SetPinMux(PIN_OLED_SDA_PORT, PIN_OLED_SDA_BIT, kPORT_MuxAsGpio);

    /* PORTC4 (pin 49) is configured as PTC4 */
    PORT_SetPinMux(BOARD_ES_RST_PORT, BOARD_ES_RST_PIN, kPORT_MuxAsGpio);

    /* PORTC8 (pin 53) is configured as PTC8 */
    PORT_SetPinMux(BOARD_ES_SCK_EN_PORT, BOARD_ES_SCK_EN_PIN, kPORT_MuxAsGpio);

    /* PORTC9 (pin 54) is configured as PTC9 */
    PORT_SetPinMux(BOARD_ES_DO_EN_PORT, BOARD_ES_DO_EN_PIN, kPORT_MuxAsGpio);

    /* PORTD0 (pin 57) is configured as PTD0 */
    PORT_SetPinMux(BOARD_SELECT_5V_PORT, BOARD_SELECT_5V_PIN, kPORT_MuxAsGpio);

    /* PORTD1 (pin 58) is configured as PTD1 */
    PORT_SetPinMux(BOARD_SELECT_3V3_PORT, BOARD_SELECT_3V3_PIN, kPORT_MuxAsGpio);

    /* PORTD2 (pin 59) is configured as UART2_RX */
    PORT_SetPinMux(PORTD, 2U, kPORT_MuxAlt3);

    /* PORTD3 (pin 60) is configured as PTD3 */
    PORT_SetPinMux(BEEP_PORT, BEEP_PIN, kPORT_MuxAsGpio);

    /* PORTD4 (pin 61) is configured as SPI1_PCS0 */
    PORT_SetPinMux(PORTD, 4U, kPORT_MuxAsGpio);

    /* PORTD5 (pin 62) is configured as SPI1_SCK */
    PORT_SetPinMux(PORTD, 5U, kPORT_MuxAlt7);

    /* PORTD6 (pin 63) is configured as SPI1_SOUT */
    PORT_SetPinMux(PORTD, 6U, kPORT_MuxAlt7);

    /* PORTD7 (pin 64) is configured as SPI1_SIN */
    PORT_SetPinMux(PORTD, 7U, kPORT_MuxAlt7);

//    /* PORTE0 (pin 1) is configured as UART1_TX */
//    PORT_SetPinMux(PORTE, 0U, kPORT_MuxAlt3);

//    /* PORTE1 (pin 2) is configured as UART1_RX */
//    PORT_SetPinMux(PORTE, 1U, kPORT_MuxAlt3);

    SIM->SOPT4 = ((SIM->SOPT4 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT4_FTM1CH0SRC_MASK | SIM_SOPT4_FTM1CLKSEL_MASK)))

                  /* FTM1 channel 0 input capture source select: FTM1_CH0 signal. */
                  | SIM_SOPT4_FTM1CH0SRC(SOPT4_FTM1CH0SRC_FTM)

                  /* FTM1 External Clock Pin Select: FTM_CLK0 pin. */
                  | SIM_SOPT4_FTM1CLKSEL(SOPT4_FTM1CLKSEL_CLK0));

    SIM->SOPT5 = ((SIM->SOPT5 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT5_UART1TXSRC_MASK)))

                  /* UART 1 transmit data source select: UART1_TX pin. */
                  | SIM_SOPT5_UART1TXSRC(SOPT5_UART1TXSRC_UART_TX));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
