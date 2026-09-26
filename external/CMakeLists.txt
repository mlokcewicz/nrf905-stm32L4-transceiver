# external targets

# cmsis_5 target
add_library(cmsis_5 INTERFACE)

target_include_directories(cmsis_5 INTERFACE CMSIS_5/CMSIS/Core/Include)

# cmsis_device_l4_target
add_library(cmsis_device_l4 STATIC)

target_compile_definitions(cmsis_device_l4 PUBLIC -DSTM32L476xx)

target_include_directories(cmsis_device_l4 PUBLIC cmsis_device_l4/Include)

aux_source_directory(cmsis_device_l4/Source/Templates CMSIS_DEVICE_L4_SRC)
target_sources(cmsis_device_l4 PRIVATE ${CMSIS_DEVICE_L4_SRC})

target_link_libraries(cmsis_device_l4 cmsis_5)
