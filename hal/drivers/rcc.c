//------------------------------------------------------------------------------

/// @file rcc.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "rcc.h"

//------------------------------------------------------------------------------

void rcc_init(struct rcc_config *cfg)
{   
    /* LSI on / off */
    RCC->CSR &= ~RCC_CSR_LSION;
    RCC->CSR |= cfg->lsi_on << RCC_CSR_LSION_Pos;
    
    while (cfg->lsi_on && !(RCC->CSR & RCC_CSR_LSIRDY)) {};
 
    /* HSI on / off */
    RCC->CR &= ~RCC_CR_HSION;
    RCC->CR |= cfg->hsi_on << RCC_CR_HSION_Pos;
    
    while (cfg->hsi_on && !(RCC->CR & RCC_CR_HSIRDY)) {};
 
    /* HSE on / off */
    RCC->CR &= ~RCC_CR_HSEON;
    RCC->CR |= cfg->hse_on << RCC_CR_HSEON_Pos;

    while (cfg->hse_on && !(RCC->CR & RCC_CR_HSERDY)) {};

    /* APHB prescaler */
    RCC->CFGR &= ~RCC_CFGR_HPRE;
    RCC->CFGR |= cfg->ahb_presc << RCC_CFGR_HPRE_Pos;

    /* APB1 prescaler */
    RCC->CFGR &= ~RCC_CFGR_PPRE1;
    RCC->CFGR |= cfg->apb1_presc << RCC_CFGR_PPRE1_Pos;

    /* APB2 prescaler */
    RCC->CFGR &= ~RCC_CFGR_PPRE2;
    RCC->CFGR |= cfg->apb2_presc << RCC_CFGR_PPRE2_Pos;

    /* Main PLL config */
    RCC->CR &= ~RCC_CR_PLLON;

    /* PLLR */
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLR;
    RCC->PLLCFGR |= cfg->pll.pllr << RCC_PLLCFGR_PLLR_Pos;

    /* PLLCLK enable */
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLREN;
    RCC->PLLCFGR |= cfg->pll.pllren << RCC_PLLCFGR_PLLREN_Pos;
    
    /* PLLN */
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;
    RCC->PLLCFGR |= cfg->pll.plln << RCC_PLLCFGR_PLLN_Pos;
    
    /* PLLM */
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;
    RCC->PLLCFGR |= cfg->pll.pllm << RCC_PLLCFGR_PLLM_Pos;
    
    /* PLLSRC */
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;
    RCC->PLLCFGR |= cfg->pll.pllsrc << RCC_PLLCFGR_PLLSRC_Pos;
    
    RCC->CR |= cfg->pll.main_pll_on << RCC_CR_PLLON_Pos;

    while (cfg->pll.main_pll_on && !(RCC->CR & RCC_CR_PLLRDY)) {};

    /* SYSCLK switch */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= cfg->sysclk_source << RCC_CFGR_SW_Pos;

    while ((RCC->CFGR & RCC_CFGR_SWS) != ((uint32_t)cfg->sysclk_source << RCC_CFGR_SWS_Pos)) {}
}

//------------------------------------------------------------------------------
