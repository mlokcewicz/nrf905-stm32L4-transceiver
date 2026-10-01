//------------------------------------------------------------------------------

/// @file hal.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef HAL_H_
#define HAL_H_

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

/// @brief Initializes system clocks, SysTick, watchdog, LED, USART2, SPI2/SPI3 and both nRF905 radios
/// @note Configures the system clock to 40 MHz and USART2 to 115200 baud; prints radio diagnostics
/// @note Starts the watchdog with a nominal 32.768 s timeout; initialization failure does not stop it
/// @return true if USART, SPI and radio initialization succeeds, otherwise false
bool hal_init(void);

/// @brief Runs a blocking radio test transmitting RADIOMSG from nRF1 to nRF2
/// @note Requires successful hal_init(); waits 1000 ms before reading and printing receiver status and data
/// @note Does not check operation results, restore radio modes or refresh the watchdog
void hal_radio_test(void);

/// @brief Toggles the LED output on PA5
/// @note Requires GPIO initialization by hal_init()
void hal_led_toggle(void);

/// @brief Reloads the independent watchdog counter
/// @note Call periodically after hal_init(), before the watchdog timeout expires
void hal_wdg_feed(void);

/// @brief Blocks for the requested delay using the SysTick millisecond counter
/// @note Requires initialized SysTick and active interrupts; does not refresh the watchdog
/// @param ms Delay duration in milliseconds
void hal_delay(uint32_t ms);

//------------------------------------------------------------------------------

#endif /* HAL_H_ */
