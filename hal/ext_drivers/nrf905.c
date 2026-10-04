//------------------------------------------------------------------------------

/// @file nrf905.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "nrf905.h"

#include <stddef.h>
#include <string.h>

//------------------------------------------------------------------------------

#define NRF905_MAX_PAYLOAD_SIZE 32

//------------------------------------------------------------------------------

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

bool nrf905_init(struct nrf905_obj *obj, struct nrf905_cfg *cfg)
{
    if (!obj || !cfg || !cfg->io_init_cb || !cfg->spi_transmit_cb ||
        !cfg->pin_set_cb || !cfg->pin_get_cb)
        return false;

    obj->io_init_cb = cfg->io_init_cb;
    obj->spi_transmit_cb = cfg->spi_transmit_cb;
    obj->pin_set_cb = cfg->pin_set_cb;
    obj->pin_get_cb = cfg->pin_get_cb;

    if (!obj->io_init_cb())
        return false;

    if (!nrf905_set_mode(obj, NRF905_MODE_POWER_DOWN))
        return false;

    uint8_t config[11] = {0};
    config[0] = NRF905_INST_W_CONFIG | 0; /* Write config from byte 0 */
    config[1] = cfg->ch_no & 0b11111111;
    config[2] = (cfg->auto_retran << 5) | (cfg->rx_red_pwr << 4) | ((cfg->pa_pwr & 0b11) << 2) | ((cfg->pll_freq & 0b1) << 1) | ((cfg->ch_no >> 8) & 0b1);
    config[3] = ((cfg->tx_afw & 0b111) << 4) | (cfg->rx_afw & 0b111);
    config[4] = cfg->rx_pw & 0b00111111;
    config[5] = cfg->tx_pw & 0b00111111;
    config[6] = cfg->rx_addr & 0x000000FF;
    config[7] = (cfg->rx_addr & 0x0000FF00) >> 8;
    config[8] = (cfg->rx_addr & 0x00FF0000) >> 16;
    config[9] = (cfg->rx_addr & 0xFF000000) >> 24;
    config[10] = (cfg->crc_16 << 7) | (cfg->crc_en << 6) | ((cfg->xof & 0b111) << 3) | (cfg->up_clk_en << 2) | (cfg->up_clk_freq & 0b11);

    if (!obj->spi_transmit_cb(config, NULL, sizeof(config)))
        return false;
    
    config[0] = NRF905_INST_R_CONFIG | 0;
    uint8_t config_read[11] = {0};
    
    if (!obj->spi_transmit_cb(config, config_read, sizeof(config)))
        return false;

    if (memcmp(config + 1, config_read + 1, sizeof(config) - 1) != 0)
        return false;
    
    return true;
}

bool nrf905_get_status(struct nrf905_obj *obj, enum nrf905_status *status)
{
    if (!obj || !status)
        return false;

    uint8_t status_buf = 0;

    if (!obj->spi_transmit_cb(NULL, &status_buf, sizeof(status_buf)))
        return false;

    *status = (!!(status_buf & 0b00100000)) << 1 | !!(status_buf & 0b10000000);

    return true;
}

bool nrf905_set_mode(struct nrf905_obj *obj, enum nrf905_mode mode)
{
    if (!obj || mode > NRF905_MODE_SHOCKBURST_TX_RADIO_EN)
        return false;

    bool pwr = mode & 0b100;
    bool ce = mode & 0b010;
    bool txe = mode & 0b001;

    obj->pin_set_cb(NRF905_PIN_PWR, pwr);
    obj->pin_set_cb(NRF905_PIN_CE, ce);
    obj->pin_set_cb(NRF905_PIN_TXE, txe);

    return true;
}

bool nrf905_set_tx_data(struct nrf905_obj *obj, uint32_t addr, const uint8_t *payload, uint8_t len)
{
    if (!obj || !payload || len == 0 || len > NRF905_MAX_PAYLOAD_SIZE)
        return false;

    uint8_t tx_addr[5] = {0};
    tx_addr[0] = NRF905_INST_W_TX_ADDR;
    tx_addr[1] = addr & 0x000000FF;
    tx_addr[2] = (addr & 0x0000FF00) >> 8;
    tx_addr[3] = (addr & 0x00FF0000) >> 16;
    tx_addr[4] = (addr & 0xFF000000) >> 24;

    if (!obj->spi_transmit_cb(tx_addr, NULL, sizeof(tx_addr)))
        return false;

    uint8_t tx_payload[33] = {0};
    tx_payload[0] = NRF905_INST_W_TX_PAYLOAD;
    memcpy(tx_payload + 1, payload, len);

    if (!obj->spi_transmit_cb(tx_payload, NULL, len + 1))
        return false;

    return true;
}   

bool nrf905_get_rx_data(struct nrf905_obj *obj, uint8_t *data, uint8_t size)
{
    if (!obj || !data || size == 0 || size > NRF905_MAX_PAYLOAD_SIZE)
        return false;

    uint8_t rx_payload[33] = {0};
    rx_payload[0] = NRF905_INST_R_RX_PAYLOAD;

    if (!obj->spi_transmit_cb(rx_payload, rx_payload, sizeof(rx_payload)))
        return false;

    memcpy(data, rx_payload + 1, size);

    return true;
}

//------------------------------------------------------------------------------
