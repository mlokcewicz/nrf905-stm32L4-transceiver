//------------------------------------------------------------------------------

/// @file systick.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include "systick.h"

#include <stm32l4xx.h>

//------------------------------------------------------------------------------

#define SYSTICK_PREEMPT_PRIO 15
#define SYSTICK_SUB_PRIO 0

//------------------------------------------------------------------------------

struct systick_ctx
{
    volatile uint32_t current_ms;
    uint32_t ms_per_tick;
};

//------------------------------------------------------------------------------

extern uint32_t SystemCoreClock;

static struct systick_ctx ctx;

//------------------------------------------------------------------------------

uint32_t systick_init(uint32_t ms_per_tick)
{
    uint32_t ret = 0;
    
    ret = SysTick_Config(SystemCoreClock / 1000U * ms_per_tick);
    if (ret != 0)
        return ret;

    ctx.ms_per_tick = ms_per_tick;

    uint32_t prioritygroup = NVIC_GetPriorityGrouping();

    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(prioritygroup, SYSTICK_PREEMPT_PRIO, SYSTICK_SUB_PRIO));  
    
    return 0;
}

uint32_t systick_get_ms(void)
{
    return ctx.current_ms;
}

bool systick_timeout_elapsed(uint32_t base_ms, uint32_t timeout_ms)
{
    return base_ms + timeout_ms < systick_get_ms(); 
}

void systick_delay(uint32_t ms)
{
    uint32_t base_ms = systick_get_ms();

    while (!systick_timeout_elapsed(base_ms, ms))
    {
        /* Blocking wait */
    }
}

//------------------------------------------------------------------------------

void SysTick_Handler(void)
{
    ctx.current_ms += ctx.ms_per_tick;
}

//------------------------------------------------------------------------------
