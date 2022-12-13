// Example to write and read 8 bits data.

#include "eeprom_spi_hd.h"

void eeprom_write_read_8bits_data_eg(void)
{
    esp_err_t result_ = ESP_OK;

    // Erases all data in eeprom.
    ESP_ERROR_CHECK(eeprom_chip_erase());
    ESP_LOGI(MAIN_TAG, "");

    // Write data into the start address 0x000A - (10) - Page 0 (no overlap between pages).
    write_data[0] = -120;
    write_data[1] = 110;
    write_data[2] = -105;
    addr = 0x000A;
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data into the start address: 0x%04X, Page 0 (no overlap between pages).", addr);
    eeprom_write_data(addr, int8_t, write_data, ARRAY_SIZE, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0x000A - (10) - Page 0
    
    for (uint8_t i = 0; i < ARRAY_SIZE; i++)
    {
        ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X", addr);
        eeprom_read_data(addr, int8_t, result_int8, result_);
        ESP_ERROR_CHECK(result_);
        ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_int8);
        addr += sizeof(result_int8);
    }

    // Writes data into the address 0x00FE - (254) - Page 1 (overlap between pages Page 1 and Page 2).
    addr = 0x00FE;
    write_data[0] = 12;
    write_data[1] = -100;
    write_data[2] = -128;
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data into the start address: 0x%04X, Page 1 (overlap between pages Page 1 and Page 2).", addr);
    eeprom_write_data(addr, int8_t, write_data, ARRAY_SIZE, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    for (uint8_t i = 0; i < ARRAY_SIZE; i++)
    {
        ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X", addr);
        eeprom_read_data(addr, int8_t, result_int8, result_);
        ESP_ERROR_CHECK(result_);
        ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_int8);
        addr += sizeof(result_int8);
    }
}