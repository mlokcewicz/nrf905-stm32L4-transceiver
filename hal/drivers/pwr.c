//------------------------------------------------------------------------------

/// @file pwr.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "pwr.h"

//------------------------------------------------------------------------------

void pwr_init(uint32_t range)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    (void)RCC->APB1ENR1; /* Allow the peripheral clock to become active */

    while (PWR->SR2 & PWR_SR2_VOSF) {}

    PWR->CR1 = (PWR->CR1 & ~PWR_CR1_VOS) | (range & PWR_CR1_VOS);

    while (PWR->SR2 & PWR_SR2_VOSF) {}
}

//------------------------------------------------------------------------------
