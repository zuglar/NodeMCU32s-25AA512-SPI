// Example to erase page of eeprom by specified address.

#include "eeprom_spi_hd.h"

void eeprom_page_erase_eg(void)
{
    // Erases all data in eeprom.
    ESP_ERROR_CHECK(eeprom_chip_erase());
    ESP_LOGI(MAIN_TAG, "");
    // Writes new data in the eeprom.
    eeprom_write_read_char_array_eg();
    ESP_LOGI(MAIN_TAG, "");
    // Checks data which are written in the page.
    ESP_ERROR_CHECK(eeprom_page_stored_data(10));
    ESP_LOGI(MAIN_TAG, "");
    // Erase page by specified address.
    ESP_ERROR_CHECK(eeprom_page_erase(0x0571));
    ESP_LOGI(MAIN_TAG, "");
    // Checks data in the erased page.
    ESP_ERROR_CHECK(eeprom_page_stored_data(10));
}