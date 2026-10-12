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

/// @brief Initializes system clocks, SysTick, watchdog, LED, UART4, SPI and the enabled nRF905 radios
/// @note Configures MSI at 2 MHz and UART4 at 115200 baud using HSI16; prints radio diagnostics
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

/// @brief Enters Low-power sleep until an interrupt becomes pending
/// @note Requires hal_init(); keeps the 2 MHz clock and peripheral DMA paths active
/// @note Returns to Low-power run; SysTick can wake the CPU every millisecond
void hal_sleep(void);

/// @brief Blocks until UART4 DMA receives payload bytes
/// @note Requires hal_init(), active interrupts and the RX DMA channel assigned to UART4
void hal_uart_rx_start(void);

/// @brief Enables nRF RX reception and configures SPI2 DMA for a hardware-triggered payload read
/// @note Requires successful hal_init(); the hardware sequence is:
/// DR rises -> TIM8 CH2 captures TI1 indirectly and requests DMA2 channel 7 -> DMA sets SPI2 CR1.SPE -> SPI drives CS low.
/// SPI TXE requests TX DMA to write the read command and dummy bytes; received bytes trigger RX DMA via RXNE.
/// After the payload is read, DR falls -> TIM8 CH2 requests DMA2 channel 7 -> DMA clears SPI2 CR1.SPE.
/// SPI releases NSS and the pull-up raises CS high, ending the transaction.
void hal_radio_rx_config(void);

/// @brief Configures TIM4 and DMA to clear transmitter CE and TX_EN on a rising DR edge
/// @note Requires hal_init() and transmitter DR connected to PB6; auto retransmit must be disabled
void hal_radio_tx_config(void);

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
