//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stdio.h>

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

int main()
{
    NVIC_SetPriorityGrouping(0x00000003U);
    
    /* SYSCLK = 4 MHz, source: MSI, AHB presc = /1 → HCLK = 4 MHz APB1/APB2 presc = /1 → PCLK1 = PCLK2 = 4 MHz */
    SystemInit();
    SystemCoreClockUpdate();

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    while(!(RCC->AHB2ENR & RCC_AHB2ENR_GPIOAEN));

    GPIOA->MODER &= ~GPIO_MODER_MODE5;  // clear mode register for GPIOA pin 5
    GPIOA->MODER |= GPIO_MODER_MODE5_0; // set mode register general puprpose output

    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_5);    // set output type  to push-pull
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD5_Msk); // disable pull-up and pull-down

    GPIOA->BSRR = (1 << 5); // set bit

    while (1)
    {
        GPIOA->BSRR = 1 << 5;
        // Delay
        GPIOA->BRR = 1 << 5;
    }

    return 0;
}

//------------------------------------------------------------------------------
