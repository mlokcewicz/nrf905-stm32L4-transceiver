//------------------------------------------------------------------------------

/// @file flash.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef FLASH_H_
#define FLASH_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdint.h>

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

/// @brief Initializes Flash memory access with the selected number of wait states
/// @note Increase latency before increasing HCLK; decrease it after decreasing HCLK
/// @note Select latency according to HCLK and the voltage range; waits for readback without a timeout
/// @param latency Flash latency value from FLASH_ACR_LATENCY_0WS through FLASH_ACR_LATENCY_4WS
void flash_init(uint32_t latency);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* FLASH_H_ */

//------------------------------------------------------------------------------
