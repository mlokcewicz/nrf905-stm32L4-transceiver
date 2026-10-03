//------------------------------------------------------------------------------

/// @file dma.h
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#ifndef DMA_H_
#define DMA_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>

//------------------------------------------------------------------------------

typedef void (*dma_tc_cb)(void);

//------------------------------------------------------------------------------

enum dma_instance
{
    DMA_INSTANCE_1,
    DMA_INSTANCE_2,
};

enum dma_channel
{
    DMA_CHANNEL_1,
    DMA_CHANNEL_2,
    DMA_CHANNEL_3,
    DMA_CHANNEL_4,
    DMA_CHANNEL_5,
    DMA_CHANNEL_6,
    DMA_CHANNEL_7,
};

enum dma_data_size
{
    DMA_DATA_SIZE_8_BIT = 0b00,
    DMA_DATA_SIZE_16_BIT = 0b01,
    DMA_DATA_SIZE_32_BIT = 0b10,
};

enum dma_direction
{
    DMA_PERIPHERAL_TO_MEMORY,
    DMA_MEMORY_TO_PERIPHERAL,
};

enum dma_priority
{
    DMA_PRIORITY_LOW,
    DMA_PRIORITY_MEDIUM,
    DMA_PRIORITY_HIGH,
    DMA_PRIORITY_VERY_HIGH,
};

enum dma_request
{
    DMA_REQUEST_0,
    DMA_REQUEST_1,
    DMA_REQUEST_2,
    DMA_REQUEST_3,
    DMA_REQUEST_4,
    DMA_REQUEST_5,
    DMA_REQUEST_6,
    DMA_REQUEST_7,
};

struct dma_channel_cfg
{
    enum dma_instance inst;
    enum dma_channel channel;
    uintptr_t peripheral_address;
    uintptr_t memory_address;
    uint32_t count;
    enum dma_data_size peripheral_size;
    enum dma_data_size memory_size;
    enum dma_direction direction;
    enum dma_priority priority;
    bool memory_increment;
    bool peripheral_increment;
    bool circular;
    bool memory_to_memory;
    dma_tc_cb tc; 
};

/// @brief Enables the clock for the selected DMA controller
/// @param inst DMA controller
void dma_init(enum dma_instance inst);

/// @brief Configures and enables the selected DMA channel
/// @param cfg Channel configuration; count is 1 to 65535 data items, not bytes
/// @return false for invalid configuration, otherwise true
bool dma_channel_config(const struct dma_channel_cfg *cfg);

/// @brief Selects the peripheral request for a disabled DMA channel through CSELR
/// @param inst DMA controller
/// @param channel DMA channel
/// @param request Request selection for the selected controller and channel
/// @return false for invalid arguments or an enabled channel, otherwise true
bool dma_channel_link(enum dma_instance inst, enum dma_channel channel, enum dma_request request);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* DMA_H_ */

//------------------------------------------------------------------------------
