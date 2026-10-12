//------------------------------------------------------------------------------

/// @file pwr.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "pwr.h"

//------------------------------------------------------------------------------

#define PWR_LOW_POWER_MODE_CFG_ITEM(main_regulatror_enable, lpms, sleepdeep, rrs) \
    { main_regulatror_enable, lpms, sleepdeep, rrs }

//------------------------------------------------------------------------------

struct pwr_low_power_mode_cfg
{
    uint8_t main_regulatror_enable: 1;
    uint8_t lpms: 3;
    uint8_t sleepdeep: 1;
    uint8_t rrs: 1;
};

//------------------------------------------------------------------------------

static const struct pwr_low_power_mode_cfg pwr_low_power_mode[] =
{
    [PWR_MODE_SLEEP] = PWR_LOW_POWER_MODE_CFG_ITEM(1, 0b000, 0, 0),
    [PWR_MODE_LOW_POWER] = PWR_LOW_POWER_MODE_CFG_ITEM(0, 0b000, 0, 0),
    [PWR_MODE_STOP_1_MR] = PWR_LOW_POWER_MODE_CFG_ITEM(1, 0b000, 1, 0),
    [PWR_MODE_STOP_1_LPR] = PWR_LOW_POWER_MODE_CFG_ITEM(1, 0b001, 1, 0),
    [PWR_MODE_STOP_2_LPR] = PWR_LOW_POWER_MODE_CFG_ITEM(1, 0b010, 1, 0),
    [PWR_MODE_STANDBY_LPR_SRAM2] = PWR_LOW_POWER_MODE_CFG_ITEM(1, 0b011, 1, 1),
    [PWR_MODE_STANDBY] = PWR_LOW_POWER_MODE_CFG_ITEM(1, 0b011, 1, 0),
    [PWR_MODE_SHUTDOWN] = PWR_LOW_POWER_MODE_CFG_ITEM(1, 0b100, 1, 0),
};

//------------------------------------------------------------------------------

static void pwr_main_regulator_enable(bool enable)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    (void)RCC->APB1ENR1;

    if (enable)
    {
        PWR->CR1 &= ~PWR_CR1_LPR;
        while (PWR->SR2 & PWR_SR2_REGLPF) {}
    }
    else
    {
        PWR->CR1 |= PWR_CR1_LPR;
        while (!(PWR->SR2 & PWR_SR2_REGLPF)) {}
    }
}

//------------------------------------------------------------------------------

void pwr_voltage_scaling_set(enum pwr_voltage_scaling range)
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
    (void)RCC->APB1ENR1; /* Allow the peripheral clock to become active */

    while (PWR->SR2 & PWR_SR2_VOSF) {}

    PWR->CR1 = (PWR->CR1 & ~PWR_CR1_VOS) | (range & PWR_CR1_VOS);

    while (PWR->SR2 & PWR_SR2_VOSF) {}
}

void pwr_sleep_mode_select(enum pwr_sleepdeep_mode mode)
{
    if ((unsigned)mode > PWR_MODE_SHUTDOWN)
        return;

    const struct pwr_low_power_mode_cfg *cfg = &pwr_low_power_mode[mode];
    pwr_main_regulator_enable(cfg->main_regulatror_enable);
    PWR->CR1 = (PWR->CR1 & ~PWR_CR1_LPMS) |
               ((uint32_t)cfg->lpms << PWR_CR1_LPMS_Pos);
    PWR->CR3 = (PWR->CR3 & ~PWR_CR3_RRS) |
               ((uint32_t)cfg->rrs << PWR_CR3_RRS_Pos);
    SCB->SCR = (SCB->SCR & ~SCB_SCR_SLEEPDEEP_Msk) |
               (cfg->sleepdeep ? SCB_SCR_SLEEPDEEP_Msk : 0);
}

void pwr_sleep_enter(bool wfi, bool sleeponexit)
{
    SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;
    if (sleeponexit)
        SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;

    __DSB();
    if (wfi)
        __WFI();
    else
        __WFE();
    __ISB();
}

//------------------------------------------------------------------------------
