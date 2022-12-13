/*
This code demonstrates how to use the SPI master half duplex mode
with 25AA512 EEPROM connected to the NodeMCU-32s.

This example code is in the Public Domain (or CC0 licensed, at your option.)
Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>

#include "string.h"
#include "math.h"

// define VSPI HOST and PINs
#define SPI_MISO_PIN GPIO_NUM_19 // NodeMCU32s MISO gpio
#define SPI_MOSI_PIN GPIO_NUM_23 // NodeMCU32s MOSI gpio
#define SPI_SCLK_PIN GPIO_NUM_18 // NodeMCU32s CLK gpio
#define SPI_SS_PIN GPIO_NUM_5    // NodeMCU32s SS gpio
#define HOST SPI2_HOST           // NodeMCU32s HOST (SPI2_HOST)
#define SPI_QUAD_HD GPIO_NUM_NC
#define SPI_QUAD_WP GPIO_NUM_NC
#define SPI_MAX_TRANSFER_SZ 32
// define divider of master clock frequency, address and command bits length
#define SPI_DEV_CLK_FREQ SPI_MASTER_FREQ_8M // SPI master clock is divided by 80MHz apb clock.
#define SPI_DEV_ADDR_BITS 16                // eeprom address bits length
#define SPI_DEV_CMD_BITS 8                  // eeprom commands bits length

// define NodeMCU32s GPIO pins to controll Write Protect (WP) and HOLD (HD) pins of eeprom
// If you don't want to control WP and HOLD pins of eeprom by NodeMCU32s comments out these two lines below
#define NODEMCU32S_WP_PIN GPIO_NUM_2 // NodeMCU32s GPIO_NUM_2 pin is connected to the WP (Write-Protect) pin of eeprom
#define NODEMCU32S_HD_PIN GPIO_NUM_4 // NodeMCU32s GPIO_NUM_4 pin is connected to the HOLD pin of eeprom

// define EEPROM page size and total bytes
#define EEPROM_PAGE_SIZE 128
#define EEPROM_TOTAL_BYTES 65536 // 512Kbit * 1024 / 8 = 65536 Bytes

// define EEPROM commands
#define CMD_READ 0b00000011  // Read data from memory array beginning at selected address
#define CMD_WRITE 0b00000010 // Write data to memory array beginning at selected address
#define CMD_WREN 0b00000110  // Set the write enable latch (enable write operations)
#define CMD_WRDI 0b00000100  // Reset the write enable latch (disable write operations)
#define CMD_RDSR 0b00000101  // Read STATUS register
#define CMD_WRSR 0b00000001  // The Write Status Register instruction (WRSR) allows the user to write to the nonvolatile bits in the STATUS register
#define CMD_PE 0b01000010    // Page Erase – erase one page in memory array
#define CMD_SE 0b11011000    // Sector Erase – erase one sector in memory array
#define CMD_CE 0b11000111    // Chip Erase – erase all sectors in memory array
#define CMD_RDID 0b10101011  // Release from Deep power-down and read electronic signature
#define CMD_DPD 0b10111001   // Deep Power-Down mode

typedef enum EEPROM_WRITE_PROTECT
{                       // Status register bit                          BP1 |  BP0
    PROTECT_NONE,       //                                               0  |   0
    PROTECT_SECTOR_3,   // Upper 1/4 (Sector 3) (C000h-FFFFh)            0  |   1
    PROTECT_SECTOR_2_3, // Upper 1/2 (Sectors 2 & 3) (8000h-FFFFh)       1  |   0
    PROTECT_ALL         // All (Sectors 0, 1, 2 & 3) (0000h-FFFFh)       1  |   1
} eeprom_write_protect;

// define GPIO PIN level
#define HIGH 1
#define LOW 0

#define ARRAY_SIZE 3

// variables
bool continuous_writing;
int8_t write_data[ARRAY_SIZE];
uint16_t addr;
int8_t result_int8;
uint32_t write_data2[ARRAY_SIZE];
uint32_t result_uint32;
uint16_t write_data3;
uint16_t result_uint16;
char *ch_arr_result;
bool dpd_mode;                // variable to store main state / mode of eeprom
uint32_t nodemcu32s_wp_level; // variable to store wp pin level of NodeMCU32s
uint32_t nodemcu32s_hd_level; // variable to store hd pin level of NodeMCU32s

// constans
static const char MAIN_TAG[] = "main";
static const char EEPROM_TAG[] = "eeprom";
static char ADDRESS_TAG[16];

/**
 * @brief Template of writing data into the address of page.
 *
 * @param addr Address.
 * @param data_type Type of data - number of bits (8, 16, 32) to write. (ex. int8_t, uint16_t, int32_t, ...).
 * @param data_buffer Data to write. Type of data has to be the same like data_type argument.
 * @param data_size Size of data to write.
 * @param continuous TRUE - data will be written continuously. FALSE - data will be written into page.
 * @param tr_result Result of transaction.
 * @return tr_result
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
#define eeprom_write_data(addr, data_type, data_buffer, data_size, continuous, tr_result)          \
    {                                                                                              \
        tr_result = ESP_OK;                                                                        \
        uint8_t sizeof_data_type = (uint8_t)sizeof(data_type);                                     \
        if (sizeof_data_type == 1 || sizeof_data_type == 2 || sizeof_data_type == 4)               \
        {                                                                                          \
            uint16_t arr_index = 0;                                                                \
            uint16_t address = addr;                                                               \
            uint16_t chunk = data_size;                                                            \
            uint16_t position_in_page = address % EEPROM_PAGE_SIZE;                                \
            uint16_t space_left_in_page = EEPROM_PAGE_SIZE - position_in_page;                     \
            if (!continuous)                                                                       \
            {                                                                                      \
                if (space_left_in_page < chunk)                                                    \
                    chunk = space_left_in_page;                                                    \
            }                                                                                      \
                                                                                                   \
            memset(&main_transaction_write, 0, sizeof(main_transaction_write));                    \
            main_transaction_write.cmd = CMD_WRITE;                                                \
                                                                                                   \
            while ((chunk != 0))                                                                   \
            {                                                                                      \
                main_transaction_write.addr = address;                                             \
                                                                                                   \
                /*  write 8 bits of input data */                                                  \
                if (sizeof_data_type == 1)                                                         \
                {                                                                                  \
                    main_transaction_write.flags = SPI_TRANS_USE_TXDATA;                           \
                    main_transaction_write.length = 8 * sizeof_data_type;                          \
                    main_transaction_write.tx_data[0] = *(data_buffer + arr_index);                \
                    address++;                                                                     \
                }                                                                                  \
                /*  write 16 or 32 bits of input data */                                           \
                else if (sizeof_data_type == 2 || sizeof_data_type == 4)                           \
                {                                                                                  \
                    data_type transf_data = *(data_buffer + arr_index);                            \
                    /* !!! Without delay the SPI_SWAP_DATA_TX result is incorrect !!! */           \
                    vTaskDelay(pdMS_TO_TICKS(1));                                                  \
                    /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */           \
                    /* from documantation "Transactions with Integers Other Than uint8_t" */       \
                    transf_data = SPI_SWAP_DATA_TX(transf_data, 8 * sizeof_data_type);             \
                    main_transaction_write.length = 8 * sizeof_data_type;                          \
                    main_transaction_write.tx_buffer = &transf_data;                               \
                    main_transaction_write.flags = 0;                                              \
                    address += sizeof_data_type;                                                   \
                }                                                                                  \
                                                                                                   \
                /* Set the write enable latch (enable write operations) */                         \
                ESP_ERROR_CHECK(eeprom_send_simple_cmd(CMD_WREN));                                 \
                                                                                                   \
                if (spi_device_polling_transmit(eeprom_handle, &main_transaction_write) == ESP_OK) \
                {                                                                                  \
                    while (eeprom_is_busy())                                                       \
                    {                                                                              \
                        ESP_LOGW(EEPROM_TAG, "Writing data ...");                                  \
                    }                                                                              \
                                                                                                   \
                    arr_index++;                                                                   \
                    chunk--;                                                                       \
                }                                                                                  \
                else                                                                               \
                {                                                                                  \
                    tr_result = ESP_ERR_INVALID_ARG;                                               \
                }                                                                                  \
            }                                                                                      \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            tr_result = ESP_ERR_INVALID_ARG;                                                       \
        }                                                                                          \
    }

