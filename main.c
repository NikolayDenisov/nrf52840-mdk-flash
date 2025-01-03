#include <stdbool.h>
#include <stdint.h>

#include "nrf52840.h"
#include "nrf_delay.h"
#include "nrf_drv_uart.h"

#define FLASH_START_ADDR 0x3F000  // адрес страницы для записи
#define FLASH_PAGE_SIZE 4096      // размер страницы флеш-памяти

// IP-адрес в формате uint32_t
uint32_t ip_address = 0xC0A80101;  // 0xC0 = 192, 0xA8 = 168, 0x01 = 1, 0x01 = 1

nrf_drv_uart_t m_uart = NRF_DRV_UART_INSTANCE(0);

void flash_write(uint32_t address, uint32_t value) {
    // Разблокируем флеш-память для записи
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;            // WEN = Write enable
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);  // Ждём, пока контроллер готов

    // Записываем значение
    *(uint32_t *)address = value;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);  // Ждём завершения записи

    // Блокируем запись
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;  // WEN = Read only
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
}

void flash_page_erase(uint32_t address) {
    // Разблокируем флеш-память для стирания
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een;  // WEN = Erase enable
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);

    // Стираем страницу
    NRF_NVMC->ERASEPAGE = address;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);  // Ждём завершения стирания

    // Блокируем запись
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;  // WEN = Read only
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
}

uint32_t flash_read(uint32_t address) {
    // Чтение из флеш-памяти осуществляется напрямую
    return *(uint32_t *)address;
}

void uart_init(void) {
    nrf_drv_uart_config_t uart_config = NRF_DRV_UART_DEFAULT_CONFIG;
    uart_config.baudrate = NRF_UART_BAUDRATE_115200;
    uart_config.pseltxd = 20;
    uart_config.pselrxd = 19;

    nrf_drv_uart_init(&m_uart, &uart_config, NULL);
}

void uart_put_string(const char *str) {
    while (*str) {
        nrf_drv_uart_tx(&m_uart, (uint8_t *)str++, 1);
    }
}

void print_ip_address(uint32_t ip_addr) {
    uint8_t byte1 = (ip_addr >> 24) & 0xFF;
    uint8_t byte2 = (ip_addr >> 16) & 0xFF;
    uint8_t byte3 = (ip_addr >> 8) & 0xFF;
    uint8_t byte4 = ip_addr & 0xFF;

    char ip_str[16];
    snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d\n", byte1, byte2, byte3, byte4);

    uart_put_string(ip_str);
}

int main(void) {
    // Стираем страницу перед записью
    // flash_page_erase(FLASH_START_ADDR);

    // Записываем IP-адрес в первую ячейку страницы
    // flash_write(FLASH_START_ADDR, ip_address);
    uart_init();
    while (true) {
        nrf_delay_ms(5000);
        uint32_t read_ip_address = flash_read(FLASH_START_ADDR);
        print_ip_address(read_ip_address);
        if (read_ip_address == ip_address) {
            __NOP();
        }
    }
}
