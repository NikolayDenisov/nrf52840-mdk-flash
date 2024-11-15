#include <stdint.h>
#include <stdbool.h>
#include "nrf52840.h"
#include "nrf_delay.h"

#define FLASH_START_ADDR  0x3F000  // адрес страницы для записи (выберите подходящий под вашу задачу)
#define FLASH_PAGE_SIZE   4096     // размер страницы флеш-памяти

// Пример IP-адреса в формате uint32_t (например, 192.168.1.1)
uint32_t ip_address = 0xC0A80101;  // 0xC0 = 192, 0xA8 = 168, 0x01 = 1, 0x01 = 1

void flash_write(uint32_t address, uint32_t value) {
    // Разблокируем флеш-память для записи
    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;  // WEN = Write enable
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy);  // Ждём, пока контроллер готов

    // Записываем значение
    *(uint32_t*)address = value;
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
    return *(uint32_t*)address;
}

int main(void) {
    // Стираем страницу перед записью
    // flash_page_erase(FLASH_START_ADDR);

    // Записываем IP-адрес в первую ячейку страницы
    // flash_write(FLASH_START_ADDR, ip_address);

    while (true) {
        nrf_delay_us(5000);
        uint32_t read_ip_address = flash_read(FLASH_START_ADDR);
        if (read_ip_address == ip_address) {
            __NOP();
        }
    }
}
