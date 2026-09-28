//------------------------------------------------------------------------------

/// @file spi.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "spi.h"

#include <string.h>

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

bool spi_init(struct spi_obj *obj, struct spi_cfg *cfg)
{
    if (!obj || !cfg)
        return false;

    obj->tc_cb = cfg->tc_cb;

    return true;
}

uint8_t spi_transmit(struct spi_obj *obj, uint8_t byte)
{
    if (!obj)
        return 0;

        
}

void spi_cs_set(struct spi_obj *obj, bool state)
{
}

void spi_deinit(struct spi_obj *obj)
{
    if (!obj)
        return;

    memset(obj, 0x00, sizeof(*obj));
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
