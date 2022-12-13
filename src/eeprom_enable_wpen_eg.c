// Example to write WPEN bit of Status Register of eeprom.

#include "eeprom_spi_hd.h"

void eeprom_enable_wpen_eg(void)
{
    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    uint8_t status_reg;
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);
    ESP_LOGI(MAIN_TAG, "");

    // Enabling WPEN bit (set to 1) of Status Register
    ESP_LOGI(MAIN_TAG, "Enabling WPEN bit (set to 1) of Status Register");
    ESP_ERROR_CHECK(eeprom_enable_wpen(true));

    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);
    ESP_LOGI(MAIN_TAG, "");

    // Disabling WPEN bit (set to 0) of Status Register
    ESP_LOGI(MAIN_TAG, "Disabling WPEN bit (set to 0) of Status Register");
    ESP_ERROR_CHECK(eeprom_enable_wpen(false));
    
    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);
}