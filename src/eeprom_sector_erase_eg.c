// Example to erase sector of eeprom by specified address.

#include "eeprom_spi_hd.h"

void eeprom_sector_erase_eg(void)
{
    esp_err_t result_ = ESP_OK;

    // Writes data into the address 0xFFF0 - (65520) - Sector 3.
    addr = 0xFFF0;
    write_data3 = 22222;
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data %d into address: 0x%04X, Sector 3.", write_data3, addr);
    eeprom_write_data(addr, uint16_t, &write_data3, 1, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0xFFF0 - (65520) - Sector 3
    ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
    eeprom_read_data(addr, uint16_t, result_uint16, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
    ESP_LOGI(MAIN_TAG, "");

    // Erases sector in eeprom.
    ESP_LOGI(MAIN_TAG, "Erase sector of eeprom. Used address in sector: 0xFF00");
    ESP_ERROR_CHECK(eeprom_sector_erase(0xFF00));

    // Read data from address 0xFFF0 - (65520) - Sector 3
    ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X after erasing the Sector 3", addr);
    eeprom_read_data(addr, uint16_t, result_uint16, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGW(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
    ESP_LOGI(MAIN_TAG, "");
}