/**
 * @brief Template of reading data from the address.
 *
 * @param addr Address.
 * @param data_type Type of data - number of bits (8, 16, 32) to write. (ex. int8_t, uint16_t, int32_t, ...).
 * @param result_buffer Buffer for storing the result of reading. Type of data has to be the same like data_type argument.
 * @param tr_result Result of transaction.
 * @return tr_result
 * - ESP_OK: on success.
 * - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 * - ESP_ERR_NO_MEM: If memory allocation is failed.
 */
#define eeprom_read_data(addr, data_type, result_buffer, tr_result)                                                       \
    {                                                                                                                     \
        tr_result = ESP_OK;                                                                                               \
        uint8_t sizeof_data_type = (uint8_t)sizeof(data_type);                                                            \
        if (sizeof_data_type == 1 || sizeof_data_type == 2 || sizeof_data_type == 4)                                      \
        {                                                                                                                 \
            /* Allocate memory for buffer of reading */                                                                   \
            uint8_t *read_data = (uint8_t *)malloc(4 * sizeof(uint8_t));                                                  \
                                                                                                                          \
            if (read_data != NULL)                                                                                        \
            {                                                                                                             \
                if (eeprom_address_data(addr, read_data) == ESP_OK)                                                       \
                {                                                                                                         \
                    /* log_buffer_hexdump(read_data); */                                                                  \
                    if (sizeof_data_type == 1)                                                                            \
                    {                                                                                                     \
                        result_buffer = read_data[0];                                                                     \
                    }                                                                                                     \
                    else if (sizeof_data_type == 2)                                                                       \
                    {                                                                                                     \
                        result_buffer = (read_data[1] << 8) | read_data[0];                                               \
                    }                                                                                                     \
                    else if (sizeof_data_type == 4)                                                                       \
                    {                                                                                                     \
                        result_buffer = (read_data[3] << 24) | (read_data[2] << 16) | (read_data[1] << 8) | read_data[0]; \
                    }                                                                                                     \
                    /* Freeing allocated memory */                                                                        \
                    free(read_data);                                                                                      \
                }                                                                                                         \
                else                                                                                                      \
                {                                                                                                         \
                    tr_result = ESP_ERR_INVALID_ARG;                                                                      \
                }                                                                                                         \
            }                                                                                                             \
            else                                                                                                          \
            {                                                                                                             \
                tr_result = ESP_ERR_NO_MEM;                                                                               \
            }                                                                                                             \
        }                                                                                                                 \
        else                                                                                                              \
        {                                                                                                                 \
            tr_result = ESP_ERR_INVALID_ARG;                                                                              \
        }                                                                                                                 \
    }

