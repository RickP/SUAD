cmake_minimum_required(VERSION 3.13)

set(APP SUAD)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(${APP})

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(${APP}
    src/main.c
    src/core1.c
    src/modules.c
    src/non_blocking_timer.c
)

pico_generate_pio_header(${APP} ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/build)

pico_enable_stdio_uart(${APP} 1)
pico_enable_stdio_usb(${APP} 0)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(${APP} pico_stdlib pico_multicore pico_sync hardware_pio) # tinyusb_device tinyusb_board)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${APP})

add_custom_target(flash
    COMMAND openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program ${APP}.elf verify reset exit"
    DEPENDS ${APP}
)
