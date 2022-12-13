/* *
 * Example for testing the function of WP pin of eeprom.
 *
 * This example works correctly if:
 * - the NODEMCU32S_WP_PIN in the "eeprom_spi_hd.h" header file is defined,
 * - the NodeMCU32s and 25AA512 eeprom have been connected correctly.
 *
 * In other cases you can get errors.
 *
 * WRITE-PROTECT FUNCTIONALITY MATRIX
 * |------------|------------|---------|--------------------|-----------------------|--------------------|
 * |     WEL    |    WPEN    |    WP   |   Protected Bloks  |   Unprotected Blocks  |   STATUS Register  |
 * | (SR bit 1) | (SR bit 7) | (Pin 3) |                    |                       |                    |
 * |------------|------------|---------|--------------------|-----------------------|--------------------|
 * |      0     |     x      |    x    |     Protected      |       Protected       |      Protected     |
 * |      1     |     0      |    x    |     Protected      |       Writable        |      Writable      |
 * |      1     |     1      | 0 (low) |     Protected      |       Writable        |      Protected     |
 * |      1     |     1      | 1 (high)|     Protected      |       Writable        |      Writable      |
 * |------------|------------|---------|--------------------|-----------------------|--------------------|
 *
 */

#include "eeprom_spi_hd.h"

void eeprom_wp_pin_eg(void)
{
    // Erases all data in eeprom.
    ESP_ERROR_CHECK(eeprom_chip_erase());
    ESP_LOGI(MAIN_TAG, "");
    
    uint8_t status_reg;
    esp_err_t result_ = ESP_OK;

    ESP_LOGI(MAIN_TAG, "Example for testing the function of WP pin of eeprom");

    ESP_LOGI(MAIN_TAG, "Check WPEN bit of EEPROM Status Register and WP pin level");
    eeprom_wpen_check();
    ESP_LOGI(MAIN_TAG, "WP pin level: %s", nodemcu32s_wp_level ? "HIGH" : "LOW");

    ESP_LOGI(MAIN_TAG, "");
    ESP_LOGI(MAIN_TAG, "We can write data into eeprom and read it from eeprom.");

    // Writes data into the address 0xFF80 - (65408).
    addr = 0xFF80;
    write_data3 = 55555;
    continuous_writing = false;
    ESP_LOGI(EEPROM_TAG, "Write data %d into address: 0x%04X", write_data3, addr);
    eeprom_write_data(addr, uint16_t, &write_data3, 1, continuous_writing, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "");

    // Read data from address 0xFF80 - (65408).
    ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
    eeprom_read_data(addr, uint16_t, result_uint16, result_);
    ESP_ERROR_CHECK(result_);
    ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
    ESP_LOGI(MAIN_TAG, "");

    ESP_LOGI(MAIN_TAG, "We can write data into Status Register of eeprom.");
    ESP_ERROR_CHECK(eeprom_enable_wpen(true));
    ESP_LOGI(MAIN_TAG, "Check WPEN bit of EEPROM Status Register and WP pin level");
    bool wpen_bit = eeprom_wpen_check();
    ESP_LOGI(MAIN_TAG, "");

    ESP_LOGW(MAIN_TAG, "Status Register of eeprom has been changed.");
    ESP_LOGW(MAIN_TAG, "--- New state ---");
    ESP_LOGW(MAIN_TAG, "WPEN bit: %d - WP pin level: %s", wpen_bit, nodemcu32s_wp_level ? "HIGH" : "LOW");
    ESP_LOGI(MAIN_TAG, "");

    // Change Status Register of eeprom by Enabling Write Protect for all sectors of eeprom.
    ESP_LOGI(EEPROM_TAG, "Change Status Register of eeprom by Enabling Write Protect for all sectors of eeprom.");
    ESP_ERROR_CHECK(eeprom_write_protect_sector(PROTECT_ALL));
    ESP_LOGI(MAIN_TAG, "");

    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGE(MAIN_TAG, "Status Register of eeprom hasn't been changed - 0x%02X", status_reg);
    ESP_LOGI(MAIN_TAG, "");

    // If the NODEMCU32S_WP_PIN configured we have to set level of NODEMCU32S_WP_PIN to HIGH
#if defined(NODEMCU32S_WP_PIN) && defined(NODEMCU32S_HD_PIN)
    if (!nodemcu32s_wp_level)
    {
        esp_err_t _result = ESP_OK;
        _result = eeprom_set_wp_level((uint32_t)HIGH);
        ESP_ERROR_CHECK(_result);
        ESP_LOGI(MAIN_TAG, "WP pin level: %s", nodemcu32s_wp_level ? "HIGH" : "LOW");
        ESP_LOGI(MAIN_TAG, "");
    }
#endif

    // Change Status Register of eeprom by Enabling Write Protect for all sectors of eeprom.
    ESP_LOGI(EEPROM_TAG, "Change Status Register of eeprom by Enabling Write Protect for all sectors of eeprom.");
    ESP_ERROR_CHECK(eeprom_write_protect_sector(PROTECT_ALL));
    ESP_LOGI(MAIN_TAG, "");

    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGW(MAIN_TAG, "Status Register of eeprom has been changed - 0x%02X", status_reg);
    ESP_LOGI(MAIN_TAG, "");

    // Disabling Write Protect for all sectors of eeprom.
    ESP_LOGI(EEPROM_TAG, "Disabling Write Protect for all sectors of eeprom");
    ESP_ERROR_CHECK(eeprom_write_protect_sector(PROTECT_NONE));
    ESP_LOGI(MAIN_TAG, "");

    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGW(MAIN_TAG, "Status Register of eeprom has been changed - 0x%02X", status_reg);
    ESP_LOGI(MAIN_TAG, "");

    // Disabling WPEN bit (set to 0) of Status Register
    ESP_LOGI(MAIN_TAG, "Disabling WPEN bit (set to 0) of Status Register");
    ESP_ERROR_CHECK(eeprom_enable_wpen(false));
    ESP_LOGI(MAIN_TAG, "");

// If the NODEMCU32S_WP_PIN configured we have to set level of NODEMCU32S_WP_PIN to LOW
#if defined(NODEMCU32S_WP_PIN) && defined(NODEMCU32S_HD_PIN)
    if (nodemcu32s_wp_level)
    {
        esp_err_t result = ESP_OK;
        result = eeprom_set_wp_level((uint32_t)LOW);
        ESP_ERROR_CHECK(result);
        ESP_LOGI(MAIN_TAG, "WP pin level: %s", nodemcu32s_wp_level ? "HIGH" : "LOW");
        ESP_LOGI(MAIN_TAG, "");
    }
#endif

    ESP_LOGI(MAIN_TAG, "Check WPEN bit of EEPROM Status Register and WP pin level");
    wpen_bit = eeprom_wpen_check();
    ESP_LOGI(MAIN_TAG, "");

    ESP_LOGW(MAIN_TAG, "Status Register of eeprom has been changed.");
    ESP_LOGW(MAIN_TAG, "--- New state ---");
    ESP_LOGW(MAIN_TAG, "WPEN bit: %d - WP pin level: %s", wpen_bit, nodemcu32s_wp_level ? "HIGH" : "LOW");
    ESP_LOGI(MAIN_TAG, "");
}