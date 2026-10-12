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

    memset(obj->config_tx, 0, sizeof(obj->config_tx));
    obj->config_tx[0] = NRF905_INST_W_CONFIG | 0; /* Write config from byte 0 */
    obj->config_tx[1] = cfg->ch_no & 0b11111111;
    obj->config_tx[2] = (cfg->auto_retran << 5) | (cfg->rx_red_pwr << 4) | ((cfg->pa_pwr & 0b11) << 2) | ((cfg->pll_freq & 0b1) << 1) | ((cfg->ch_no >> 8) & 0b1);
    obj->config_tx[3] = ((cfg->tx_afw & 0b111) << 4) | (cfg->rx_afw & 0b111);
    obj->config_tx[4] = cfg->rx_pw & 0b00111111;
    obj->config_tx[5] = cfg->tx_pw & 0b00111111;
    obj->config_tx[6] = cfg->rx_addr & 0x000000FF;
    obj->config_tx[7] = (cfg->rx_addr & 0x0000FF00) >> 8;
    obj->config_tx[8] = (cfg->rx_addr & 0x00FF0000) >> 16;
    obj->config_tx[9] = (cfg->rx_addr & 0xFF000000) >> 24;
    obj->config_tx[10] = (cfg->crc_16 << 7) | (cfg->crc_en << 6) | ((cfg->xof & 0b111) << 3) | (cfg->up_clk_en << 2) | (cfg->up_clk_freq & 0b11);

    return obj->spi_transmit_cb(obj->config_tx, NULL, sizeof(obj->config_tx));
}

bool nrf905_get_config(struct nrf905_obj *obj, uint8_t **data)
{
    if (!obj || !data)
        return false;

    memset(obj->config_rx, 0, sizeof(obj->config_rx));
    obj->config_rx[0] = NRF905_INST_R_CONFIG;
    *data = obj->config_rx + 1;
    return obj->spi_transmit_cb(obj->config_rx, obj->config_rx, sizeof(obj->config_rx));
}

bool nrf905_get_status(struct nrf905_obj *obj, uint8_t **data)
{
    if (!obj || !data)
        return false;

    obj->status = 0;
    *data = &obj->status;
    return obj->spi_transmit_cb(NULL, &obj->status, sizeof(obj->status));
}

enum nrf905_status nrf905_parse_status(uint8_t raw)
{
    return (raw & 0b10000000 ? NRF905_STATUS_AM : 0) |
           (raw & 0b00100000 ? NRF905_STATUS_DR : 0);
}

bool nrf905_set_mode(struct nrf905_obj *obj, enum nrf905_mode mode)
{
    if (!obj || mode > NRF905_MODE_SHOCKBURST_TX_RADIO_EN)
        return false;

    bool pwr = mode & 0b100;
    bool ce = mode & 0b010;
    bool txe = mode & 0b001;

    /* Select the direction in standby, then enable the requested radio mode. */
    obj->pin_set_cb(NRF905_PIN_CE, false);
    obj->pin_set_cb(NRF905_PIN_PWR, pwr);
    obj->pin_set_cb(NRF905_PIN_TXE, txe);
    obj->pin_set_cb(NRF905_PIN_CE, ce);

    return true;
}

bool nrf905_set_tx_addr(struct nrf905_obj *obj, uint32_t addr)
{
    if (!obj)
        return false;

    /* Keep the command alive until asynchronous SPI DMA completes. */
    obj->tx_addr[0] = NRF905_INST_W_TX_ADDR;
    obj->tx_addr[1] = addr & 0x000000FF;
    obj->tx_addr[2] = (addr & 0x0000FF00) >> 8;
    obj->tx_addr[3] = (addr & 0x00FF0000) >> 16;
    obj->tx_addr[4] = (addr & 0xFF000000) >> 24;

    return obj->spi_transmit_cb(obj->tx_addr, NULL, sizeof(obj->tx_addr));
}  

bool nrf905_set_tx_data(struct nrf905_obj *obj, const uint8_t *payload, uint8_t len)
{
    if (!obj || !payload || len == 0 || len > NRF905_MAX_PAYLOAD_SIZE)
        return false;

    obj->tx_payload[0] = NRF905_INST_W_TX_PAYLOAD;
    memcpy(obj->tx_payload + 1, payload, len);

    return obj->spi_transmit_cb(obj->tx_payload, NULL, len + 1);
}   

bool nrf905_get_rx_data(struct nrf905_obj *obj, uint8_t **data)
{
    if (!obj || !data)
        return false;

    memset(obj->rx_payload, 0x00, sizeof(obj->rx_payload));

    obj->rx_payload[0] = NRF905_INST_R_RX_PAYLOAD;

    *data = obj->rx_payload + 1;
    return obj->spi_transmit_cb(obj->rx_payload, obj->rx_payload, sizeof(obj->rx_payload));
}

//------------------------------------------------------------------------------
