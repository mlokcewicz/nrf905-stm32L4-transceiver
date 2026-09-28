//------------------------------------------------------------------------------

/// @file spi.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "spi.h"

#include <string.h>

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

static SPI_TypeDef *spi_base[] =
{
    [SPI_INSTANCE_1] = SPI1,
    [SPI_INSTANCE_2] = SPI2,
    [SPI_INSTANCE_3] = SPI3,
};

//------------------------------------------------------------------------------

bool spi_init(struct spi_obj *obj, struct spi_cfg *cfg)
{
    if (!obj || !cfg || (uint32_t)cfg->inst > SPI_INSTANCE_3 || (uint32_t)cfg->data_size < SPI_DATASIZE_4_BIT || (uint32_t)cfg->data_size > SPI_DATASIZE_16_BIT)
        return false;

    const uintptr_t spi_clock_enable[][2] =
    {
        [SPI_INSTANCE_1] = {(uintptr_t)&RCC->APB2ENR, RCC_APB2ENR_SPI1EN},
        [SPI_INSTANCE_2] = {(uintptr_t)&RCC->APB1ENR1, RCC_APB1ENR1_SPI2EN},
        [SPI_INSTANCE_3] = {(uintptr_t)&RCC->APB1ENR1, RCC_APB1ENR1_SPI3EN},
    };

    if (cfg->io_init && !cfg->io_init())
        return false;

    obj->inst = cfg->inst;
    obj->tc_cb = cfg->tc_cb;
    obj->cs_set = cfg->cs_set;

    SPI_TypeDef *base = spi_base[obj->inst];

    /* Enable the selected SPI peripheral clock. */
    volatile uint32_t *clock_reg = (volatile uint32_t *)spi_clock_enable[obj->inst][0];
    *clock_reg |= (uint32_t)spi_clock_enable[obj->inst][1];
    (void)*clock_reg; /* Allow the peripheral clock to become active */

    /* Disable SPI before configuring its registers. */
    base->CR1 &= ~SPI_CR1_SPE;
    spi_cs_set(obj, true);

    base->CR1 &= ~SPI_CR1_LSBFIRST;
    base->CR1 |= cfg->lsb_first << SPI_CR1_LSBFIRST_Pos;

    base->CR1 &= ~SPI_CR1_BR;
    base->CR1 |= cfg->clock_presc << SPI_CR1_BR_Pos;

    base->CR1 |= SPI_CR1_MSTR;
  
    cfg->cpol ? (base->CR1 |= SPI_CR1_CPOL) : (base->CR1 &= ~SPI_CR1_CPOL);     
    cfg->cpha ? (base->CR1 |= SPI_CR1_CPHA) : (base->CR1 &= ~SPI_CR1_CPHA);     

    if (cfg->hw_cs)
    {
        base->CR1 &= ~SPI_CR1_SSM;
        base->CR2 |= SPI_CR2_SSOE;
    }
    else
    {
        base->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
        base->CR2 &= ~SPI_CR2_SSOE;
    }    
    
    base->CR2 &= ~SPI_CR2_DS;
    base->CR2 |= (uint32_t)cfg->data_size << SPI_CR2_DS_Pos;

    base->CR2 |= SPI_CR2_FRXTH;

    /* Enable SPI */
    base->CR1 |= SPI_CR1_SPE;

    return true;
}

uint8_t spi_transmit(struct spi_obj *obj, uint8_t *data_rx, uint8_t *data_tx, uint32_t len)
{
    if (!obj || !data_tx || !data_rx || len == 0)
        return 0;

    SPI_TypeDef *base = spi_base[obj->inst];
    int32_t pos = 0;

    if (obj->cs_set) obj->cs_set(false);
    
    while (len--)
    {
        while (!(base->SR & SPI_SR_TXE)) {};
        
        *(volatile uint8_t *)&base->DR = data_tx[pos];
        
        while (!(base->SR & SPI_SR_RXNE)) {};

        data_rx[pos] = *(volatile uint8_t *)&base->DR;
        
        pos++;
    }

    while (base->SR & SPI_SR_BSY) {}
    
    if (obj->cs_set) obj->cs_set(true);
    
    return 0;
}

void spi_cs_set(struct spi_obj *obj, bool state)
{
    if (obj && obj->cs_set)
        obj->cs_set(state);
}

void spi_deinit(struct spi_obj *obj)
{
    if (!obj)
        return;

    memset(obj, 0x00, sizeof(*obj));
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
