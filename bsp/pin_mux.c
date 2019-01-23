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

    /* PORTA12 (pin 28) is configured as gpio */
    PORT_SetPinMux(PIN_RTC_OUT_PORT, PIN_RTC_OUT_BIT, kPORT_MuxAsGpio);

    /* PORTA13 (pin 29) is configured as FTM1_CH1 */
    PORT_SetPinMux(PORTA, 13U, kPORT_MuxAlt3);

    /* PORTA18 (pin 32) is configured as EXTAL0 */
    PORT_SetPinMux(PORTA, 18U, kPORT_PinDisabledOrAnalog);

    /* PORTA19 (pin 33) is configured as XTAL0 */
    PORT_SetPinMux(PORTA, 19U, kPORT_PinDisabledOrAnalog);

    /* PORTA3 (pin 25) is configured as JTAG_TMS */
    PORT_SetPinMux(PORTA, 3U, kPORT_MuxAlt7);

    /* PORTB17 (pin 40) is configured as FTM_CLKIN1 */
    PORT_SetPinMux(PORTB, 17U, kPORT_MuxAlt4);
    
    /* PORTC0 (pin 43) is configured as ADC0_SE14 */
    PORT_SetPinMux(PORTC, 0U, kPORT_PinDisabledOrAnalog);

    /* PORTD0 (pin 57) is configured as PTD0 */
    PORT_SetPinMux(PIN_SELECT_5V_PORT, PIN_SELECT_5V_BIT, kPORT_MuxAsGpio);

    /* PORTD1 (pin 58) is configured as PTD1 */
    PORT_SetPinMux(PIN_SELECT_3V3_PORT, PIN_SELECT_3V3_BIT, kPORT_MuxAsGpio);

    /* PORTD2 (pin 59) is configured as UART2_RX */
    PORT_SetPinMux(PIN_SWO_PORT, PIN_SWO_BIT, kPORT_MuxAsGpio);

    /* PORTD4 (pin 61) is configured as SPI1_PCS0 */
    PORT_SetPinMux(PORTD, 4U, kPORT_MuxAsGpio);

    /* PORTD5 (pin 62) is configured as SPI1_SCK */
    PORT_SetPinMux(PORTD, 5U, kPORT_MuxAlt7);

    /* PORTD6 (pin 63) is configured as SPI1_SOUT */
    PORT_SetPinMux(PORTD, 6U, kPORT_MuxAlt7);

    /* PORTD7 (pin 64) is configured as SPI1_SIN */
    PORT_SetPinMux(PORTD, 7U, kPORT_MuxAlt7);

//    SIM->SOPT4 = ((SIM->SOPT4 &
//                   /* Mask bits to zero which are setting */
//                   (~(SIM_SOPT4_FTM1CH0SRC_MASK | SIM_SOPT4_FTM1CLKSEL_MASK)))

//                  /* FTM1 channel 0 input capture source select: FTM1_CH0 signal. */
//                  | SIM_SOPT4_FTM1CH0SRC(SOPT4_FTM1CH0SRC_FTM)

//                  /* FTM1 External Clock Pin Select: FTM_CLK0 pin. */
//                  | SIM_SOPT4_FTM1CLKSEL(SOPT4_FTM1CLKSEL_CLK0));

    SIM->SOPT4 = ((SIM->SOPT4 &
                   /* Mask bits to zero which are setting */
                   (~(SIM_SOPT4_FTM1CLKSEL_MASK)))

                  /* FTM1 External Clock Pin Select: FTM_CLK1 pin. */
                  | SIM_SOPT4_FTM1CLKSEL(SOPT4_FTM1CLKSEL_CLK1));
//    SIM->SOPT5 = ((SIM->SOPT5 &
//                   /* Mask bits to zero which are setting */
//                   (~(SIM_SOPT5_UART1TXSRC_MASK)))

//                  /* UART 1 transmit data source select: UART1_TX pin. */
//                  | SIM_SOPT5_UART1TXSRC(SOPT5_UART1TXSRC_UART_TX));

}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
