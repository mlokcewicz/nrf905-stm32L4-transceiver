//------------------------------------------------------------------------------

/// @file spi.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef SPI_H_
#define SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

typedef void (*spi_transmit_complete_cb)(uint8_t received_byte);
typedef bool (*spi_io_init_cb)(void); // Return true if I/O initialization succeeds
typedef void (*spi_cs_set_cb)(bool state); // Physical CS level: false = selected, true = inactive

//------------------------------------------------------------------------------

enum spi_instance
{
    SPI_INSTANCE_1,
    SPI_INSTANCE_2,
    SPI_INSTANCE_3,
};

/* Encoded SPI_CR2.DS values: frame size minus one. */
enum spi_data_size
{
    SPI_DATASIZE_4_BIT = 3,
    SPI_DATASIZE_5_BIT = 4,
    SPI_DATASIZE_6_BIT = 5,
    SPI_DATASIZE_7_BIT = 6,
    SPI_DATASIZE_8_BIT = 7,
    SPI_DATASIZE_9_BIT = 8,
    SPI_DATASIZE_10_BIT = 9,
    SPI_DATASIZE_11_BIT = 10,
    SPI_DATASIZE_12_BIT = 11,
    SPI_DATASIZE_13_BIT = 12,
    SPI_DATASIZE_14_BIT = 13,
    SPI_DATASIZE_15_BIT = 14,
    SPI_DATASIZE_16_BIT = 15,
};

enum spi_clock_presc
{
    SPI_CLOCK_PRESC_2 = 0b000,
    SPI_CLOCK_PRESC_4 = 0b001,
    SPI_CLOCK_PRESC_8 = 0b010,
    SPI_CLOCK_PRESC_16 = 0b011,
    SPI_CLOCK_PRESC_32 = 0b100,
    SPI_CLOCK_PRESC_64 = 0b101,
    SPI_CLOCK_PRESC_128 = 0b110,
    SPI_CLOCK_PRESC_256 = 0b111,
};

//------------------------------------------------------------------------------

struct spi_cfg
{
    enum spi_instance inst;
    enum spi_clock_presc clock_presc;
    enum spi_data_size data_size;
    bool cpol;
    bool cpha;
    bool lsb_first;
    bool hw_cs;

    spi_io_init_cb io_init;
    spi_cs_set_cb cs_set;
    spi_transmit_complete_cb tc_cb;
};

struct spi_obj
{
    enum spi_instance inst;
    spi_cs_set_cb cs_set;
    spi_transmit_complete_cb tc_cb;
};

//------------------------------------------------------------------------------

/// @brief Initializes the SPI object, peripheral clock and optional I/O callbacks
/// @param obj Pointer to the SPI instance object @ref spi_obj
/// @param cfg configuration structure pointer
/// @return true if initialized successfully, otherwise false
bool spi_init(struct spi_obj *obj, struct spi_cfg *cfg);

/// @brief Exchanges transmit and receive buffers using blocking polling
/// @param obj Pointer to an initialized SPI instance object @ref spi_obj
/// @param data_rx Receive buffer with space for len bytes
/// @param data_tx Transmit buffer containing len bytes
/// @param len Number of byte-buffer entries to exchange; zero performs no transfer
/// @return Always 0, including when a pointer is NULL or len is zero; received data is stored in data_rx
uint8_t spi_transmit(struct spi_obj *obj, uint8_t *data_rx, uint8_t *data_tx, uint32_t len);

/// @brief Sets or resets SS pin
/// @param obj Pointer to the SPI instance object @ref spi_obj
/// @param state Physical SS level: false selects the device, true deselects it
void spi_cs_set(struct spi_obj *obj, bool state);

/// @brief Resets the SPI instance object
/// @param obj Pointer to the SPI instance object @ref spi_obj
void spi_deinit(struct spi_obj *obj);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* SPI_H_ */

//------------------------------------------------------------------------------
