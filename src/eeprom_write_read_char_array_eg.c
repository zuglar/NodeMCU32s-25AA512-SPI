// Example to write and read character array data.

#include "eeprom_spi_hd.h"

void eeprom_write_read_char_array_eg(void)
{
    esp_err_t result_ = ESP_OK;

    // Erases all data in eeprom.
    ESP_ERROR_CHECK(eeprom_chip_erase());
    ESP_LOGI(MAIN_TAG, "");
    
    // Write character array into the address 0x0571 - (1393) - Page 10 (overlap between pages Page 10 and Page 11).
    addr = 0x0571;
    continuous_writing = false;
    const char ch_arr[] = "NodeMCU32s and 25AA512 EEPROM ESP-IDF SPI example!";
    size_t ch_arr_length = sizeof(ch_arr) / sizeof(ch_arr[0]);
    ESP_LOGI(MAIN_TAG, "Write: %s", ch_arr);
    ESP_LOGI(MAIN_TAG, "Write character array into the start address: 0x%04X, Page 10 (overlap between pages Page 10 and Page 11).", addr);
    eeprom_write_data(addr, char, ch_arr, ch_arr_length, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // // Read data from address 0x0570 - (1392)
    // // Allocate memory for buffer of reading
    ch_arr_result = malloc(ch_arr_length * sizeof(char));
    if (ch_arr_result == NULL)
        return;
    
    for (size_t i = 0; i < ch_arr_length; i++)
    {
        eeprom_read_data(addr, char, *(ch_arr_result + i), result_);
        ESP_ERROR_CHECK(result_);
        addr++;
    }
    
    ESP_LOGI(MAIN_TAG, "Read: %s", ch_arr_result);

    ESP_LOGI(MAIN_TAG, "");

    // Continuous write character array into the address 0x0571 - (1393) - Page 10 (overlap between pages Page 10 and Page 11).
    addr = 0x0571;
    continuous_writing = true;
    ESP_LOGI(MAIN_TAG, "Continuous write character array into the start address: 0x%04X, Page 10 (overlap between pages Page 10 and Page 11).", addr);
    ESP_LOGI(MAIN_TAG, "Write: %s", ch_arr);
    eeprom_write_data(addr, char, ch_arr, ch_arr_length, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);

    for (uint8_t i = 0; i < ch_arr_length; i++)
    {
        eeprom_read_data(addr, char, *(ch_arr_result + i), result_);
        ESP_ERROR_CHECK(result_);
        addr++;
    }
    ESP_LOGI(MAIN_TAG, "");
    ESP_LOGI(MAIN_TAG, "Read: %s", ch_arr_result);
}