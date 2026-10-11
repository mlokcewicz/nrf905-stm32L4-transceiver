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

    hal_radio_tx_config();
    hal_radio_rx_config();
    hal_uart_rx_start();
    
    hal_radio_loopback_test_hw_driven();

    printf("Transceiver started, waiting for messages...\n");
    
    while (1)
    {
        hal_led_toggle();
        hal_wdg_feed();
        
        hal_delay(500);
    }

    return 0;
}

//------------------------------------------------------------------------------
