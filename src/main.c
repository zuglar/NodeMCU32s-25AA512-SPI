#include "eeprom_spi_hd.h"

void app_main()
{
    /* !!!!!!!!!! Don't modify the code in this section.  !!!!!!!!!! */
    /* !!!!!!!!!!!!!!!!!!!! START SECTION  !!!!!!!!!!!!!!!!!!!! */
    uint8_t status_reg;

    ESP_LOGI(MAIN_TAG, "**************************************************************");
    ESP_LOGI(MAIN_TAG, "********************* BASE CHECK STARTED *********************");

#if defined(NODEMCU32S_WP_PIN) && defined(NODEMCU32S_HD_PIN)
    // Configure NodeMCU32s GPIO pins for WP and HD control
    ESP_LOGI(MAIN_TAG, "Configure NodeMCU32s GPIO pins for WP and HD control.");
    ESP_ERROR_CHECK(eeprom_wp_hd_pins_init());
#else
    nodemcu32s_wp_level = HIGH;
    nodemcu32s_hd_level = HIGH;
#endif

    ESP_LOGI(MAIN_TAG, "");
    // Initialize the SPI bus and add device (EEPROM) to the bus.
    ESP_LOGI(MAIN_TAG, "Initialize the SPI bus and add device (EEPROM) to the bus.");
    ESP_ERROR_CHECK(init_spi());
    ESP_LOGI(MAIN_TAG, "");

    // To use examples below we have to check WPEN status bit in status register of eeprom
    ESP_LOGI(MAIN_TAG, "Check WPEN bit of EEPROM Status Register");
    if (eeprom_wpen_check())
    {
        // If WPEN is 1 we have to disbale it and clear bits of sectors protection

        // If the NODEMCU32S_WP_PIN configured we have to set level of NODEMCU32S_WP_PIN to HIGH
#if defined(NODEMCU32S_WP_PIN) && defined(NODEMCU32S_HD_PIN)
        if (!nodemcu32s_wp_level)
        {
            esp_err_t result = ESP_OK;
            result = eeprom_set_wp_level((uint32_t)HIGH);
            ESP_ERROR_CHECK(result);
            ESP_LOGI(MAIN_TAG, "WP pin level: %s", nodemcu32s_wp_level ? "HIGH" : "LOW");
        }
#endif

        // Disabling WPEN bit (set to 0) of Status Register
        ESP_LOGI(MAIN_TAG, "Disabling WPEN bit (set to 0) of Status Register");
        ESP_ERROR_CHECK(eeprom_enable_wpen(false));
        // Disabling Write Protect for all sectors of eeprom.
        ESP_LOGI(EEPROM_TAG, "Disabling Write Protect for all sectors of eeprom");
        ESP_ERROR_CHECK(eeprom_write_protect_sector(PROTECT_NONE));

        // Read STATUS register.
        ESP_LOGI(EEPROM_TAG, "Read Status Register");
        ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
        ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);

        // If the NODEMCU32S_WP_PIN configured we have to set level of NODEMCU32S_WP_PIN to LOW
#if defined(NODEMCU32S_WP_PIN) && defined(NODEMCU32S_HD_PIN)
        if (nodemcu32s_wp_level)
        {
            esp_err_t result = ESP_OK;
            result = eeprom_set_wp_level((uint32_t)LOW);
            ESP_ERROR_CHECK(result);
            ESP_LOGI(MAIN_TAG, "WP pin level: %s", nodemcu32s_wp_level ? "HIGH" : "LOW");
        }
#endif
    }
    ESP_LOGI(MAIN_TAG, "********************* BASE CHECK FINISHED ********************");
    ESP_LOGI(MAIN_TAG, "");
    /* !!!!!!!!!!!!!!!!!!!! END SECTION  !!!!!!!!!!!!!!!!!!!! */

    // Example to print out the page number with start and end addresses.
    // eeprom_pages_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // Print out the all stored data in the eeprom.
    // eeprom_all_stored_data();
    // ESP_LOGI(MAIN_TAG, "");

    // Print out the stored data in the page.
    // eeprom_page_stored_data(10);
    // ESP_LOGI(MAIN_TAG, "");

    // Example to write and read the status register of eeprom.
    // eeprom_status_register_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // Example to write and read 8 bits data.
    // eeprom_write_read_8bits_data_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // Example to write and read 16 bits data.
    // eeprom_write_read_16bits_data_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // Example to write and read 32 bits data.
    // eeprom_write_read_32bits_data_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // Example to write and read character array data.
    eeprom_write_read_char_array_eg();
    ESP_LOGI(MAIN_TAG, "");

    // Example to write and read float number.
    eeprom_write_read_float_data_eg();
    ESP_LOGI(MAIN_TAG, "");

    // Example to erase page of eeprom with specified address.
    // eeprom_page_erase_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // Example to erase all stored data in the eeprom.
    // eeprom_chip_erase_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // Example to write WPEN bit of Status Register of eeprom.
    // eeprom_enable_wpen_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // // Example to write protection of sector of eeprom.
    // eeprom_write_protect_sector_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // Example to erase sector of eeprom with specified address.
    // eeprom_sector_erase_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // Example to set eeprom into DEEP POWER-DOWN mode and RELEASE eeprom from DEEP POWER-DOWN AND read ELECTRONIC SIGNATURE
    // eeprom_dpd_rdid_mode_eg();
    // ESP_LOGI(MAIN_TAG, "");

    // For using this example read the comment in the "eeprom_wp_pin_eg.c" file
    // Example for testing the function of WP pin of eeprom
    // eeprom_wp_pin_eg();

    // For using this example read the comment in the "eeprom_hd_pin_eg.c" file
    // Example for testing the function of HD (HOLD) pin of eeprom
    // eeprom_hd_pin_eg();

    // loop
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}