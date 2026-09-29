//------------------------------------------------------------------------------

/// @file nrf905.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "nrf905.h"

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

    /* Keep CE low before selecting TX mode and powering up into standby. */
    if (!obj->pin_set_cb(NRF905_PIN_CE, false) ||
        !obj->pin_set_cb(NRF905_PIN_TXE, true) ||
        !obj->pin_set_cb(NRF905_PIN_PWR, true))
        return false;

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
