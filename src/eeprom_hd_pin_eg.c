/* *
 * Example for testing the function of HD (HOLD) pin of eeprom.
 *
 * This example works correctly if:
 * - the NODEMCU32S_HD_PIN in the "eeprom_spi_hd.h" header file is defined,
 * - the NodeMCU32s and 25AA512 eeprom have been connected correctly.
 *
 * In other cases you can get errors.
 *
 */

#include "eeprom_spi_hd.h"

void eeprom_hd_pin_eg(void)
{

    ESP_LOGI(MAIN_TAG, "Example for testing the function of HD (HOLD) pin of eeprom");
    ESP_LOGI(MAIN_TAG, "");
    esp_err_t result_ = ESP_OK;
    bool hold_enabled = false;
    uint8_t array_size = 35;
    uint8_t array_pos = 0;
    addr = 0xF000;
    uint16_t addr_pos = 0;

    // Erases all data in eeprom.
    ESP_ERROR_CHECK(eeprom_chip_erase());
    ESP_LOGI(MAIN_TAG, "");
    
    // Allocate memory for data 
    uint16_t *data = malloc(array_size * sizeof(uint16_t));

    if (data == NULL)
    {
        ESP_LOGE(MAIN_TAG, "Memory allocation error occurred.");
        return;
    }
    
    // Write data to eeprom
    for (uint8_t i = 0; i < array_size;)
    {
        data[i] = i * 100 + 10000;

        if (!hold_enabled)
        {
            // data[i] = i + 10000;
            ESP_LOGI(EEPROM_TAG, "Write data %d into address: 0x%04X", data[i], addr);
            eeprom_write_data(addr, uint16_t, &data[i], 1, true, result_);
            ESP_ERROR_CHECK(result_);
            addr += sizeof(uint16_t);
        }

        if (i == 20)
        {
            // Enabling HOLD pin - Set level to LOW
            // If the NODEMCU32S_HD_PIN configured we have to set level of NODEMCU32S_HD_PIN to LOW
#if defined(NODEMCU32S_WP_PIN) && defined(NODEMCU32S_HD_PIN)
            if (nodemcu32s_hd_level)
            {
                esp_err_t result = ESP_OK;
                result = eeprom_set_hd_level((uint32_t)LOW);
                ESP_ERROR_CHECK(result);
                ESP_LOGI(MAIN_TAG, "HOLD pin level: %s", nodemcu32s_hd_level ? "HIGH" : "LOW");
                array_pos = i;
                hold_enabled = true;
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
#endif
        }

        if (i == 34)
        {
            // Disabling HOLD pin - Set level to HIGH
            // If the NODEMCU32S_HD_PIN configured we have to set level of NODEMCU32S_HD_PIN to HIGH
#if defined(NODEMCU32S_WP_PIN) && defined(NODEMCU32S_HD_PIN)
            if (!nodemcu32s_hd_level)
            {
                esp_err_t result = ESP_OK;
                result = eeprom_set_hd_level((uint32_t)HIGH);
                ESP_ERROR_CHECK(result);
                ESP_LOGI(MAIN_TAG, "HOLD pin level: %s", nodemcu32s_hd_level ? "HIGH" : "LOW");
                i = array_pos;
                hold_enabled = false;
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
#endif
        }
        i++;
    }

    // Freeing allocated memory
    if (data != NULL)
        free(data);

    ESP_LOGI(MAIN_TAG, "");

    // Read data from eeprom
    addr = 0xF000;

    for (uint8_t i = 0; i < array_size; i++)
    {
        if (!hold_enabled)
        {
            ESP_LOGI(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
            eeprom_read_data(addr, uint16_t, result_uint16, result_);
            ESP_ERROR_CHECK(result_);
            ESP_LOGI(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
            addr += sizeof(uint16_t);
        }
        else if (hold_enabled)
        {
            ESP_LOGE(EEPROM_TAG, "Read data from address: 0x%04X ", addr);
            eeprom_read_data(addr, uint16_t, result_uint16, result_);
            ESP_ERROR_CHECK(result_);
            ESP_LOGE(MAIN_TAG, "Data in address:  0x%04X = %d", addr, result_uint16);
            addr += sizeof(uint16_t);
        }

        if (i == 10)
        {
            // Enabling HOLD pin - Set level to LOW
            // If the NODEMCU32S_HD_PIN configured we have to set level of NODEMCU32S_HD_PIN to LOW
#if defined(NODEMCU32S_WP_PIN) && defined(NODEMCU32S_HD_PIN)
            if (nodemcu32s_hd_level)
            {
                esp_err_t result = ESP_OK;
                result = eeprom_set_hd_level((uint32_t)LOW);
                ESP_ERROR_CHECK(result);
                ESP_LOGI(MAIN_TAG, "HOLD pin level: %s", nodemcu32s_hd_level ? "HIGH" : "LOW");
                array_pos = i;
                addr_pos = addr;
                hold_enabled = true;
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
#endif
        }

        if (i == 20)
        {
            // Disabling HOLD pin - Set level to HIGH
            // If the NODEMCU32S_HD_PIN configured we have to set level of NODEMCU32S_HD_PIN to HIGH
#if defined(NODEMCU32S_WP_PIN) && defined(NODEMCU32S_HD_PIN)
            if (!nodemcu32s_hd_level)
            {
                esp_err_t result = ESP_OK;
                result = eeprom_set_hd_level((uint32_t)HIGH);
                ESP_ERROR_CHECK(result);
                ESP_LOGI(MAIN_TAG, "HOLD pin level: %s", nodemcu32s_hd_level ? "HIGH" : "LOW");
                i = array_pos;
                addr = addr_pos;
                hold_enabled = false;
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
#endif
        }
    }
}