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

bool hal_init(void);

void hal_radio_test(void);

void hal_led_toggle(void);

void hal_wdg_feed(void);

void hal_delay(uint32_t ms);

//------------------------------------------------------------------------------

#endif /* HAL_H_ */
