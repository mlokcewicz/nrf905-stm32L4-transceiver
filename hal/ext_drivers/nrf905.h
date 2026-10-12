//------------------------------------------------------------------------------

/// @file nrf905.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef NRF905_H_
#define NRF905_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

enum nrf905_pin
{
    NRF905_PIN_CS,
    NRF905_PIN_PWR,
    NRF905_PIN_CE,
    NRF905_PIN_TXE,
    NRF905_PIN_DRDY,
};

enum nrf905_status
{
    NRF905_STATUS_AM = 1 << 0,
    NRF905_STATUS_DR = 1 << 1,
};

enum nrf905_mode
{
    NRF905_MODE_POWER_DOWN = 0b000,
    NRF905_MODE_STANDBY = 0b100,
    NRF905_MODE_RX_READ = 0b100,
    NRF905_MODE_SHOCKBURST_RX_RADIO_EN = 0b110,
    NRF905_MODE_SHOCKBURST_TX_RADIO_EN_CE_LOW = 0b101,
    NRF905_MODE_SHOCKBURST_TX_RADIO_EN = 0b111,
};

enum nrf905_pll_freq
{
    NRF905_PLL_FREQ_433_MHZ,
    NRF905_PLL_FREQ_868_915_MHZ,
};

enum nrf905_pa_pwr
{
    NRF905_PA_PWR_MINUS_10_DBM = 0b00,
    NRF905_PA_PWR_MINUS_2_DBM = 0b01,
    NRF905_PA_PWR_6_DBM = 0b10,
    NRF905_PA_PWR_10_DBM = 0b11,
};

enum nrf905_addr_width
{
    NRF905_ADDR_WIDTH_1_BYTE = 0b001,
    NRF905_ADDR_WIDTH_4_BYTE = 0b100,
};

enum nrd905_up_clk_freq
{
    NRF905_UP_CLK_FREQ_4_MHZ = 0b00,
    NRF905_UP_CLK_FREQ_2_MHZ = 0b01,
    NRF905_UP_CLK_FREQ_1_MHZ = 0b10,
    NRF905_UP_CLK_FREQ_500_KHZ = 0b11,
};

enum nrf905_xof
{
    NRF905_XOF_4_MHZ = 0b000,
    NRF905_XOF_8_MHZ = 0b001,
    NRF905_XOF_12_MHZ = 0b010,
    NRF905_XOF_16_MHZ = 0b011,
    NRF905_XOF_20_MHZ = 0b100,
};

enum nrf905_instruction
{
    NRF905_INST_W_CONFIG = 0b00000000,
    NRF905_INST_R_CONFIG = 0b00010000,
    NRF905_INST_W_TX_PAYLOAD = 0b00100000,
    NRF905_INST_R_TX_PAYLOAD = 0b00100001,
    NRF905_INST_W_TX_ADDR = 0b00100010,
    NRF905_INST_R_TX_ADDR = 0b00100011,
    NRF905_INST_R_RX_PAYLOAD = 0b00100100,
    NRF905_INST_CHANNEL_CONFIG = 0b10000000000000000,
};

//------------------------------------------------------------------------------

typedef bool (*nrf905_io_init_cb)(void);
typedef bool (*nrf905_spi_transmit_cb)(uint8_t *data_tx, uint8_t *data_rx, uint32_t len);
typedef bool (*nrf905_pin_set_cb)(enum nrf905_pin pin, bool state);
typedef bool (*nrf905_pin_get_cb)(enum nrf905_pin pin);

//------------------------------------------------------------------------------

struct nrf905_cfg
{
    nrf905_io_init_cb io_init_cb;
    nrf905_spi_transmit_cb spi_transmit_cb;
    nrf905_pin_set_cb pin_set_cb;
    nrf905_pin_get_cb pin_get_cb;

    uint16_t ch_no;
    enum nrf905_pll_freq pll_freq;
    enum nrf905_pa_pwr pa_pwr;
    bool rx_red_pwr;
    bool auto_retran;
    enum nrf905_addr_width rx_afw;
    enum nrf905_addr_width tx_afw;
    uint8_t rx_pw;
    uint8_t tx_pw;
    uint32_t rx_addr;
    enum nrd905_up_clk_freq up_clk_freq;
    bool up_clk_en;
    enum nrf905_xof xof;
    bool crc_en;
    bool crc_16;
};

