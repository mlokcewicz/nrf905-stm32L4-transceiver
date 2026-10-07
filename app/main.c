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

    hal_radio_loopback_test_hw_driven();

    printf("Transceiver started, waiting for messages...\n");
    
    while (1)
    {
        hal_led_toggle();
        hal_wdg_feed();
        
        hal_radio_rx_config_reload();
        hal_uart_wait_for_msg();
        hal_radio_tx_send();
        
        hal_delay(1000);
    }

    return 0;
}

//------------------------------------------------------------------------------
