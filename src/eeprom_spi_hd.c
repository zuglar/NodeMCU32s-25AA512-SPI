/*
This code demonstrates how to use the SPI master half duplex mode
with 25AA512 EEPROM connected to the NodeMCU-32s.

This example code is in the Public Domain (or CC0 licensed, at your option.)
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.
 */

#include "eeprom_spi_hd.h"

esp_err_t eeprom_wp_hd_pins_init(void)
{
    esp_err_t result = ESP_OK;
    // Configure WP pin
    gpio_pad_select_gpio(NODEMCU32S_WP_PIN);
    if((result = gpio_set_direction(NODEMCU32S_WP_PIN, GPIO_MODE_OUTPUT)) != ESP_OK)
        return result;
    // set base level of wp pin
    nodemcu32s_wp_level = LOW;

    if((result = gpio_set_level(NODEMCU32S_WP_PIN, nodemcu32s_wp_level)) != ESP_OK)
        return result;

    // Configure HD pin
    gpio_pad_select_gpio(NODEMCU32S_HD_PIN);
    if((result = gpio_set_direction(NODEMCU32S_HD_PIN, GPIO_MODE_OUTPUT)) != ESP_OK)
        return result;
    // set base level of hd pin
    nodemcu32s_hd_level = HIGH;

    if((result = gpio_set_level(NODEMCU32S_HD_PIN, nodemcu32s_hd_level)) != ESP_OK)
        return result;

    return result;
}

esp_err_t init_spi(void)
{
    esp_err_t result = ESP_OK;

    spi_bus_config_t bus_cfg = {.miso_io_num = SPI_MISO_PIN,
                                .mosi_io_num = SPI_MOSI_PIN,
                                .sclk_io_num = SPI_SCLK_PIN,
                                .quadhd_io_num = SPI_QUAD_HD,
                                .quadwp_io_num = SPI_QUAD_WP,
                                .max_transfer_sz = SPI_MAX_TRANSFER_SZ,
                                //.flags = SPICOMMON_BUSFLAG_IOMUX_PINS,
                                .intr_flags = 0};
    result |= spi_bus_initialize(HOST, &bus_cfg, /* SPI_DMA_DISABLED */ SPI_DMA_CH_AUTO);
    if (result != ESP_OK)
        return result;
    // configuration eeprom device and add it to the bus
    spi_device_interface_config_t dev_cfg = {.address_bits = SPI_DEV_ADDR_BITS,
                                             .clock_speed_hz = SPI_DEV_CLK_FREQ,
                                             .command_bits = SPI_DEV_CMD_BITS,
                                             .flags = SPI_DEVICE_HALFDUPLEX,
                                             .mode = 0,
                                             .post_cb = cs_inactive,
                                             .pre_cb = cs_active,
                                             .queue_size = 1,
                                             .spics_io_num = SPI_SS_PIN};
    result |= spi_bus_add_device(HOST, &dev_cfg, &eeprom_handle);
    if (result != ESP_OK)
        return result;

    // eeprom is not in DPD mode
    dpd_mode = false;

    return result;
}

void cs_active(spi_transaction_t *tr)
{
    ESP_EARLY_LOGV(EEPROM_TAG, "ss pin %d low.", SPI_SS_PIN);
    // ESP_LOGI(EEPROM_TAG, "cs_active pin: %d - level: LOW", SPI_SS_PIN);
    gpio_set_level(SPI_SS_PIN, LOW);
}

void cs_inactive(spi_transaction_t *tr)
{
    gpio_set_level(SPI_SS_PIN, HIGH);
    ESP_EARLY_LOGV(EEPROM_TAG, "ss pin %d high.", SPI_SS_PIN);
    // ESP_LOGI(EEPROM_TAG, "cs_inactive pin: %d - level: HIGH", SPI_SS_PIN);
}

esp_err_t eeprom_set_wp_level(const uint32_t state)
{
    nodemcu32s_wp_level = state;
    return gpio_set_level(NODEMCU32S_WP_PIN, nodemcu32s_wp_level);
}

esp_err_t eeprom_set_hd_level(const uint32_t state)
{
    nodemcu32s_hd_level = state;
    return gpio_set_level(NODEMCU32S_HD_PIN, nodemcu32s_hd_level);
}

void log_buffer_hexdump(uint8_t *data)
{
    ESP_LOG_BUFFER_HEXDUMP(EEPROM_TAG,
                           data,
                           sizeof(data),
                           ESP_LOG_INFO);
}

uint8_t eeprom_is_busy(void)
{
    uint8_t status;
    vTaskDelay(1 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status));
    // check status register and WIP bit and return the result
    return (status & 0x01);
}

