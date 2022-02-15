#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "core1.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 51
#define WS2812_PIN 2

input_devices input = {
    .serial_key = false,
    .radio_receive_key = false,
    .radio_transmit_key = false,
    .button_key = false,
    .simon_up_key = false,
    .simon_down_key = false,
    .simon_left_key = false,
    .simon_right_key = false,
    .matrix_up_key = false,
    .matrix_down_key = false,
    .matrix_left_key = false,
    .matrix_right_key = false,
    .dip_switches = {false, false, false, false, false, false},
    .poti_pos = 0,
    .less_time_jumper = false,
    .no_error_jumper = false
};

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static void check_input(input_devices* input) {

}

void core1_entry() {

    // Init ws2812 pio
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, (const pio_program_t*) &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    uint8_t current_segment[3] = {0, 0, 0};
    output_devices output;

    while (true) {
        check_input(&input);
        send_input(&input);

        if (get_output(&output, false)) {
            memcpy(current_segment, output.segment, 3);
            printf("Core1: %i\n", current_segment[1]);
        }
    }
}

void send_output(output_devices *output) {
    multicore_fifo_push_timeout_us((uintptr_t) output, 10);
}

bool get_output(output_devices *output, bool block) {
    if (block || multicore_fifo_rvalid()) {
        uintptr_t ptr = (uintptr_t) multicore_fifo_pop_blocking();
        *output = *(output_devices*) ptr;
        return true;
    }
    return false;
}

void send_input(input_devices *input) {
    multicore_fifo_push_timeout_us((uintptr_t) input, 10);
}

bool get_input(input_devices *input, bool block) {
    if (block || multicore_fifo_rvalid()) {
        uintptr_t ptr = (uintptr_t) multicore_fifo_pop_blocking();
        *input = *(input_devices*) ptr;
        return true;
    }
    return false;
}
