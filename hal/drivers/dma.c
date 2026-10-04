//------------------------------------------------------------------------------

/// @file dma.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "dma.h"

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

struct dma_context
{
    DMA_Channel_TypeDef *channel_base;
    dma_tc_cb tc;
};

//------------------------------------------------------------------------------

static struct dma_context ctx[2][7] =
{
    [DMA_INSTANCE_1] =
    {
        [DMA_CHANNEL_1] = {.channel_base = DMA1_Channel1},
        [DMA_CHANNEL_2] = {.channel_base = DMA1_Channel2},
        [DMA_CHANNEL_3] = {.channel_base = DMA1_Channel3},
        [DMA_CHANNEL_4] = {.channel_base = DMA1_Channel4},
        [DMA_CHANNEL_5] = {.channel_base = DMA1_Channel5},
        [DMA_CHANNEL_6] = {.channel_base = DMA1_Channel6},
        [DMA_CHANNEL_7] = {.channel_base = DMA1_Channel7},
    },
    [DMA_INSTANCE_2] =
    {
        [DMA_CHANNEL_1] = {.channel_base = DMA2_Channel1},
        [DMA_CHANNEL_2] = {.channel_base = DMA2_Channel2},
        [DMA_CHANNEL_3] = {.channel_base = DMA2_Channel3},
        [DMA_CHANNEL_4] = {.channel_base = DMA2_Channel4},
        [DMA_CHANNEL_5] = {.channel_base = DMA2_Channel5},
        [DMA_CHANNEL_6] = {.channel_base = DMA2_Channel6},
        [DMA_CHANNEL_7] = {.channel_base = DMA2_Channel7},
    },
};

//------------------------------------------------------------------------------

static void dma_irq_handler(enum dma_instance inst, enum dma_channel channel, uint32_t tc_flag)
{
    DMA_TypeDef *base = inst == DMA_INSTANCE_1 ? DMA1 : DMA2;
    struct dma_context *context = &ctx[inst][channel];

    if (base->ISR & tc_flag)
    {
        base->IFCR = tc_flag;
        if (context->tc)
            context->tc();
    }
}

//------------------------------------------------------------------------------

void dma_init(enum dma_instance inst)
{
    RCC->AHB1ENR |= inst == DMA_INSTANCE_1 ? RCC_AHB1ENR_DMA1EN : RCC_AHB1ENR_DMA2EN;
    (void)RCC->AHB1ENR;
}

bool dma_channel_config(const struct dma_channel_cfg *cfg)
{
    if (!cfg || (uint32_t)cfg->inst > DMA_INSTANCE_2 || (uint32_t)cfg->channel > DMA_CHANNEL_7 ||
        (uint32_t)cfg->memory_size > DMA_DATA_SIZE_32_BIT ||
        (uint32_t)cfg->peripheral_size > DMA_DATA_SIZE_32_BIT || (uint32_t)cfg->direction > DMA_MEMORY_TO_PERIPHERAL ||
        (uint32_t)cfg->priority > DMA_PRIORITY_VERY_HIGH || cfg->count == 0 || cfg->count > 65535 ||
        !cfg->peripheral_address || !cfg->memory_address || (cfg->memory_to_memory && cfg->circular))
        return false;

    struct dma_context *context = &ctx[cfg->inst][cfg->channel];
    DMA_Channel_TypeDef *base = context->channel_base;
    
    base->CCR &= ~DMA_CCR_EN;
    base->CCR &= ~DMA_CCR_TCIE;
    context->tc = cfg->tc;

    base->CPAR = cfg->peripheral_address;
    base->CMAR = cfg->memory_address;
    base->CNDTR = cfg->count;
    base->CCR = ((uint32_t)cfg->memory_to_memory << DMA_CCR_MEM2MEM_Pos) |
                ((uint32_t)cfg->priority << DMA_CCR_PL_Pos) |
                ((uint32_t)cfg->memory_size << DMA_CCR_MSIZE_Pos) |
                ((uint32_t)cfg->peripheral_size << DMA_CCR_PSIZE_Pos) |
                ((uint32_t)cfg->memory_increment << DMA_CCR_MINC_Pos) |
                ((uint32_t)cfg->peripheral_increment << DMA_CCR_PINC_Pos) |
                ((uint32_t)cfg->circular << DMA_CCR_CIRC_Pos) |
                ((uint32_t)cfg->direction << DMA_CCR_DIR_Pos);

    const IRQn_Type irq[][7] =
    {
        [DMA_INSTANCE_1] =
        {
            [DMA_CHANNEL_1] = DMA1_Channel1_IRQn,
            [DMA_CHANNEL_2] = DMA1_Channel2_IRQn,
            [DMA_CHANNEL_3] = DMA1_Channel3_IRQn,
            [DMA_CHANNEL_4] = DMA1_Channel4_IRQn,
            [DMA_CHANNEL_5] = DMA1_Channel5_IRQn,
            [DMA_CHANNEL_6] = DMA1_Channel6_IRQn,
            [DMA_CHANNEL_7] = DMA1_Channel7_IRQn,
        },
        [DMA_INSTANCE_2] =
        {
            [DMA_CHANNEL_1] = DMA2_Channel1_IRQn,
            [DMA_CHANNEL_2] = DMA2_Channel2_IRQn,
            [DMA_CHANNEL_3] = DMA2_Channel3_IRQn,
            [DMA_CHANNEL_4] = DMA2_Channel4_IRQn,
            [DMA_CHANNEL_5] = DMA2_Channel5_IRQn,
            [DMA_CHANNEL_6] = DMA2_Channel6_IRQn,
            [DMA_CHANNEL_7] = DMA2_Channel7_IRQn,
        },
    };

    const uint32_t tc_flag[] =
    {
        [DMA_CHANNEL_1] = DMA_IFCR_CTCIF1,
        [DMA_CHANNEL_2] = DMA_IFCR_CTCIF2,
        [DMA_CHANNEL_3] = DMA_IFCR_CTCIF3,
        [DMA_CHANNEL_4] = DMA_IFCR_CTCIF4,
        [DMA_CHANNEL_5] = DMA_IFCR_CTCIF5,
        [DMA_CHANNEL_6] = DMA_IFCR_CTCIF6,
        [DMA_CHANNEL_7] = DMA_IFCR_CTCIF7,
    };

    if (context->tc)
    {
        DMA_TypeDef *dma_base = cfg->inst == DMA_INSTANCE_1 ? DMA1 : DMA2;
        dma_base->IFCR = tc_flag[cfg->channel];
        base->CCR |= DMA_CCR_TCIE;

        NVIC_EnableIRQ(irq[cfg->inst][cfg->channel]);
    }
    else
    {
        base->CCR &= ~DMA_CCR_TCIE;
        NVIC_DisableIRQ(irq[cfg->inst][cfg->channel]);
    }

    base->CCR |= DMA_CCR_EN;
    
    return true;
}

