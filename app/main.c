//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stdio.h>

#include <stm32l4xx.h>

#include <systick.h>
#include <rcc.h>
#include <gpio.h>

//------------------------------------------------------------------------------

int main()
{
    NVIC_SetPriorityGrouping(0x00000003U);
    
    /* SYSCLK = 4 MHz, source: MSI, AHB presc = /1 → HCLK = 4 MHz APB1/APB2 presc = /1 → PCLK1 = PCLK2 = 4 MHz */
    SystemInit();

    struct rcc_config cfg = 
    {
        .lsi_on = true,
        .hsi_on = true,
        .hse_on = false,
        .pll.main_pll_on = true,
        .pll.pllm = RCC_PLLM_DIV2,
        .pll.plln = 10,
        .pll.pllr = RCC_PLLR_DIV2,
        .pll.pllren = true,
        .pll.pllsrc = RCC_PLLCFGR_PLLSRC_HSI,
        .ahb_presc = RCC_CFGR_HPRE_DIV1,
        .apb1_presc = RCC_CFGR_PPRE1_DIV1,
        .apb2_presc = RCC_CFGR_PPRE2_DIV1,
        .sysclk_source = RCC_CFGR_SW_PLL,
    };

    rcc_init(&cfg);

    SystemCoreClockUpdate();

    systick_init();

    GPIO_CONFIG(A, 5, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    
    while (1)
    {
        GPIO_TOGGLE(A, 5);
        systick_delay(500);
    }

    return 0;
}

//------------------------------------------------------------------------------

/*
CE = standby, (hi - on)
TXE - (hi - tx, low - rx)
PWR - (hi - on)
DR - (hi - tx finished or new data received)
SO - MISO
SI - MOSI
SCK - clock
CSN - CS


*/
