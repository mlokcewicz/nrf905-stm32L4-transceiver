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

bool nrf905_send(struct nrf905_obj *obj, uint8_t *data, uint8_t size);

bool nrf905_receive(struct nrf905_obj *obj, uint8_t *data, uint8_t size, uint8_t *rd);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* NRF905_H_ */

//------------------------------------------------------------------------------
