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

add_executable(${APP})

target_sources(${APP} PUBLIC
    src/main.c
    src/core1.c
    src/modules/module_1.c
    src/modules/module_2.c
    src/modules/module_3.c
    src/modules/module_4.c
    src/modules/module_5.c
    src/modules/animations.c
    src/lib/non_blocking_timer.c
    src/usb/usb_descriptors.c
    src/usb/msc_disk.c
)

pico_generate_pio_header(${APP} ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/build)

# Make sure headers can be found
target_include_directories(${APP} PUBLIC
    src
    src/lib
    src/modules
    src/usb
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(${APP} PUBLIC pico_stdlib pico_multicore pico_sync hardware_pio hardware_adc tinyusb_device tinyusb_board)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${APP})

add_custom_target(pack_files
    COMMAND python3 ../pack_msc_files.py
)
add_dependencies(${APP} pack_files)

add_custom_target(small_msc
    COMMAND cp ../src/usb/content_minimal.h ../src/usb/content.h
)

add_custom_target(flash
    COMMAND openocd -f interface/picoprobe.cfg -f target/rp2040.cfg -c "program ${APP}.elf verify reset exit"
    DEPENDS ${APP}
)
