#include <stdbool.h>
#include <stdint.h>

#include "nrf52840.h"
#include "nrf_delay.h"
#include "nrf_drv_uart.h"

#define FLASH_START_ADDR 0x3F000  // адрес страницы для записи
#define FLASH_PAGE_SIZE 4096      // размер страницы флеш-памяти

// IP-адрес в формате uint32_t
uint32_t ip_address = 0xC0A80101;    // 0xC0 = 192, 0xA8 = 168, 0x01 = 1, 0x01 = 1
uint32_t ip_address_2 = 0xC0A80102;  // 192.168.1.2

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

void dummy_function() {
    // Простая инструкция, чтобы секция `.text` не была пустой
    __asm__ volatile("nop");
}

int main(void) {
    dummy_function();
    // Стираем страницу перед записью
    flash_page_erase(FLASH_START_ADDR);

    // Записываем IP-адрес в первую ячейку страницы
    flash_write(FLASH_START_ADDR, ip_address);
    flash_write(FLASH_START_ADDR, 0xC0A80001);  // 192.168.0.1 (Изменились только "A8" → "00")
    flash_write(FLASH_START_ADDR, ip_address);

    // Записываем второй IP-адрес через 4 байта
    flash_write(FLASH_START_ADDR + 4, ip_address_2);

    uart_init();
    while (true) {
        nrf_delay_ms(5000);
        uint32_t read_ip_address_1 = flash_read(FLASH_START_ADDR);
        print_ip_address(read_ip_address_1);

        uint32_t read_ip_address_2 = flash_read(FLASH_START_ADDR + 4);
        print_ip_address(read_ip_address_2);

        uint32_t const page_sz = NRF_FICR->CODEPAGESIZE;
        uint32_t const code_sz = NRF_FICR->CODESIZE;

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Page: %lu, Code: %lu\n", (unsigned long)page_sz, (unsigned long)code_sz);

        uart_put_string(buffer);

        uint32_t ram_size = NRF_FICR->INFO.RAM;      // Размер RAM в килобайтах
        uint32_t flash_size = NRF_FICR->INFO.FLASH;  // Размер Flash в килобайтах

        char buffer_for_memory[64];
        snprintf(buffer_for_memory, sizeof(buffer_for_memory), "RAM: %lu KB, Flash: %lu KB\n", (unsigned long)ram_size,
                 (unsigned long)flash_size);

        uart_put_string(buffer_for_memory);

        if (read_ip_address_1 == ip_address) {
            __NOP();
        }
    }
}
