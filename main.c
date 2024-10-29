#include <stdint.h>
#include <stdbool.h>
#include "nrf52840.h"
#include "nrf_delay.h"

#define UART_TX_PIN 20

void uart_init(void)
{
    NRF_UART0->PSEL.TXD = UART_TX_PIN;

    NRF_UART0->PSEL.RTS = 0xFFFFFFFF;
    NRF_UART0->PSEL.CTS = 0xFFFFFFFF;

    NRF_UART0->BAUDRATE = 0x01D7E000;
    NRF_UART0->ENABLE = 4;
    NRF_UART0->TASKS_STARTTX = 1;
}

void uart_send_char(char c)
{
    NRF_UART0->TXD = c;
    while (NRF_UART0->EVENTS_TXDRDY == 0);
    NRF_UART0->EVENTS_TXDRDY = 0;
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