/*
 * Basic device handle
 */
spi_device_handle_t eeprom_handle;

/*
 * Basic spi_transaction_ext_t to execute (CMD_READ, CMD_WRDI, CMD_RDSR , CMD_WREN).
 */
spi_transaction_ext_t main_transaction_ext;

/*
 * Basic spi_transaction_t to execute (CMD_WRITE, CMD_WRSR).
 */
spi_transaction_t main_transaction_write;

/**
 * @brief Initialize GPIO pins of NodeMCU32s to control WP and HD pins of eeprom.
 *
 * @param void
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_wp_hd_pins_init(void);

/**
 * @brief Initialize the SPI bus and add device (EEPROM) to the bus.
 *
 * @param void
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t init_spi(void);

/**
 * @brief Callback to be called before a transmission is started.
 *
 * @param tr - Description of transaction to execute.
 * @return void
 */
void cs_active(spi_transaction_t *tr);

/**
 * @brief Callback to be called after a transmission has completed.
 *
 * @param tr - Description of transaction to execute.
 * @return void
 */
void cs_inactive(spi_transaction_t *tr);

/**
 * @brief Set state of WP (Write-Protect) pin of eeprom.
 * When WP is low and WPEN is high, writing to the nonvolatile
 * bits in the STATUS register is disabled.
 * When WP is high, all functions, including writes to the nonvolatile
 * bits in the STATUS register, operate normally.
 *
 * @param state - HIGH or LOW.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_: If error occurs.
 */
esp_err_t eeprom_set_wp_level(const uint32_t state);

/**
 * @brief Set state of HOLD pin of eeprom.
 * HIGH: transmission is enabled. LOW: - transmission is disabled.
 *
 * @param state - HIGH or LOW.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_: If error occurs.
 */
esp_err_t eeprom_set_hd_level(const uint32_t state);

/**
 * @brief Prints ESP_LOG_BUFFER_HEXDUMP.
 *
 * @param data – address of data.
 * @return void
 */
void log_buffer_hexdump(uint8_t *data);

/**
 * @brief Check the Write-In-Process. WIP bit indicates whether the
 * eeprom is busy with a write operation. When set to
 * a ‘1’, a write is in progress, when set to a ‘0’, no write
 * is in progress.
 *
 * @param void
 * @return
 *  - 1: The eeprom is busy, a write is in progress.
 *  - 0: No write is in progress.
 */
uint8_t eeprom_is_busy(void);

/**
 * @brief Read status register of eeprom in to the buffer.
 *
 * @param buffer to store data of status register.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_read_status_reg(uint8_t *buffer);

/**
 * @brief Enabling / Disabling WPEN bit of Status Register of eeprom.
 *
 * @param status TRUE: WPEN enabled, FALSE: WPEN disabled.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_enable_wpen(bool status);

/**
 * @brief Check WPEN bit of Status Register of eeprom.
 *
 * @param void
 * @return
 *  - false: WPEN bit disabled.
 *  - true: WPEN bit enabled.
 */
bool eeprom_wpen_check(void);

