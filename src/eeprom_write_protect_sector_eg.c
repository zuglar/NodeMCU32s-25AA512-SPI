// Example to write protection of sector of eeprom.

#include "eeprom_spi_hd.h"

void eeprom_write_protect_sector_eg(void)
{
    esp_err_t result_ = ESP_OK;

    // Writes data into the address 0x0781 - (1921) - Page 15.
    addr = 0x0781;
    write_data3 = 11111;
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data %d into address: 0x%04X, Page 15.", write_data3, addr);
    eeprom_write_data(addr, uint16_t, &write_data3, 1, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0x0781 - (10) - Page 15
    ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
    eeprom_read_data(addr, uint16_t, result_uint16, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
    ESP_LOGI(MAIN_TAG, "");

    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    uint8_t status_reg;
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);

    // Enabling Write Protect for all sectors of eeprom.
    ESP_LOGI(EEPROM_TAG, "Enabling Write Protect for all sectors of eeprom.");
    ESP_ERROR_CHECK(eeprom_write_protect_sector(PROTECT_ALL));

    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);

    // Writes data into the address 0x0781 - (1921) - Page 15.
    write_data3 = 55555;
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data %d into address: 0x%04X, Page 15.", write_data3, addr);
    eeprom_write_data(addr, uint16_t, &write_data3, 1, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0x0781 - (10) - Page 15
    ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
    eeprom_read_data(addr, uint16_t, result_uint16, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGW(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
    ESP_LOGI(MAIN_TAG, "");

    // Disabling Write Protect for all sectors of eeprom.
    ESP_LOGI(EEPROM_TAG, "Disabling Write Protect for all sectors of eeprom");
    ESP_ERROR_CHECK(eeprom_write_protect_sector(PROTECT_NONE));

    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);

    // Writes data into the address 0x0781 - (1921) - Page 15.
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data %d into address: 0x%04X, Page 15.", write_data3, addr);
    eeprom_write_data(addr, uint16_t, &write_data3, 1, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0x0781 - (10) - Page 15
    ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
    eeprom_read_data(addr, uint16_t, result_uint16, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
}