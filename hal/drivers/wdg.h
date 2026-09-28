//------------------------------------------------------------------------------

/// @file wdg.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef WDG_H_
#define WDG_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

#define IWDG_PRESCALER_4                0x00000000u                   
#define IWDG_PRESCALER_8                IWDG_PR_PR_0                  
#define IWDG_PRESCALER_16               IWDG_PR_PR_1                  
#define IWDG_PRESCALER_32               (IWDG_PR_PR_1 | IWDG_PR_PR_0) 
#define IWDG_PRESCALER_64               IWDG_PR_PR_2                  
#define IWDG_PRESCALER_128              (IWDG_PR_PR_2 | IWDG_PR_PR_0) 
#define IWDG_PRESCALER_256              (IWDG_PR_PR_2 | IWDG_PR_PR_1) 

//------------------------------------------------------------------------------

/// @brief Starts and configures the independent watchdog, then reloads its counter
/// @note Waits for prescaler and reload register updates without a timeout
/// @note Timeout in seconds is (reload + 1) * prescaler divisor / LSI frequency in Hz
/// @note Once started, the watchdog cannot be disabled by software; feed it before timeout to avoid reset
/// @param prescaler IWDG_PRESCALER_* value selecting an LSI clock divisor from 4 to 256
/// @param reload Counter reload value from 0 to 4095; parameters are not validated
void wdg_init(uint32_t prescaler, uint32_t reload);

/// @brief Reloads the independent watchdog counter to prevent a watchdog reset
/// @note Call periodically after wdg_init(), before the configured timeout expires
void wdg_feed(void);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* WDG_H_ */

//------------------------------------------------------------------------------