bool dma_channel_link(enum dma_instance inst, enum dma_channel channel, enum dma_request request)
{
    if ((uint32_t)inst > DMA_INSTANCE_2 || (uint32_t)channel > DMA_CHANNEL_7 || (uint32_t)request > DMA_REQUEST_7)
        return false;

    const uint32_t pos[] =
    {
        [DMA_CHANNEL_1] = DMA_CSELR_C1S_Pos,
        [DMA_CHANNEL_2] = DMA_CSELR_C2S_Pos,
        [DMA_CHANNEL_3] = DMA_CSELR_C3S_Pos,
        [DMA_CHANNEL_4] = DMA_CSELR_C4S_Pos,
        [DMA_CHANNEL_5] = DMA_CSELR_C5S_Pos,
        [DMA_CHANNEL_6] = DMA_CSELR_C6S_Pos,
        [DMA_CHANNEL_7] = DMA_CSELR_C7S_Pos,
    };

    DMA_Request_TypeDef *request_base = inst == DMA_INSTANCE_1 ? DMA1_CSELR : DMA2_CSELR;
    request_base->CSELR = (request_base->CSELR & ~(0b1111U << pos[channel])) | ((uint32_t)request << pos[channel]);

    return true;
}

//------------------------------------------------------------------------------

void DMA1_Channel1_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_1, DMA_CHANNEL_1, DMA_ISR_TCIF1);
}

void DMA1_Channel2_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_1, DMA_CHANNEL_2, DMA_ISR_TCIF2);
}

void DMA1_Channel3_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_1, DMA_CHANNEL_3, DMA_ISR_TCIF3);
}

void DMA1_Channel4_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_1, DMA_CHANNEL_4, DMA_ISR_TCIF4);
}

void DMA1_Channel5_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_1, DMA_CHANNEL_5, DMA_ISR_TCIF5);
}

void DMA1_Channel6_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_1, DMA_CHANNEL_6, DMA_ISR_TCIF6);
}

void DMA1_Channel7_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_1, DMA_CHANNEL_7, DMA_ISR_TCIF7);
}

void DMA2_Channel1_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_2, DMA_CHANNEL_1, DMA_ISR_TCIF1);
}

void DMA2_Channel2_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_2, DMA_CHANNEL_2, DMA_ISR_TCIF2);
}

void DMA2_Channel3_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_2, DMA_CHANNEL_3, DMA_ISR_TCIF3);
}

void DMA2_Channel4_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_2, DMA_CHANNEL_4, DMA_ISR_TCIF4);
}

void DMA2_Channel5_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_2, DMA_CHANNEL_5, DMA_ISR_TCIF5);
}

void DMA2_Channel6_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_2, DMA_CHANNEL_6, DMA_ISR_TCIF6);
}

void DMA2_Channel7_IRQHandler(void)
{
    dma_irq_handler(DMA_INSTANCE_2, DMA_CHANNEL_7, DMA_ISR_TCIF7);
}
