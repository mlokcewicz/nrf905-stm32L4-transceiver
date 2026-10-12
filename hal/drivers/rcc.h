//------------------------------------------------------------------------------

/// @file rcc.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef RCC_H_
#define RCC_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

#define RCC_PLLM_DIV1 0
#define RCC_PLLM_DIV2 1
#define RCC_PLLM_DIV3 2
#define RCC_PLLM_DIV4 3
#define RCC_PLLM_DIV5 4
#define RCC_PLLM_DIV6 5
#define RCC_PLLM_DIV7 6
#define RCC_PLLM_DIV8 7

#define RCC_PLLR_DIV2 0
#define RCC_PLLR_DIV4 1
#define RCC_PLLR_DIV6 2
#define RCC_PLLR_DIV8 3

//------------------------------------------------------------------------------

struct main_pll_config
{
    bool main_pll_on;
    uint8_t pllr;    
    uint8_t pllren;
    uint8_t plln;
    uint8_t pllm;    
    uint8_t pllsrc;
};

struct rcc_config
{
    bool lsi_on;
    bool msi_on;
    uint32_t msi_range;
    uint32_t msi_standby_range;
    bool hsi_on;
    bool hse_on;
    uint32_t ahb_presc;
    uint32_t apb1_presc;
    uint32_t apb2_presc;
    uint8_t sysclk_source;

    struct main_pll_config pll;
};

struct rcc_sleep_clock_config
{
    uint32_t ahb1;
    uint32_t ahb2;
    uint32_t apb1;
    uint32_t apb2;
};

//------------------------------------------------------------------------------

/// @brief Configures MSI, LSI, HSI, HSE, bus prescalers, the main PLL and SYSCLK
/// @note Blocks until enabled oscillators and the main PLL are ready, without a timeout
/// @note Temporarily selects MSI before stopping the main PLL; waits for SYSCLK switches.
/// @note Caller configures voltage and Flash latency for current MSI and target frequencies.
/// @note Selected SYSCLK and PLL sources must be enabled in cfg. Does not update SystemCoreClock.
/// @param cfg Pointer to the clock configuration structure @ref rcc_config
void rcc_init(struct rcc_config *cfg);

/// @brief Selects the normal MSI range from RCC_CR instead of the startup range from RCC_CSR
/// @note Set RCC_CR.MSIRANGE and suitable voltage/Flash latency before switching.
/// @note MSI must be off or ready. Does not update SystemCoreClock.
/// @note Software cannot switch back to the CSR range; reset or Standby clears MSIRGSEL.
void rcc_msi_range_restore_from_standby(void);

/// @brief Keeps the selected buses' peripheral and memory clocks enabled during Sleep
/// @param cfg Masks of RCC_AHB1SMENR, AHB2SMENR, APB1SMENR1 and APB2SMENR bits
void rcc_sleep_clocks_enable(const struct rcc_sleep_clock_config *cfg);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* RCC_H_ */

//------------------------------------------------------------------------------
