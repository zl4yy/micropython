/*

	Basic UART control for Texas Instruments LM4F Microcontrollers
	Based on micropython's minimal port code
	
	Yannick Devos - ZL4YY (https://blog.qscope.org)
	https://github.com/zl4yy/micropython
	10 May 2021
	
	Software provided under MIT License

*/

#include <unistd.h>
#include "boards/lm4f_registers.h"
#include "py/mpconfig.h"

/*
 * Core UART functions to implement for a port
 */

#if MICROPY_MIN_USE_STM32_MCU
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
} periph_uart_t;
#define USART1 ((periph_uart_t *)0x40011000)

#elif MICROPY_MIN_USE_LM4F_MCU
#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable

// Send single character
void UART_OutChar(char data){
  while((UART0_FR_R&UART_FR_TXFF) != 0);
  UART0_DR_R = data;
};
#endif


// Receive single character
int mp_hal_stdin_rx_chr(void) {
    unsigned char c = 0;

    #if MICROPY_MIN_USE_STDOUT
    int r = read(STDIN_FILENO, &c, 1);
    (void)r;
    #elif MICROPY_MIN_USE_STM32_MCU
    // wait for RXNE
    while ((USART1->SR & (1 << 5)) == 0) {};
    c = USART1->DR;
    #elif MICROPY_MIN_USE_LM4F_MCU
    while((UART0_FR_R&UART_FR_RXFE) != 0) {};
    c = (char)(UART0_DR_R&0xFF);
    #endif
    return c;
};

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, mp_uint_t len) {
    #if MICROPY_MIN_USE_STDOUT
    int r = write(STDOUT_FILENO, str, len);
    (void)r;
    #elif MICROPY_MIN_USE_STM32_MCU
    while (len--) {
        // wait for TXE
        while ((USART1->SR & (1 << 7)) == 0) {};
        USART1->DR = *str++;
    };
    #elif MICROPY_MIN_USE_LM4F_MCU
    while (len--) {
        UART_OutChar(*str++);
    };
    #endif
};
