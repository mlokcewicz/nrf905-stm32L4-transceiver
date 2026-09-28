//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stdio.h>

#include <stm32l4xx.h>

#include <systick.h>
#include <rcc.h>
#include <gpio.h>
#include <flash.h>
#include <pwr.h>
#include <usart.h>

//------------------------------------------------------------------------------

static struct usart_obj usart = {0};

//------------------------------------------------------------------------------

int __io_putchar(int ch)
{
  usart_send(&usart, (uint8_t*)&ch, 1);
  return 1;
}

static bool io_init(void)
{
    GPIO_CONFIG(A, 2, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 7);
    GPIO_CONFIG(A, 3, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 7);

    return true;
}

//------------------------------------------------------------------------------

int main()
{
    NVIC_SetPriorityGrouping(0x00000003U);
    
    /* SYSCLK = 4 MHz, source: MSI, AHB presc = /1 → HCLK = 4 MHz APB1/APB2 presc = /1 → PCLK1 = PCLK2 = 4 MHz */
    SystemInit();

    /* Set voltage scalling range */
    pwr_init(PWR_VOLTAGE_SCALING_RANGE1);

    /* Vcore range 1, HCLK <= 48 MHz */
    flash_init(FLASH_ACR_LATENCY_2WS);

    /* SYSCLK = HCLK = PCLK1 = PCLK2 = 40 MHz, HSI on, LS on */
    struct rcc_config cfg = 
    {
        .lsi_on = true,
        .hsi_on = true,
        .hse_on = false,
        .pll.main_pll_on = true,
        .pll.pllm = RCC_PLLM_DIV2,
        .pll.plln = 10,
        .pll.pllr = RCC_PLLR_DIV2,
        .pll.pllren = true,
        .pll.pllsrc = RCC_PLLCFGR_PLLSRC_HSI,
        .ahb_presc = RCC_CFGR_HPRE_DIV1,
        .apb1_presc = RCC_CFGR_PPRE1_DIV1,
        .apb2_presc = RCC_CFGR_PPRE2_DIV1,
        .sysclk_source = RCC_CFGR_SW_PLL,
    };

    rcc_init(&cfg);

    SystemCoreClockUpdate();

    systick_init();

    struct usart_cfg usart_cfg =
    {
        .inst = USART_INSTANCE_2,
        .rx_tx = USART_TX_ENABLE | USART_RX_ENABLE,
        .data_size = USART_DATASIZE_8_BIT,
        .parity = USART_PARITY_DISABLED,
        .stop_bits = USART_STOP_BITS_1,
        .oversampling = USART_OVERSAMPLING_16,
        .baudrate = 115200,
        .io_init = io_init,
    };

    if (!usart_init(&usart, &usart_cfg))
        return 1;

    GPIO_CONFIG(A, 5, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);
    
    while (1)
    {
        GPIO_TOGGLE(A, 5);
        printf("Hello!\n");

        systick_delay(500);
    }

    return 0;
}

//------------------------------------------------------------------------------

/*
CE = standby, (hi - on)
TXE - (hi - tx, low - rx)
PWR - (hi - on)
DR - (hi - tx finished or new data received)
SO - MISO
SI - MOSI
SCK - clock
CSN - CS


*/
