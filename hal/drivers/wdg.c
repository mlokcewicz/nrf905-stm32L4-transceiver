//------------------------------------------------------------------------------

/// @file wdg.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "wdg.h"

//------------------------------------------------------------------------------

void wdg_init(uint32_t prescaler, uint32_t reload)
{
    /* Enable WDG */
    IWDG->KR = 0x0000CCCCu;

    /* Enable write access */
    IWDG->KR = 0x00005555u;

    IWDG->PR = prescaler;
    IWDG->RLR = reload;

    /* Wait for the prescaler and reload updates in the LSI clock domain. */
    while (IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU)) {};

    /* Reload */
    IWDG->KR = 0x0000AAAAu;

    /* Disable write access */
    IWDG->KR = 0x00000000u;
}

void wdg_feed(void)
{
    IWDG->KR = 0x0000AAAAu;
}

//------------------------------------------------------------------------------