esp_err_t eeprom_read_status_reg(uint8_t *buffer)
{
    esp_err_t result = ESP_OK;

    memset(&main_transaction_ext, 0, sizeof(main_transaction_ext));
    main_transaction_ext.base.cmd = CMD_RDSR;
    main_transaction_ext.base.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_USE_RXDATA;
    main_transaction_ext.base.length = 8;
    main_transaction_ext.base.rxlength = 8;
    main_transaction_ext.address_bits = 0;

    result |= spi_device_polling_transmit(eeprom_handle, &main_transaction_ext.base);
    if (result != ESP_OK)
        return result;

    *buffer = main_transaction_ext.base.rx_data[0];
    return result;
}

esp_err_t eeprom_enable_wpen(bool status)
{
    esp_err_t result = ESP_OK;
    memset(&main_transaction_write, 0, sizeof(main_transaction_write));
    main_transaction_write.cmd = CMD_WRSR;
    main_transaction_write.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_USE_TXDATA;
    main_transaction_write.length = 8;
    if (status)
        main_transaction_write.tx_data[0] = 0x80;
    else
        main_transaction_write.tx_data[0] = 0x00;

    /* Set the write enable latch (enable write operations) */
    ESP_ERROR_CHECK(eeprom_send_simple_cmd(CMD_WREN));

    result |= spi_device_polling_transmit(eeprom_handle, &main_transaction_write);

    while (eeprom_is_busy())
    {
        ESP_LOGW(EEPROM_TAG, "EEPROM Write WPEN bit...");
    }

    if (result != ESP_OK)
        return result;

    return result;
}

bool eeprom_wpen_check(void)
{
    uint8_t status_reg;
    ESP_ERROR_CHECK(eeprom_read_status_reg(&status_reg));
    ESP_LOGI(EEPROM_TAG, "Status Register: 0x%02X", status_reg);

    if(status_reg & 0x80)
    {
        ESP_LOGI(EEPROM_TAG, "WPEN bit value: 1");
        return true;
    }

    ESP_LOGI(EEPROM_TAG, "WPEN bit value: 0");
    return false;
}

esp_err_t eeprom_write_protect_sector(eeprom_write_protect sector)
{
    esp_err_t result = ESP_OK;

    memset(&main_transaction_write, 0, sizeof(main_transaction_write));
    main_transaction_write.cmd = CMD_WRSR;
    main_transaction_write.flags = SPI_TRANS_VARIABLE_ADDR | SPI_TRANS_USE_TXDATA;
    main_transaction_write.length = 8;
    if (sector != PROTECT_NONE)
        main_transaction_write.tx_data[0] = (uint8_t)((sector << 2) | 0x80);
    else
        main_transaction_write.tx_data[0] = 0x00;

    // gpio_set_level(SPI_QUAD_WP, (uint32_t)HIGH);
    /* Set the write enable latch (enable write operations) */
    ESP_ERROR_CHECK(eeprom_send_simple_cmd(CMD_WREN));

    result |= spi_device_polling_transmit(eeprom_handle, &main_transaction_write);

    while (eeprom_is_busy())
    {
        ESP_LOGW(EEPROM_TAG, "EEPROM Write data into Status Register...");
    }

    if (result != ESP_OK)
        return result;

    // gpio_set_level(SPI_QUAD_WP, (uint32_t)LOW);
    return result;
}

esp_err_t eeprom_send_simple_cmd(const uint16_t command)
{
    memset(&main_transaction_ext, 0, sizeof(main_transaction_ext));
    main_transaction_ext.base.cmd = command;
    main_transaction_ext.base.flags = SPI_TRANS_VARIABLE_ADDR;
    main_transaction_ext.address_bits = 0;

    return (spi_device_polling_transmit(eeprom_handle, &main_transaction_ext.base));
}

esp_err_t eeprom_chip_erase(void)
{
    esp_err_t result = ESP_OK;
    ESP_LOGW(EEPROM_TAG, "EEPROM chip erasing started.");
    // Set the write enable latch (enable write operations)
    ESP_ERROR_CHECK(eeprom_send_simple_cmd(CMD_WREN));

    result |= eeprom_send_simple_cmd(CMD_CE);

    while (eeprom_is_busy())
    {
        ESP_LOGW(EEPROM_TAG, "EEPROM chip erasing ...");
    }

    if (result != ESP_OK)
        return result;

    ESP_LOGW(EEPROM_TAG, "EEPROM chip erasing finished.");

    return result;
}

