//------------------------------------------------------------------------------

/// @file main.c
/// @note Copyright (C) Michał Łokcewicz. All rights reserved.

//------------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>

#include <hal.h>

//------------------------------------------------------------------------------

int main()
{
    hal_init();

    hal_radio_test();
    hal_radio_test_hw();

    while (1)
    {
        hal_led_toggle();
        hal_wdg_feed();

        hal_delay(500);
    }

    return 0;
}

//------------------------------------------------------------------------------
