// Example to write and read 32 bits data.

#include "eeprom_spi_hd.h"

void eeprom_write_read_32bits_data_eg(void)
{
    esp_err_t result_ = ESP_OK;

    // Erases all data in eeprom.
    ESP_ERROR_CHECK(eeprom_chip_erase());
    ESP_LOGI(MAIN_TAG, "");

    // Writes data into the address 0x0180 - (384) - Page 3.
    addr = 0x0180;
    write_data2[0] = 1000000;
    write_data2[1] = 4294967295;
    write_data2[2] = 300000001;
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data into the start address: 0x%04X, Page 3 (no overlap between pages).", addr);
    eeprom_write_data(addr, uint32_t, write_data2, ARRAY_SIZE, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0x0180 - (384) - Page 3
    for (uint8_t i = 0; i < ARRAY_SIZE; i++)
    {
        ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X", addr);
        eeprom_read_data(addr, uint32_t, result_uint32, result_);
        ESP_ERROR_CHECK(result_);
        ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %zu", addr, result_uint32);
        addr += sizeof(result_uint32);
    }
}