esp_err_t eeprom_sector_erase(const uint16_t addr)
{
    if (addr > 0xFFFE)
        return ESP_ERR_INVALID_ARG;

    esp_err_t result = ESP_OK;
    ESP_LOGW(EEPROM_TAG, "EEPROM sector erasing started.");

    memset(&main_transaction_write, 0, sizeof(main_transaction_write));
    main_transaction_write.cmd = CMD_SE;
    main_transaction_write.addr = addr;

    // Set the write enable latch (enable write operations)
    ESP_ERROR_CHECK(eeprom_send_simple_cmd(CMD_WREN));

    result |= spi_device_polling_transmit(eeprom_handle, &main_transaction_write);

    if (result != ESP_OK)
        return result;

    while (eeprom_is_busy())
    {
        ESP_LOGW(EEPROM_TAG, "EEPROM sector erasing ...");
    }

    ESP_LOGW(EEPROM_TAG, "EEPROM sector erasing finished.");
    return result;
}

esp_err_t eeprom_page_erase(const uint16_t addr)
{
    if (addr > 0xFFFE)
        return ESP_ERR_INVALID_ARG;

    esp_err_t result = ESP_OK;
    ESP_LOGW(EEPROM_TAG, "EEPROM page erasing started.");

    memset(&main_transaction_write, 0, sizeof(main_transaction_write));
    main_transaction_write.cmd = CMD_PE;
    main_transaction_write.addr = addr;

    // Set the write enable latch (enable write operations)
    ESP_ERROR_CHECK(eeprom_send_simple_cmd(CMD_WREN));

    result |= spi_device_polling_transmit(eeprom_handle, &main_transaction_write);

    if (result != ESP_OK)
        return result;

    while (eeprom_is_busy())
    {
        ESP_LOGW(EEPROM_TAG, "EEPROM page erasing ...");
    }

    ESP_LOGW(EEPROM_TAG, "EEPROM page erasing finished.");
    return result;
}

esp_err_t eeprom_dpd_rdid_mode(const bool mode)
{
    esp_err_t result = ESP_OK;
    // check main state / mode of eeprom
    // if eeprom is not in DPD mode the eeprom can be set in DPD mode
    if (!dpd_mode && mode)
    {
        result = eeprom_send_simple_cmd(CMD_DPD);
    }
    // if eeprom is in DPD mode then the eeprom can be released from it
    else if (dpd_mode && !mode)
    {
        memset(&main_transaction_ext, 0, sizeof(main_transaction_ext));
        main_transaction_ext.base.cmd = CMD_RDID;
        main_transaction_ext.base.flags = SPI_TRANS_USE_RXDATA;
        main_transaction_ext.base.addr = 0x0000; // dummy address
        main_transaction_ext.base.length = 8;
        main_transaction_ext.base.rxlength = 8;

        result |= spi_device_polling_transmit(eeprom_handle, &main_transaction_ext.base);
    }
    else
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (result != ESP_OK)
        return result;

    // if mode is RDID the Manufacturer’s ID will be printed
    if (!mode)
        ESP_LOGI(EEPROM_TAG, "Manufacturer’s ID: 0x%02X", main_transaction_ext.base.rx_data[0]);

    // if the mode of eeprom changed successfully the main state / mode is changed
    dpd_mode = !dpd_mode;

    return result;
}

