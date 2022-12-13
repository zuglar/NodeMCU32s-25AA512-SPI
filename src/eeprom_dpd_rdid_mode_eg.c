// Example to set eeprom into DEEP POWER-DOWN mode and RELEASE eeprom from DEEP POWER-DOWN AND read ELECTRONIC SIGNATURE

#include "eeprom_spi_hd.h"

void eeprom_dpd_rdid_mode_eg(void)
{
    esp_err_t result_ = ESP_OK;

    // Writes data into the address 0xF280 - (62080).
    addr = 0xF280;
    write_data3 = 666;
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data %d into address: 0x%04X", write_data3, addr);
    eeprom_write_data(addr, uint16_t, &write_data3, 1, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // Set eeprom into DPD mode
    ESP_LOGI(MAIN_TAG, "Set eeprom into DPD mode");
    ESP_ERROR_CHECK(eeprom_dpd_rdid_mode(true));
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0xF280 - (62080).
    ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
    eeprom_read_data(addr, uint16_t, result_uint16, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
    ESP_LOGI(MAIN_TAG, "");

    // RELEASE eeprom from DEEP POWER-DOWN AND read ELECTRONIC SIGNATURE
    ESP_LOGI(MAIN_TAG, "RELEASE eeprom from DEEP POWER-DOWN AND read ELECTRONIC SIGNATURE");
    ESP_ERROR_CHECK(eeprom_dpd_rdid_mode(false));
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0xF280 - (62080).
    ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
    eeprom_read_data(addr, uint16_t, result_uint16, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
}