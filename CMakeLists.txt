cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

# toolchain configuration
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/hal/platforms/${HW_VERSION}/toolchain.cmake)

# project configuration
project("nrf905_stm32l4_transceiver" C CXX ASM)

# global headers (FreeRTOSConfig.h)
include_directories(app)

# Application modules
add_subdirectory(app)
add_subdirectory(middlewares)
add_subdirectory(external)
add_subdirectory(hal)
