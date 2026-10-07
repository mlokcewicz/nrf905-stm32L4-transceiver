//------------------------------------------------------------------------------

/// @file usart.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef USART_H_
#define USART_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

typedef bool (*usart_io_init_cb)(void); // Return true if I/O initialization succeeds
typedef bool (*usart_udre_cb)(volatile uint8_t *data_to_send); // Return true to continue transmission
typedef bool (*usart_rxc_cb)(volatile uint8_t data_received, volatile bool error); // Return true to continue reception
typedef void (*usart_txc_cb)(void);
typedef void (*usart_dma_config_cb)(uintptr_t periph_address, uint8_t *data, uint32_t len, bool is_rx); 

//------------------------------------------------------------------------------

enum usart_instance
{
    USART_INSTANCE_1,
    USART_INSTANCE_2,
    USART_INSTANCE_3,
    USART_INSTANCE_4,
    USART_INSTANCE_5,
};

enum usart_rx_tx
{
    USART_TX_ENABLE = 1 << 0,
    USART_RX_ENABLE = 1 << 1,
};

enum usart_data_size
{
    USART_DATASIZE_7_BIT = 0b10,
    USART_DATASIZE_8_BIT = 0b00,
    USART_DATASIZE_9_BIT = 0b01,
};

enum usart_parity
{
    USART_PARITY_DISABLED = 0b00,
    USART_PARITY_EVEN = 0b01,
    USART_PARITY_ODD = 0b11,
};

enum usart_oversampling
{
    USART_OVERSAMPLING_16 = 0,
    USART_OVERSAMPLING_8 = 1,
};

enum usart_stop_bits
{
    USART_STOP_BITS_0_5 = 0b01,
    USART_STOP_BITS_1 = 0b00,
    USART_STOP_BITS_1_5 = 0b11,
    USART_STOP_BITS_2 = 0b10,
};

//------------------------------------------------------------------------------

struct usart_cfg
{
    enum usart_instance inst;
    enum usart_rx_tx rx_tx;
    enum usart_data_size data_size;
    enum usart_parity parity;
    enum usart_stop_bits stop_bits;
    enum usart_oversampling oversampling; 

    uint32_t baudrate;

    usart_io_init_cb io_init;
    usart_udre_cb udre_cb;
    usart_rxc_cb rxc_cb;
    usart_txc_cb txc_cb;
    usart_dma_config_cb dma_config;
};

struct usart_obj 
{
    enum usart_instance inst;
    usart_udre_cb udre_cb;
    usart_rxc_cb rxc_cb;
    usart_txc_cb txc_cb;
    usart_dma_config_cb dma_config;
};

//------------------------------------------------------------------------------

/// @brief Initializes USART module according to given configuration
/// @note Derives the USART clock from RCC and sets baud rate with the configured oversampling, using integer division without rounding
/// @note Requires an up-to-date SystemCoreClock for PCLK/SYSCLK sources; clock configuration must be stable
/// @note Calls cfg->io_init, if provided, after argument validation; returns false immediately if the callback fails
/// @note Enables the peripheral clock; enable the selected clock source before calling or in io_init; oscillator constants must match hardware
/// @note If RXC or TXC callbacks are provided, send and receive functions only enables suitable inerrupts
/// @param obj Pointer to the USART instance object @ref usart_obj
/// @param cfg configuration structure pointer @ref struct usart_cfg
/// @return true on success, false for invalid arguments, I/O initialization failure, unsupported data size or baud rate, or an unavailable clock
bool usart_init(struct usart_obj *obj, struct usart_cfg *cfg);

/// @brief Sends given data if polling mode is chosen
/// @note If TXC callback is provided, this function only enables TXC interrupt
/// @param obj Pointer to the USART instance object @ref usart_obj
/// @param data data to send pointer
/// @param len data to send length 
void usart_send(struct usart_obj *obj, uint8_t *data, uint32_t len);

/// @brief Waits for the transmission to finish and disables the USART peripheral
/// @param obj Pointer to the USART instance object @ref usart_obj
void usart_send_finish(struct usart_obj *obj);

/// @brief Receives given amount of data and stores in buffer pointed by data
/// @param obj Pointer to the USART instance object @ref usart_obj
/// @param data pointer to input buffer
/// @param len data to receive length 
/// @return true if received successfully, false in case of parity / overrun / frame errors
bool usart_receive(struct usart_obj *obj, uint8_t *data, uint32_t len);

/// @brief Sends one byte using blocking polling
/// @note Enables USART, waits for the transmit register to be empty and for transmission to complete; leaves USART enabled
/// @note Requires an initialized transmitter and no active DMA transmission; no timeout
/// @param obj Pointer to the USART instance object
/// @param ch character to send
void usart_putc(struct usart_obj *obj, char ch);

/// @brief Sends given data if polling mode is chosen
/// @note If TXC callback is provided, this function only enables TXC interrupt
/// @param obj Pointer to the USART instance object @ref usart_obj
/// @param str - null-terminated string to send pointer
void usart_print(struct usart_obj *obj, char *str);

/// @brief Resets the USART instance object
/// @param obj Pointer to the USART instance object @ref usart_obj
void usart_deinit(struct usart_obj *obj);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* USART_H_ */

//------------------------------------------------------------------------------
