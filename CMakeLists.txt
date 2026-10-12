cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

# toolchain configuration
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/hal/platforms/${HW_VERSION}/toolchain.cmake)

# project configuration
project("nrf905_stm32l4_transceiver" C CXX ASM)

# global headers
include_directories(app)

# Configurable radio settings (override with -DNAME=value)
option(CONFIG_TRANSMITTER_EN "Enable the nRF TX radio" ON)
option(CONFIG_RECEIVER_EN "Enable the nRF RX radio" ON)
set(CONFIG_RADIO_RX_ADDRESS "0xDDCCBBAAUL" CACHE STRING "Radio receive address")
set(CONFIG_RADIO_TX_ADDRESS "CONFIG_RADIO_RX_ADDRESS" CACHE STRING "Radio transmit address; defaults to the receive address")
set(CONFIG_RADIO_CHANNEL "108" CACHE STRING "nRF905 channel number")
set(CONFIG_RADIO_BAND "NRF905_PLL_FREQ_433_MHZ" CACHE STRING "nRF905 frequency band enum")
set(CONFIG_RADIO_TX_POWER "NRF905_PA_PWR_10_DBM" CACHE STRING "nRF905 transmit power enum")

# Application modules
add_subdirectory(app)
add_subdirectory(middlewares)
add_subdirectory(external)
add_subdirectory(hal)

# Pass radio settings to the hardware platform
target_compile_definitions(platform PRIVATE
    CONFIG_RADIO_RX_ADDRESS=${CONFIG_RADIO_RX_ADDRESS}
    CONFIG_RADIO_TX_ADDRESS=${CONFIG_RADIO_TX_ADDRESS}
    CONFIG_RADIO_CHANNEL=${CONFIG_RADIO_CHANNEL}
    CONFIG_RADIO_BAND=${CONFIG_RADIO_BAND}
    CONFIG_RADIO_TX_POWER=${CONFIG_RADIO_TX_POWER}
)

target_compile_definitions(platform PUBLIC
    CONFIG_TRANSMITTER_EN=$<BOOL:${CONFIG_TRANSMITTER_EN}>
    CONFIG_RECEIVER_EN=$<BOOL:${CONFIG_RECEIVER_EN}>
)
