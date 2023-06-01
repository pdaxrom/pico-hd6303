#pragma once

#ifdef PICO_HD6303_HW

#define CPU_AD0  0
#define CPU_AD1  1
#define CPU_AD2  2
#define CPU_AD3  3
#define CPU_AD4  4
#define CPU_AD5  5
#define CPU_AD6  6
#define CPU_AD7  7
#define CPU_A8   8
#define CPU_A9   9
#define CPU_A10 10
#define CPI_A11 11
#define CPU_A12 12
#define CPU_A13 13
#define CPU_A14 14
#define CPU_A15 15
#define CPU_RW  16
#define CPU_AS  17
#define CPU_E   18
#define CPU_RST 19
#define CPU_CLK 24
#define CPU_NMI 26
#define CPU_IRQ 27

#define LED_PIN 25

#define SPI0_MISO 20
#define SPI0_CSN  21
#define SPI0_SCK  22
#define SPI0_MOSI 23

#define UART_TX_PIN (28)
#define UART_RX_PIN (29)
#define UART_ID     uart0
#define BAUD_RATE   115200

#else

/*
 * Minimal configuration:
 * NMI (4), IRQ1 (5), STBY (7), P21 (9) - +3.3v
 * P20 (8), P22 (10), VSS (1) - GND
 * VCC (21) - +3.3V
 */

#define CPU_AD0  0
#define CPU_AD1  1
#define CPU_AD2  2
#define CPU_AD3  3
#define CPU_AD4  4
#define CPU_AD5  5
#define CPU_AD6  6
#define CPU_AD7  7
#define CPU_A8   8
#define CPU_A9   9
#define CPU_A10 10
#define CPI_A11 11
#define CPU_A12 12
#define CPU_A13 13
#define CPU_A14 14
#define CPU_A15 15
#define CPU_RW  18
#define CPU_AS  19
#define CPU_E   20
#define CPU_RST 22
#define CPU_CLK 21

#define LED_PIN 25

#define UART_TX_PIN (16)
#define UART_RX_PIN (17)
#define UART_ID     uart0
#define BAUD_RATE   115200

#endif
