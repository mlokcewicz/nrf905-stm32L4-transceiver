# nRF905 STM32L4 Transceiver

433 MHz radio transceiver based on nRF905 and STM32L476.

## Capabilities

* One code base for transmitter and receiver
* Pure hardware-driven architecture for receiver (no main loop nor interrupts)
* Custom, fixed size framre structure
* Communication with PC over UART
* Bare metal architecture, no OS, based on custom low level peripheral drivers

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

## Build

### Generate ninja files
`cmake --preset <hw_version>_<config>`

### Build project
`cmake --build --preset <hw_version>_<config>`

## Radio configuration

Radio settings are CMake cache variables defined in the root `CMakeLists.txt`.
Override them when configuring the project with `-DNAME=value`:

| Variable | Default | Meaning |
| --- | --- | --- |
| `CONFIG_RADIO_RX_ADDRESS` | `0xDDCCBBAAUL` | Receive address |
| `CONFIG_RADIO_TX_ADDRESS` | `CONFIG_RADIO_RX_ADDRESS` | Transmit address; follows the receive address unless overridden |
| `CONFIG_RADIO_CHANNEL` | `108` | Channel number (0?511) |
| `CONFIG_RADIO_BAND` | `NRF905_PLL_FREQ_433_MHZ` | Band: `NRF905_PLL_FREQ_433_MHZ` or `NRF905_PLL_FREQ_868_915_MHZ` |
| `CONFIG_RADIO_TX_POWER` | `NRF905_PA_PWR_10_DBM` | Power: `NRF905_PA_PWR_MINUS_10_DBM`, `NRF905_PA_PWR_MINUS_2_DBM`, `NRF905_PA_PWR_6_DBM` or `NRF905_PA_PWR_10_DBM` |

```sh
cmake --preset v0.0.1_debug -DHAL_RADIO_CHANNEL=110 -DHAL_RADIO_RX_ADDRESS=0xAABBCCDDUL -DHAL_RADIO_TX_POWER=NRF905_PA_PWR_6_DBM
cmake --build --preset v0.0.1_debug
```

Values persist in the build directory's CMake cache. Reconfigure and rebuild after changing them.

`CONFIG_TRANSMITTER_EN` and `CONFIG_RECEIVER_EN` are independent ON/OFF options, both ON by default. They include or exclude the respective radio initialization, SPI/DMA callbacks, buffers and radio functions. Public HAL functions remain available and do nothing when their radio is disabled. Loopback tests run only with both options enabled. With the transmitter disabled, the main loop does not wait for UART messages.

