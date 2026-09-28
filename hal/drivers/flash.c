//------------------------------------------------------------------------------

/// @file flash.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "flash.h"

//------------------------------------------------------------------------------

void flash_init(uint32_t latency)
{
    FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | (latency & FLASH_ACR_LATENCY);

    while ((FLASH->ACR & FLASH_ACR_LATENCY) != (latency & FLASH_ACR_LATENCY)) {}
}

//------------------------------------------------------------------------------
