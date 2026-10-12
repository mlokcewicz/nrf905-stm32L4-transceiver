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

#include <stdbool.h>
#include <stdint.h>

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

enum pwr_voltage_scaling
{
    PWR_VOLTAGE_SCALING_RANGE1 = PWR_CR1_VOS_0,
    PWR_VOLTAGE_SCALING_RANGE2 = PWR_CR1_VOS_1,
};

enum pwr_sleepdeep_mode
{
    PWR_MODE_SLEEP,
    PWR_MODE_LOW_POWER,
    PWR_MODE_STOP_1_MR,
    PWR_MODE_STOP_1_LPR,
    PWR_MODE_STOP_2_LPR,
    PWR_MODE_STANDBY_LPR_SRAM2,
    PWR_MODE_STANDBY,
    PWR_MODE_SHUTDOWN,
};

//------------------------------------------------------------------------------

/// @brief Sets the main regulator voltage scaling range
/// @note Enables the PWR peripheral clock and waits for voltage scaling without a timeout
/// @note Call in main regulator Run mode; select Range 1 before increasing the clock above 26 MHz
/// @note Reduce the clock to at most 26 MHz before selecting Range 2; configure Flash latency separately
/// @param range PWR_VOLTAGE_SCALING_RANGE1 (up to 80 MHz) or PWR_VOLTAGE_SCALING_RANGE2 (up to 26 MHz)
void pwr_voltage_scaling_set(enum pwr_voltage_scaling range);

/// @brief Configures the Run regulator, LPMS, SRAM2 retention and SLEEPDEEP without sleeping
/// @note PWR_MODE_LOW_POWER requires Range 2 and SYSCLK <= 2 MHz beforehand.
/// @note Deep modes restore the main Run regulator first; hardware selects the sleep regulator.
/// @note Configure wakeup sources separately. Invalid enum values leave registers unchanged.
void pwr_sleep_mode_select(enum pwr_sleepdeep_mode mode);

/// @brief Executes WFI or WFE with the selected Cortex sleep settings
/// @param wfi true executes WFI; false executes WFE
/// @param sleeponexit Sets SLEEPONEXIT to sleep automatically on return to Thread mode
/// @note Select the mode with pwr_sleep_mode_select beforehand; SLEEPDEEP is preserved
/// @note Configure clocks and wakeup sources beforehand; does not change the regulator
/// @note Leaves the selected SCR bits set after returning; WFE consumes a pending event without sleeping
void pwr_sleep_enter(bool wfi, bool sleeponexit);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* PWR_H_ */

//------------------------------------------------------------------------------
