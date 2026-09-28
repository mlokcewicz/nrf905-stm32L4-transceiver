//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stdio.h>

#include <stm32l4xx.h>

#include <systick.h>
#include <gpio.h>

//------------------------------------------------------------------------------

int main()
{
    NVIC_SetPriorityGrouping(0x00000003U);
    
    /* SYSCLK = 4 MHz, source: MSI, AHB presc = /1 → HCLK = 4 MHz APB1/APB2 presc = /1 → PCLK1 = PCLK2 = 4 MHz */
    SystemInit();
    SystemCoreClockUpdate();

    systick_init();

    GPIO_CONFIG(A, 5, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE, 0);

    
    while (1)
    {
        GPIO_TOGGLE(A, 5);
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