struct nrf905_obj
{
    nrf905_io_init_cb io_init_cb;
    nrf905_spi_transmit_cb spi_transmit_cb;
    nrf905_pin_set_cb pin_set_cb;
    nrf905_pin_get_cb pin_get_cb;

    uint8_t config_tx[10 + 1];
    uint8_t config_rx[10 + 1];
    uint8_t status;
    uint8_t tx_addr[4 + 1];
    uint8_t tx_payload[32 + 1];
    uint8_t rx_payload[32 + 1];
};

//------------------------------------------------------------------------------

/// @brief Initializes callbacks and GPIO, enters power-down and starts writing configuration
/// @note Does not verify readback. Caller waits, reads configuration and compares it.
/// @note Configuration field ranges are not validated; all callbacks are required.
/// @return true if setup succeeds and the SPI callback accepts the write
bool nrf905_init(struct nrf905_obj *obj, struct nrf905_cfg *cfg);

/// @brief Starts reading all 10 configuration bytes
/// @param data Receives a pointer to object-owned configuration bytes, excluding SPI status
/// @note Read the bytes only after SPI completion. Next configuration read overwrites them.
/// @note The output pointer is set before calling SPI, even if the callback rejects the transfer.
/// @return The SPI callback result; false for invalid arguments
bool nrf905_get_config(struct nrf905_obj *obj, uint8_t **data);

/// @brief Compares configuration readback with the encoded settings without SPI access
/// @param cfg Expected settings; callback fields are ignored
/// @param data Ten completed configuration bytes returned by nrf905_get_config
/// @return true if all bytes match, false for NULL arguments or a mismatch
bool nrf905_config_validate(const struct nrf905_cfg *cfg, const uint8_t *data);

/// @brief Starts reading the raw radio status byte
/// @param data Receives a pointer to the object-owned raw status byte
/// @note Wait for SPI completion before parsing; next status read overwrites the byte.
/// @note The output pointer is set before calling SPI, even if the callback rejects the transfer.
/// @return The SPI callback result; false for invalid arguments
bool nrf905_get_status(struct nrf905_obj *obj, uint8_t **data);

/// @brief Decodes AM and DR flags from a completed raw status byte without SPI access
enum nrf905_status nrf905_parse_status(uint8_t raw);

/// @brief Lowers CE, sets PWR and TXE, then raises CE if the mode requires it
/// @param obj Initialized radio object
/// @param mode Requested operating mode
/// @return false for NULL obj or mode above the highest enum value, otherwise true
bool nrf905_set_mode(struct nrf905_obj *obj, enum nrf905_mode mode);

/// @brief Writes the TX address register
/// @note Does not start radio transmission; call with the radio in standby or power-down mode
/// @param obj Initialized radio object
/// @param addr Destination address, intended to be stored least-significant byte first
/// @return true if the SPI callback accepts the write, false otherwise
bool nrf905_set_tx_addr(struct nrf905_obj *obj, uint32_t addr);

/// @brief Starts writing len payload bytes to the TX payload register
/// @note Does not start radio transmission; call with the radio in standby or power-down mode
/// @param obj Initialized radio object
/// @param payload Buffer containing len payload bytes
/// @param len Number of bytes to copy and transmit, from 1 to 32
/// @return true if the SPI callback accepts the write, false otherwise
bool nrf905_set_tx_data(struct nrf905_obj *obj, const uint8_t *payload, uint8_t len);

/// @brief Starts reading the full 32-byte RX payload without copying to caller memory
/// @param data Receives a pointer to object-owned payload bytes, excluding SPI status
/// @note Wait for SPI completion before reading; next RX payload read overwrites the buffer.
/// @note The output pointer is set before calling SPI, even if the callback rejects the transfer.
/// @return The SPI callback result; false for invalid arguments
bool nrf905_get_rx_data(struct nrf905_obj *obj, uint8_t **data);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* NRF905_H_ */

//------------------------------------------------------------------------------
