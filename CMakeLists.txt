cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico-hd6303 C CXX ASM)

# Copy program to ram during boot
# set(PICO_COPY_TO_RAM 1)

pico_sdk_init()

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        )

# Build pio
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/hd6303_pi.pio)

# Make sure TinyUSB can find tusb_config.h
#target_include_directories(${PROJECT_NAME} PUBLIC
#        ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC
    hardware_pio
    pico_stdlib
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})