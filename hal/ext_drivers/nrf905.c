//------------------------------------------------------------------------------

/// @file nrf905.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "nrf905.h"

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
        !cfg->pin_set_cb || !cfg->pin_get_cb || !cfg->drdy_cb)
        return false;

    obj->io_init_cb = cfg->io_init_cb;
    obj->spi_transmit_cb = cfg->spi_transmit_cb;
    obj->pin_set_cb = cfg->pin_set_cb;
    obj->pin_get_cb = cfg->pin_get_cb;
    obj->drdy_cb = cfg->drdy_cb;

    if (!obj->io_init_cb())
        return false;

    if (!nrf905_set_mode(obj, NRF905_MODE_POWER_DOWN))
        return false;

    uint8_t config[10] = {0};
    config[0] = cfg->ch_no & 0b11111111;
    config[1] = (cfg->auto_retran << 5) | (cfg->rx_red_pwr << 4) | ((cfg->pa_pwr & 0b11) << 2) | ((cfg->pll_freq & 0b1) << 1) | ((cfg->ch_no >> 8) & 0b1);
    config[2] = ((cfg->tx_afw & 0b111) << 4) | (cfg->rx_afw & 0b111);
    config[3] = cfg->rx_pw & 0b00111111;
    config[4] = cfg->tx_pw & 0b00111111;
    config[5] = cfg->rx_addr[0];
    config[6] = cfg->rx_addr[1];
    config[7] = cfg->rx_addr[2];
    config[8] = cfg->rx_addr[3];
    config[9] = (cfg->crc_16 << 7) | (cfg->crc_en << 6) | ((cfg->xof & 0b111) << 3) | (cfg->up_clk_en << 2) | (cfg->op_clk_freq & 0b11);

    /* Configuration packing only; SPI register write is not implemented yet. */
    (void)config;
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

bool nrf905_send(struct nrf905_obj *obj, uint8_t *data, uint8_t size)
{

    return true;
}

bool nrf905_receive(struct nrf905_obj *obj, uint8_t *data, uint8_t size, uint8_t *rd)
{
    return true;
}


//------------------------------------------------------------------------------