/**
 * @brief Enabling / Disabling Write Protect for sector(s) of eeprom.
 *
 * @param sector sector to be protected.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_write_protect_sector(eeprom_write_protect sector);

/**
 * @brief Sends simple command, instruction to the eeprom.
 *
 * @param command
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_send_simple_cmd(const uint16_t command);

/**
 * @brief Chip Erase – erase all sectors in memory array.
 *
 * @param void
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_chip_erase(void);

/**
 * @brief Sector Erase – erase all bits (FFh) inside the given sector.
 *
 * @param addr Address in the sector.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_sector_erase(const uint16_t addr);

/**
 * @brief Page Erase – erase one page in memory array.
 *
 * @param addr Address in the page.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_page_erase(const uint16_t addr);

/**
 * @brief Set eeprom into DEEP POWER-DOWN MODE and release from Deep power-down and read Manufacturer’s ID.
 *
 * @param mode TRUE - set DPD mode. FALSE - set RDID mode.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_: If error occurs.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_dpd_rdid_mode(const bool mode);

/**
 * @brief Read 32 bits length data from address.
 *
 * @param addr Address.
 * @param result_buffer Buffer to store result of transaction.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_: If the initialization is incorrect.
 */
esp_err_t eeprom_address_data(const uint16_t addr, uint8_t *result_buffer);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// https://stackoverflow.com/questions/21384343/what-is-wrong-with-this-int-to-float-conversion-in-c/21384978#21384978
// https://gregstoll.com/~gregstoll/floattohex/
/**
 * @brief Write float data in to the address.
 *
 * @param addr Address.
 * @param data Data to write.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_write_float_data(const uint16_t addr, const float *data);

/**
 * @brief Read float data from address.
 *
 * @param addr Address.
 * @param buffer Buffer to store the result of float data.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 *  - ESP_ERR_NO_MEM: Error during allocating the memory for rx_data.
 */
esp_err_t eeprom_read_float_data(const uint16_t addr, float *buffer);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Reads and prints out all stored data in eeprom.
 *
 * @param void
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_NO_MEM: If memory allocation is failed.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_all_stored_data(void);

/**
 * @brief Reads and prints out all stored data from the selected page.
 *
 * @param page Page number of eeprom.
 * @return
 *  - ESP_OK: on success.
 *  - ESP_ERR_NO_MEM: If memory allocation is failed.
 *  - ESP_ERR_INVALID_ARG: If the initialization is incorrect.
 */
esp_err_t eeprom_page_stored_data(const uint16_t page);

/**
 * @brief Prints out number of eeprom of pages with start and end addresses.
 *
 * @param void
 * @return void
 */
void eeprom_pages_eg(void);

/**
 * @brief Read and write status register example.
 *
 * @param void
 * @return void
 */
void eeprom_status_register_eg(void);

/**
 * @brief Read and write 8 bits data example.
 *
 * @param void
 * @return void
 */
void eeprom_write_read_8bits_data_eg(void);

/**
 * @brief Read and write 16 bits data example.
 *
 * @param void
 * @return void
 */
void eeprom_write_read_16bits_data_eg(void);

/**
 * @brief Read and write 32 bits data example.
 *
 * @param void
 * @return void
 */
void eeprom_write_read_32bits_data_eg(void);

/**
 * @brief Read and write float data example.
 *
 * @param void
 * @return void
 */
void eeprom_write_read_float_data_eg(void);

/**
 * @brief Read and write character array example.
 *
 * @param void
 * @return void
 */
void eeprom_write_read_char_array_eg(void);

/**
 * @brief Page erase of eeprom example.
 *
 * @param void
 * @return void
 */
void eeprom_page_erase_eg(void);

/**
 * @brief Sector erase of eeprom example.
 *
 * @param void
 * @return void
 */
void eeprom_sector_erase_eg(void);

/**
 * @brief Chip erase example.
 *
 * @param void
 * @return void
 */
void eeprom_chip_erase_eg(void);

/**
 * @brief Example to write WPEN bit of Status Register of eeprom.
 *
 * @param void
 * @return void
 */
void eeprom_enable_wpen_eg(void);

/**
 * @brief Example to write protection of sector of eeprom.
 *
 * @param void
 * @return void
 */
void eeprom_write_protect_sector_eg(void);

/**
 * @brief Example to set eeprom into DEEP POWER-DOWN mode and
 * RELEASE eeprom from DEEP POWER-DOWN AND read ELECTRONIC SIGNATURE
 *
 * @param void
 * @return void
 */
void eeprom_dpd_rdid_mode_eg(void);

/**
 * @brief Example for testing the function of WP pin of eeprom
 *
 * @param void
 * @return void
 */
void eeprom_wp_pin_eg(void);

/**
 * @brief Example for testing the function of HD (HOLD) pin of eeprom
 *
 * @param void
 * @return void
 */
void eeprom_hd_pin_eg(void);
