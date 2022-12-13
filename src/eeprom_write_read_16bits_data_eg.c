// Example to write and read 16 bits data.

#include "eeprom_spi_hd.h"

void eeprom_write_read_16bits_data_eg(void)
{
    esp_err_t result_ = ESP_OK;

    // Erases all data in eeprom.
    ESP_ERROR_CHECK(eeprom_chip_erase());
    ESP_LOGI(MAIN_TAG, "");

    // Writes data into the address 0x0280 - (640) - Page 5.
    addr = 0x0280;
    write_data3 = 65535;
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data into address: 0x%04X, Page 5.", addr);
    eeprom_write_data(addr, uint16_t, &write_data3, 1, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0x0280 - (10) - Page 5
    ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
    eeprom_read_data(addr, uint16_t, result_uint16, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
    ESP_LOGI(MAIN_TAG, "");
}
