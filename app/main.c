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

    hal_radio_rx_config();

    hal_radio_loopback_test();
    
    while (1)
    {
        hal_led_toggle();
        hal_wdg_feed();
        
        hal_radio_rx_config2();
        hal_radio_loopback_test();
        hal_delay(1000);
    }

    return 0;
}

//------------------------------------------------------------------------------
