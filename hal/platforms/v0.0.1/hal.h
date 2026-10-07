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

/// @brief Initializes system clocks, SysTick, watchdog, LED, USART2, SPI and the enabled nRF905 radios
/// @note Configures the system clock to 40 MHz and USART2 to 115200 baud; prints radio diagnostics
/// @note Starts the watchdog with a nominal 32.768 s timeout; initialization failure does not stop it
/// @return true if USART, SPI and radio initialization succeeds, otherwise false
bool hal_init(void);

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

/// @brief Blocks until USART2 DMA receives payload bytes
/// @note Requires hal_init(), active interrupts and the RX DMA channel assigned to USART2
void hal_uart_wait_for_msg(void);

/// @brief Enables nRF RX reception and configures SPI3 DMA for a hardware-triggered payload read
/// @note Requires successful hal_init(); the hardware sequence is:
/// DR rises -> TIM3 input capture requests DMA1 channel 6 -> DMA sets SPI3 CR1.SPE -> SPI drives CS low.
/// SPI TXE requests TX DMA to write the read command and dummy bytes; received bytes trigger RX DMA via RXNE.
/// After the payload is read, DR falls -> TIM3 requests DMA1 channel 6 -> DMA clears SPI3 CR1.SPE.
/// SPI releases NSS and the pull-up raises CS high, ending the transaction.
void hal_radio_rx_config(void);

/// @brief Reloads the nRF RX configuration and configures SPI3 DMA for a hardware-triggered payload read
/// @note Requires successful hal_init() and a previously configured nRF RX instance
void hal_radio_rx_config_reload(void);

/// @brief Sends a 32-byte payload from nRF TX to the configured radio address
/// @note Requires hal_init(), nRF TX in standby; when the receiver is enabled, its hardware reception and SPI TX DMA must be ready
void hal_radio_tx_send(void);

/// @brief Runs a blocking radio test transmitting RADIOMSG from nRF TX to nRF RX
/// @note Requires successful hal_init(); waits 1000 ms before reading and printing receiver status and data
/// @note Does not check operation results, restore radio modes or refresh the watchdog
void hal_radio_loopback_test(void);

/// @brief Runs a blocking radio test transmitting RADIOMSG from nRF TX to nRF RX 
/// @note Requires successful hal_init(); use pure HW-driven receptrion using DMA requests from DR pin over Timer
/// @note Does not check operation results, restore radio modes or refresh the watchdog
void hal_radio_loopback_test_hw_driven(void);

//------------------------------------------------------------------------------

#endif /* HAL_H_ */
