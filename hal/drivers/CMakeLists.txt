# drivers target
add_library(drivers STATIC)

target_include_directories(drivers PUBLIC .)

target_sources(drivers PRIVATE 
    systick.c
    rcc.c
    pwr.c
    flash.c
    wdg.c
    usart.c
    spi.c
)

target_link_libraries(drivers
    cmsis_device_l4 
    cmsis_5 
)
