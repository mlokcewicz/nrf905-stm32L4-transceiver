//------------------------------------------------------------------------------

/// @file systick.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef SYSTICK_H_
#define SYSTICK_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

/// @brief Initializes SysTick with the requested interrupt period
/// @note Reinitialization discards the partially counted tick and preserves the millisecond counter
/// @param ms_per_tick Interrupt period in milliseconds
/// @return 0 if initialized successfully, 1 if the reload value is invalid
uint32_t systick_init(uint32_t ms_per_tick);

/// @brief Returns the current SysTick millisecond counter value
/// @return Milliseconds accumulated from completed ticks
uint32_t systick_get_ms(void);

/// @brief Checks whether the current millisecond count is greater than the timeout deadline
/// @note Does not handle millisecond counter overflow
/// @param base_ms Millisecond count at the start of the timeout
/// @param timeout_ms Timeout duration in milliseconds
/// @return true if the current millisecond count exceeds base_ms + timeout_ms, otherwise false
bool systick_timeout_elapsed(uint32_t base_ms, uint32_t timeout_ms);

/// @brief Blocks until the requested delay has elapsed
/// @note Requires SysTick interrupts to advance the counter; does not handle counter overflow
/// @param ms Delay duration in milliseconds; waits until the counter exceeds the deadline
void systick_delay(uint32_t ms);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* SYSTICK_H_ */

//------------------------------------------------------------------------------