esp_err_t eeprom_address_data(const uint16_t addr, uint8_t *result_buffer)
{
    esp_err_t result = ESP_OK;

    memset(&main_transaction_ext, 0, sizeof(main_transaction_ext));
    main_transaction_ext.base.cmd = CMD_READ;
    main_transaction_ext.base.flags = SPI_TRANS_USE_RXDATA;
    main_transaction_ext.base.addr = addr;
    main_transaction_ext.base.length = 8 * 4;
    main_transaction_ext.base.rxlength = 8 * 4;

    result |= spi_device_polling_transmit(eeprom_handle, &main_transaction_ext.base);

    if (result != ESP_OK)
        return result;

    for (uint8_t i = 0; i < 4; i++)
        result_buffer[i] = main_transaction_ext.base.rx_data[i];

    return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// https://stackoverflow.com/questions/21384343/what-is-wrong-with-this-int-to-float-conversion-in-c/21384978#21384978
// https://gregstoll.com/~gregstoll/floattohex/
esp_err_t eeprom_write_float_data(const uint16_t addr, const float *data)
{
    esp_err_t result = ESP_OK;
    continuous_writing = true;

    eeprom_write_data(addr, uint32_t, (uint32_t *)data, 1, continuous_writing, result);

    return result;
}

esp_err_t eeprom_read_float_data(const uint16_t addr, float *buffer)
{
    // Allocate memory for buffer of reading (HEX data)
    uint8_t *read_float_data = (uint8_t *)malloc(4 * sizeof(uint8_t));
    if (read_float_data == NULL)
        return ESP_ERR_NO_MEM;

    esp_err_t result = ESP_OK;
    result |= eeprom_address_data(addr, read_float_data);

    if (result != ESP_OK)
        return result;

    log_buffer_hexdump(read_float_data);

    uint32_t rx_result = (uint32_t)((read_float_data[3] << 24) | (read_float_data[2] << 16) | (read_float_data[1] << 8) | read_float_data[0]);

    uint32_t signbit = (rx_result & 0x80000000) >> 31;
    uint32_t exponent = (rx_result & 0x7F800000) >> 23;
    uint32_t mantissa = (rx_result & 0x007FFFFF) | 0x00800000;

    // Freeing allocated memory
    if (read_float_data != NULL)
        free(read_float_data);

    *buffer = ((signbit == 1) ? -1.0 : 1.0) * mantissa / pow(2.0, (127 - exponent + 23));

    return result;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

esp_err_t eeprom_all_stored_data(void)
{
    esp_err_t result = ESP_OK;
    // Allocate memory for buffer of reading (HEX data) 16 bytes
    uint8_t *data_buffer_16 = malloc(16 * sizeof(uint8_t));

    if (data_buffer_16 == NULL)
        return ESP_ERR_NO_MEM;

    // Allocate memory for buffer of reading (HEX data) 4 bytes
    uint8_t *data_buffer_4 = malloc(4 * sizeof(uint8_t));
    if (data_buffer_4 == NULL)
        return ESP_ERR_NO_MEM;

    for (uint32_t i = 0; i < EEPROM_TOTAL_BYTES; i += EEPROM_PAGE_SIZE)
    {
        for (uint16_t j = 0; j < EEPROM_PAGE_SIZE; j += 16)
        {
            snprintf(ADDRESS_TAG, 16, "Address: 0x%04X", (uint16_t)(i + j));

            for (uint8_t k = 0; k < 16; k += 4)
            {
                result |= eeprom_address_data((uint16_t)(i + j + k), data_buffer_4);
                if (result != ESP_OK)
                {
                    result = ESP_ERR_INVALID_ARG;
                    break;
                }

                data_buffer_16[0 + k] = data_buffer_4[0];
                data_buffer_16[1 + k] = data_buffer_4[1];
                data_buffer_16[2 + k] = data_buffer_4[2];
                data_buffer_16[3 + k] = data_buffer_4[3];

                vTaskDelay(pdMS_TO_TICKS(10));
            }
            ESP_LOG_BUFFER_HEXDUMP(ADDRESS_TAG, data_buffer_16, 16, ESP_LOG_INFO);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Freeing allocated memory - buffer 16 bytes
    if (data_buffer_16 != NULL)
        free(data_buffer_16);

    // Freeing allocated memory - buffer 4 bytes
    if (data_buffer_4 != NULL)
        free(data_buffer_4);

    return result;
}

esp_err_t eeprom_page_stored_data(const uint16_t page)
{
    if (page > 511)
        return ESP_ERR_INVALID_ARG;

    esp_err_t result = ESP_OK;
    // Allocate memory for buffer of reading (HEX data) 16 bytes
    uint8_t *data_buffer_16 = malloc(16 * sizeof(uint8_t));

    if (data_buffer_16 == NULL)
        return ESP_ERR_NO_MEM;

    // Allocate memory for buffer of reading (HEX data) 4 bytes
    uint8_t *data_buffer_4 = malloc(4 * sizeof(uint8_t));
    if (data_buffer_4 == NULL)
        return ESP_ERR_NO_MEM;

    // Start address of page
    uint16_t address = page * EEPROM_PAGE_SIZE;

    for (uint16_t j = 0; j < EEPROM_PAGE_SIZE; j += 16)
    {
        snprintf(ADDRESS_TAG, 16, "Address: 0x%04X", (j + address));

        for (uint8_t k = 0; k < 16; k += 4)
        {
            result |= eeprom_address_data((uint16_t)(j + k + address), data_buffer_4);
            if (result != ESP_OK)
            {
                result = ESP_ERR_INVALID_ARG;
                break;
            }

            data_buffer_16[0 + k] = data_buffer_4[0];
            data_buffer_16[1 + k] = data_buffer_4[1];
            data_buffer_16[2 + k] = data_buffer_4[2];
            data_buffer_16[3 + k] = data_buffer_4[3];

            vTaskDelay(pdMS_TO_TICKS(10));
        }
        ESP_LOG_BUFFER_HEXDUMP(ADDRESS_TAG, data_buffer_16, 16, ESP_LOG_INFO);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Freeing allocated memory - buffer 16 bytes
    if (data_buffer_16 != NULL)
        free(data_buffer_16);

    // Freeing allocated memory - buffer 4 bytes
    if (data_buffer_4 != NULL)
        free(data_buffer_4);

    return result;
}
