// Example to print out the page number with start and end addresses.

#include "eeprom_spi_hd.h"

void eeprom_pages_eg(void)
{
    uint16_t start_addr;
    uint16_t end_addr;
    uint16_t page_num;
    
    ESP_LOGI(MAIN_TAG, "Prints page number, start and end addresses of pages.");
    page_num = 0;
    for (int i = 0; i < EEPROM_TOTAL_BYTES; i += EEPROM_PAGE_SIZE)
    {
        start_addr = i;
        end_addr = start_addr + EEPROM_PAGE_SIZE - 1;
        ESP_LOGI(MAIN_TAG, "Page %3d | 0x%04X | 0x%04X", page_num, start_addr, end_addr);
        page_num++;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}