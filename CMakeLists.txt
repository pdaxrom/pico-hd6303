cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico-hd6303 C CXX ASM)

# Copy program to ram during boot
set(PICO_COPY_TO_RAM 1)

pico_sdk_init()

add_executable(${PROJECT_NAME})

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bootrom.h
       COMMAND make ${CMAKE_CURRENT_BINARY_DIR}/bootrom.h H_OUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/ clean
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/support/BOOTROM
)

target_sources(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        ${CMAKE_CURRENT_BINARY_DIR}/bootrom.h
        )

# Build pio
pico_generate_pio_header(${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/hd6303_pihw.pio
    ${CMAKE_CURRENT_LIST_DIR}/hd6303_pi.pio
)

# Make sure TinyUSB can find tusb_config.h
target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC
    hardware_pio
    hardware_flash
    pico_multicore
    pico_stdlib
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})
