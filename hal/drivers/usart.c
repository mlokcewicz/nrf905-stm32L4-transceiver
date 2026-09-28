//------------------------------------------------------------------------------

/// @file usart.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "usart.h"

#include <string.h>

#include <stm32l4xx.h>

#ifndef HSI_VALUE
#define HSI_VALUE 16000000U
#endif

#ifndef LSE_VALUE
#define LSE_VALUE 32768U
#endif

//------------------------------------------------------------------------------

static USART_TypeDef * usart_base[] =
{
    [USART_INSTANCE_1] = USART1,
    [USART_INSTANCE_2] = USART2,
    [USART_INSTANCE_3] = USART3,
    [USART_INSTANCE_4] = UART4,
    [USART_INSTANCE_5] = UART5,
};

//------------------------------------------------------------------------------

static uint32_t usart_get_clock_hz(enum usart_instance inst)
{
    const uint8_t clock_source_pos[] =
    {
        [USART_INSTANCE_1] = RCC_CCIPR_USART1SEL_Pos,
        [USART_INSTANCE_2] = RCC_CCIPR_USART2SEL_Pos,
        [USART_INSTANCE_3] = RCC_CCIPR_USART3SEL_Pos,
        [USART_INSTANCE_4] = RCC_CCIPR_UART4SEL_Pos,
        [USART_INSTANCE_5] = RCC_CCIPR_UART5SEL_Pos,
    };

    uint32_t source = (RCC->CCIPR >> clock_source_pos[inst]) & 0x3U; /* Extract the two-bit USARTxSEL/UARTxSEL field */

    if (source == 2U) /* 10: HSI16 selected as USART clock */
        return (RCC->CR & RCC_CR_HSIRDY) ? HSI_VALUE : 0U;
    if (source == 3U) /* 11: LSE selected as USART clock */
        return (RCC->BDCR & RCC_BDCR_LSERDY) ? LSE_VALUE : 0U;

    uint32_t cfgr = RCC->CFGR;

    if (source == 1U) /* 01: SYSCLK selected as USART clock */
    {
        /* Undo the AHB division to obtain SYSCLK. */
        uint32_t hpre = (cfgr & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos;
        return SystemCoreClock << AHBPrescTable[hpre];
    }

    /* 00: PCLK selected; USART1 uses PCLK2, USART2/3 and UART4/5 use PCLK1. */
    uint32_t ppre = (inst == USART_INSTANCE_1) ? (cfgr & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos : (cfgr & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos;

    return SystemCoreClock >> APBPrescTable[ppre];
}

//------------------------------------------------------------------------------

bool usart_init(struct usart_obj *obj, struct usart_cfg *cfg)
{
    if (!obj || !cfg || cfg->data_size == USART_DATASIZE_9_BIT || (uint32_t)cfg->inst > USART_INSTANCE_5 || cfg->baudrate == 0U || (uint32_t)cfg->oversampling > USART_OVERSAMPLING_8)
        return false;

    const uintptr_t usart_clock_enable[][2] =
    {
        [USART_INSTANCE_1] = {(uintptr_t)&RCC->APB2ENR, RCC_APB2ENR_USART1EN},
        [USART_INSTANCE_2] = {(uintptr_t)&RCC->APB1ENR1, RCC_APB1ENR1_USART2EN},
        [USART_INSTANCE_3] = {(uintptr_t)&RCC->APB1ENR1, RCC_APB1ENR1_USART3EN},
        [USART_INSTANCE_4] = {(uintptr_t)&RCC->APB1ENR1, RCC_APB1ENR1_UART4EN},
        [USART_INSTANCE_5] = {(uintptr_t)&RCC->APB1ENR1, RCC_APB1ENR1_UART5EN},
    };

    if (cfg->io_init && !cfg->io_init())
        return false;

    uint32_t clock_hz = usart_get_clock_hz(cfg->inst);
    uint32_t brr = (cfg->oversampling == USART_OVERSAMPLING_8) ? (2U * clock_hz) / cfg->baudrate : clock_hz / cfg->baudrate;

    if (brr < 16U || brr > 0xFFFFU)
        return false;

    /* With oversampling by 8, BRR bit 3 is zero and the fractional bits are shifted right. */
    if (cfg->oversampling == USART_OVERSAMPLING_8)
        brr = (brr & 0xFFF0U) | ((brr & 0xFU) >> 1U);

    /* Store interrupts callbacks */
    obj->udre_cb = cfg->udre_cb;
    obj->rxc_cb = cfg->rxc_cb;
    obj->txc_cb = cfg->txc_cb;

    obj->inst = cfg->inst;

    USART_TypeDef *base = usart_base[obj->inst];

    /* Enable the selected USART peripheral clock. */
    volatile uint32_t *clock_reg = (volatile uint32_t *)usart_clock_enable[obj->inst][0];
    *clock_reg |= (uint32_t)usart_clock_enable[obj->inst][1];
    (void)*clock_reg; /* Allow the peripheral clock to become active */

    /* Disable USART */
    base->CR1 &= ~USART_CR1_UE;

    /* Set oversampling and baud rate. */
    base->CR1 &= ~USART_CR1_OVER8;
    base->CR1 |= (uint32_t)cfg->oversampling << USART_CR1_OVER8_Pos;
    base->BRR = brr;

    /* Set word len */
    base->CR1 &= ~USART_CR1_M1;
    base->CR1 &= ~USART_CR1_M0;
    base->CR1 |= !!(cfg->data_size & 0b10)<< USART_CR1_M1_Pos;
    base->CR1 |= !!(cfg->data_size & 0b01) << USART_CR1_M0_Pos;

    /* Set parity */
    base->CR1 &= ~USART_CR1_PCE;
    base->CR1 |= cfg->parity << USART_CR1_PCE_Pos;
    if (cfg->parity)
    {
        base->CR1 &= ~USART_CR1_PS;
        base->CR1 |= !!(cfg->parity & 0b10) << USART_CR1_PS_Pos;
    }

    /* Set stop bits */
    base->CR2 &= ~USART_CR2_STOP;
    base->CR2 |= cfg->stop_bits << USART_CR2_STOP_Pos;

    /* Enable transmitter / receiver */
    base->CR1 &= ~USART_CR1_TE;
    base->CR1 |= !!(cfg->rx_tx & USART_TX_ENABLE) << USART_CR1_TE_Pos;
    base->CR1 &= ~USART_CR1_RE;
    base->CR1 |= !!(cfg->rx_tx & USART_RX_ENABLE) << USART_CR1_RE_Pos;
    
    /* Enable USART */
    base->CR1 |= USART_CR1_UE;


    return true;
}

void usart_send(struct usart_obj *obj, uint8_t *data, uint32_t len)
{
    if (!obj || !data || len == 0)
        return;

    USART_TypeDef *base = usart_base[obj->inst];

    uint32_t pos = 0;

    while (len--)
    {
        while (!(base->ISR & USART_ISR_TXE)) {};

        base->TDR = data[pos++];
    }

    while (!(base->ISR & USART_ISR_TC));
}

bool usart_receive(struct usart_obj *obj, uint8_t *data, uint32_t len)
{
    if (!obj || !data || len == 0)
        return false;

    USART_TypeDef *base = usart_base[obj->inst];

    uint32_t pos = 0;

    while (len--)
    {
        while (!(base->ISR & USART_ISR_RXNE)) {};

        data[pos++] = base->RDR;
    }

    return true;
}

void usart_print(struct usart_obj *obj, char *str)
{
    usart_send(obj, (uint8_t*)str, strlen(str));
}

void usart_deinit(struct usart_obj *obj)
{
    if (!obj)
        return;

    memset(obj, 0x00, sizeof(*obj));
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
