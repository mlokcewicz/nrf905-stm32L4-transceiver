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
};

//------------------------------------------------------------------------------

/// @brief Initializes GPIO, enters power-down mode and writes and verifies all 10 configuration bytes
/// @note All callbacks are required; configuration field ranges are not validated
/// @note Leaves the radio in power-down mode
/// @param obj Object receiving the configured callbacks
/// @param cfg Configuration and platform callbacks
/// @return true if initialization and register readback succeed, otherwise false
bool nrf905_init(struct nrf905_obj *obj, struct nrf905_cfg *cfg);

/// @brief Reads the radio status through a one-byte SPI transaction
/// @param obj Initialized radio object
/// @param status Output status flags; unchanged if the operation fails
/// @return true if the status was read, false for NULL arguments or SPI failure
bool nrf905_get_status(struct nrf905_obj *obj, enum nrf905_status *status);

/// @brief Writes PWR, CE and TXE pin levels encoded by mode, in that order
/// @param obj Initialized radio object
/// @param mode Requested operating mode
/// @return false for NULL obj or mode above the highest enum value, otherwise true
bool nrf905_set_mode(struct nrf905_obj *obj, enum nrf905_mode mode);

/// @brief Writes the TX address register and a zero-padded 32-byte TX payload buffer
/// @note Does not start radio transmission; call with the radio in standby or power-down mode
/// @param obj Initialized radio object
/// @param addr Destination address, intended to be stored least-significant byte first
/// @param payload Buffer containing len payload bytes
/// @param len Number of bytes to copy, from 1 to 32; remaining buffer bytes are zeroed
/// @return true if both SPI writes succeed, false for invalid arguments or SPI failure
bool nrf905_set_tx_data(struct nrf905_obj *obj, uint32_t addr, const uint8_t *payload, uint8_t len);

/// @brief Reads size bytes from the beginning of the RX payload register
/// @param obj Initialized radio object
/// @param data Destination buffer with space for size bytes
/// @param size Number of bytes to read, from 1 to 32
/// @return true if the SPI read succeeds, false for invalid arguments or SPI failure
bool nrf905_get_rx_data(struct nrf905_obj *obj, uint8_t *data, uint8_t size);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* NRF905_H_ */

//------------------------------------------------------------------------------
