//------------------------------------------------------------------------------

/// @file pwr.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef PWR_H_
#define PWR_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdint.h>

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

#define PWR_VOLTAGE_SCALING_RANGE1 PWR_CR1_VOS_0
#define PWR_VOLTAGE_SCALING_RANGE2 PWR_CR1_VOS_1

//------------------------------------------------------------------------------

/// @brief Initializes power control with the selected main regulator voltage scaling range
/// @note Enables the PWR peripheral clock and waits for voltage scaling without a timeout
/// @note Call in main regulator Run mode; select Range 1 before increasing the clock above 26 MHz
/// @note Reduce the clock to at most 26 MHz before selecting Range 2; configure Flash latency separately
/// @param range PWR_VOLTAGE_SCALING_RANGE1 (up to 80 MHz) or PWR_VOLTAGE_SCALING_RANGE2 (up to 26 MHz)
void pwr_init(uint32_t range);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* PWR_H_ */

//------------------------------------------------------------------------------
