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
typedef bool (*nrf905_drdy_cb)(void);

//------------------------------------------------------------------------------

struct nrf905_cfg
{
    nrf905_io_init_cb io_init_cb;
    nrf905_spi_transmit_cb spi_transmit_cb;
    nrf905_pin_set_cb pin_set_cb;
    nrf905_pin_get_cb pin_get_cb;
    nrf905_drdy_cb drdy_cb;

    uint16_t ch_no;
    enum nrf905_pll_freq pll_freq;
    enum nrf905_pa_pwr pa_pwr;
    bool rx_red_pwr;
    bool auto_retran;
    enum nrf905_addr_width rx_afw;
    enum nrf905_addr_width tx_afw;
    uint8_t rx_pw;
    uint8_t tx_pw;
    uint8_t rx_addr[4];
    enum nrd905_up_clk_freq op_clk_freq;
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
    nrf905_drdy_cb drdy_cb;
};

//------------------------------------------------------------------------------

bool nrf905_init(struct nrf905_obj *obj, struct nrf905_cfg *cfg);

bool nrf905_set_mode(struct nrf905_obj *obj, enum nrf905_mode mode);

bool nrf905_send(struct nrf905_obj *obj, uint8_t *data, uint8_t size);

bool nrf905_receive(struct nrf905_obj *obj, uint8_t *data, uint8_t size, uint8_t *rd);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* NRF905_H_ */

//------------------------------------------------------------------------------
