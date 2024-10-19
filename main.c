#include <stdint.h>
#include <stdbool.h>
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_delay.h"

#define UART_TX_PIN 20

void uart_init(void)
{
    NRF_P0->PIN_CNF[UART_TX_PIN] = (1 << 0) | // Output direction
                                   (1 << 1) | // Input buffer disconnected
                                   (0 << 2) ; // Connect pull-up or pull-down resistors

    NRF_UART0->PSEL.TXD = UART_TX_PIN;

    NRF_UART0->PSEL.RTS = 0xFFFFFFFF;
    NRF_UART0->PSEL.CTS = 0xFFFFFFFF;

    NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud115200;
    NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled;
    NRF_UART0->TASKS_STARTTX = 1;
}

void uart_send_char(char c)
{
    NRF_UART0->TXD = c;
    while (NRF_UART0->EVENTS_TXDRDY == 0);
    NRF_UART0->EVENTS_TXDRDY = 0;
}

void uart_send_string(const char* str) {
    while (*str != '\0') {
        uart_send_char(*str);
        str++;
    }
}


int main(void)
{
    uart_init();
    const char s = 'S';

    while (true)
    {
        uart_send_char(s);
        nrf_delay_ms(5000);
    }
}