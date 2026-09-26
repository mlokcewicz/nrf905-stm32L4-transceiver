# nRF905 STM32L4 Transceiver

433 MHz radio transceiver based on nRF905 and STM32L476.

## Capabilities

* One code base for transmitter and receiver
* Pure hardware-driven architecture for receiver (no main loop nor interrupts)
* Custom, fixed size framre structure
* Communication with PC over UART

## Tools

* CMake 3.20.0
* Ninja 1.11.1 
* Arm GNU Toolchain 13.2.rel1 (Build arm-13.7) 13.2.1 20231009
* SEGGER J-Link Software V7.96k
* OpenOCD v0.12.0-i686-w64-mingw32 (optional)
* STSW-LINK009 v2.0.2 (optional)

## External libraries

* CMSIS_5 5.9.0
* cmsis_device_l4 v1.7.2
* FreeRTOS-Kernel V10.4.6
* stm32l4xx_hal_driver v1.13.4 (optional)

## Build

### Generate ninja files
`cmake --preset <hw_version>_<config>`

### Build project
`cmake --build --preset <hw_version>_<config>`
