// Example to write and read the status register of eeprom.

#include "eeprom_spi_hd.h"

void eeprom_status_register_eg(void)
{
    // Read STATUS register.
    ESP_LOGI(EEPROM_TAG, "Read Status Register");
    uint8_t status_reg;
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);

    ESP_LOGI(MAIN_TAG, "");
    // Set the write enable latch (enable write operations)
    ESP_LOGI(EEPROM_TAG, "Write Enable - Write Enable Latch (WEL) set to a 1, the latch allows writes to the array.");
    ESP_ERROR_CHECK(eeprom_send_simple_cmd(CMD_WREN));

    ESP_LOGI(MAIN_TAG, "");
    // Check STATUS register.
    ESP_LOGI(EEPROM_TAG, "Check Status Register");
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);

    ESP_LOGI(MAIN_TAG, "");
    // Reset the write enable latch (disable write operations).
    ESP_LOGI(EEPROM_TAG, "Write Disable - Write Enable Latch (WEL) set to a 0.");
    ESP_ERROR_CHECK(eeprom_send_simple_cmd(CMD_WRDI));

    ESP_LOGI(MAIN_TAG, "");
    // Check STATUS register.
    ESP_LOGI(EEPROM_TAG, "Check Status Register");
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(MAIN_TAG, "Status Register: 0x%02X", status_reg);
}
