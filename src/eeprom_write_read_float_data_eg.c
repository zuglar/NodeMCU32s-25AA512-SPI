// Example to write and read float number.

#include "eeprom_spi_hd.h"

void eeprom_write_read_float_data_eg(void)
{

    // Erases all data in eeprom.
    ESP_ERROR_CHECK(eeprom_chip_erase());
    ESP_LOGI(MAIN_TAG, "");

    // Writes float data into the address 0x0300 - (768) - Page 6.
    float float_data = -3.14159265359;
    addr = 0x0300;
    ESP_LOGI(EEPROM_TAG, "Writes float data: %f into address: 0x%04X", float_data, addr);
    ESP_ERROR_CHECK(eeprom_write_float_data(addr, &float_data));
    ESP_LOGI(MAIN_TAG, "");

    // Read float data from address 0x0300 - (768) - Page 6. and store result it into the buffer
    float result_float = 0.0;
    ESP_LOGI(EEPROM_TAG, "Read float data from address: 0x%04X", addr);
    ESP_ERROR_CHECK(eeprom_read_float_data(addr, &result_float));
    ESP_LOGI(MAIN_TAG, "Float data decimal: %f", result_float);
}