// Example to erase all stored data in the eeprom.

#include "eeprom_spi_hd.h"

void eeprom_chip_erase_eg(void)
{
    // Erases all data in eeprom.
    ESP_ERROR_CHECK(eeprom_chip_erase());
    ESP_LOGI(MAIN_TAG, "");

     // Checks data in the erased eeprom.
     ESP_ERROR_CHECK(eeprom_all_stored_data());
}