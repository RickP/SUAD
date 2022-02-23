#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/critical_section.h"
#include "non_blocking_timer.h"
#include "core1.h"
#include "ws2812.pio.h"

#define WS2812_PIN 2

static void set_pixels(output_devices *output);
static void drive_segment(output_devices *output);
static void check_input(input_devices* input);

static input_devices input;
static output_devices output;

void core1_entry() {

    // Init ws2812 pio
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, (const pio_program_t*) &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);


    bool output_initialized = false;

    // Initialize segment GPIOs
    for (int i=3; i<14; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_OUT);
        gpio_put(i, 1);
    }

    // Initialize key matrix GPIOs
    gpio_init(22);
    gpio_set_dir(22, GPIO_OUT);
    gpio_put(22, 0);
    gpio_init(26);
    gpio_set_dir(26, GPIO_OUT);
    gpio_put(26, 0);
    gpio_init(27);
    gpio_set_dir(27, GPIO_OUT);
    gpio_put(27, 0);
    for (int i=16; i<22; i++) {
        gpio_init(i);
        gpio_set_dir(i, GPIO_IN);
        gpio_pull_down(i);
    }

    uint32_t loop_counter = 0;
    non_blocking_timer_handler loops;
    init_non_blocking_timer(&loops, 1000);
    start_non_blocking_timer(&loops);

    while (true) {
        // Get current input GPIO states and send them to core0
        check_input(&input);
        send_input(&input);

        // Check if we have a config for the output GPIOs and set them
        if (get_output(&output, false)) {
            set_pixels(&output);
            output_initialized = true;
        }

        if (output_initialized) {
            drive_segment(&output);
        }

        loop_counter++;
        if (non_blocking_timer_expired(&loops)) {
            printf("Core1 made %d loops per second\n", loop_counter);
            start_non_blocking_timer(&loops);
            loop_counter = 0;
        }
    }
}

static void check_input(input_devices* input) {
    // @ToDo: query keyboard matrix, jumpers and potentiometer
    static bool key_state[3][6];
    static bool last_key_state[3][6];
    int row = 0;

    input->poti_pos = 23;
    // Check row 1
    gpio_put(27, 1);
    for (int i=16; i<22; i++) {
        key_state[0][row++] = gpio_get(i);
    }
    gpio_put(27, 0);
    // Check row 2
    gpio_put(26, 1);
    row = 0;
    for (int i=16; i<22; i++) {
        key_state[1][row++] = gpio_get(i);
    }
    gpio_put(26, 0);
    // Check row 3
    gpio_put(22, 1);
    row = 0;
    for (int i=16; i<22; i++) {
        key_state[2][row++] = gpio_get(i);
    }
    gpio_put(22, 0);

    for (int i=0; i<3; i++) {
        for (int j=0; j<6; j++) {
            if (key_state[i][j] != last_key_state[i][j]) {
                if (key_state[i][j]) {
                    printf("Key pressed: Row %d - Col: %d\n", i, j);
                } else {
                    printf("Key released: Row %d - Col: %d\n", i, j);
                }
            }
            last_key_state[i][j] = key_state[i][j];
        }
    }

    input->serial_key = key_state[0][0];
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static void put_pixel_array(uint32_t *arr, int n, bool reverse) {
    for (int i = 0; i < n; i++) {
        if (reverse) {
            put_pixel(arr[n - 1 - i]);
        } else {
            put_pixel(arr[i]);
        }
    }
}

// Put the pixels on the ws2812 bus in the right order
static void set_pixels(output_devices *output) {
    static uint32_t pixels[51];
    static uint32_t last_pixels[51] = {0};

    memcpy(pixels, output->error_leds, 51);
    bool pixels_changed = false;
    for (int i=0; i < 51; i++) {
        if (pixels[i] != last_pixels[i]) {
            pixels_changed = true;
            break;
        }
    }
    memcpy(last_pixels, pixels, 51);

    if (!pixels_changed) {
        return;
    }

    put_pixel_array(output->error_leds, 3, false);
    put_pixel(output->radio_module_state);
    put_pixel(output->radio_module_blink);
    put_pixel(output->button_module_state);
    put_pixel_array(output->button_module_leds, 4, false);
    put_pixel(output->simon_module_state);
    put_pixel(output->simon_module_blink);
    put_pixel(output->dip_module_state);
    put_pixel_array(output->dip_module_top, 6, true);
    put_pixel_array(output->dip_module_bottom, 6, false);
    put_pixel_array(output->maze_module_leds[4], 5, false);
    put_pixel_array(output->maze_module_leds[3], 5, true);
    put_pixel_array(output->maze_module_leds[2], 5, false);
    put_pixel_array(output->maze_module_leds[1], 5, true);
    put_pixel_array(output->maze_module_leds[0], 5, false);
    put_pixel(output->maze_module_state);
}

static void drive_segment(output_devices *output) {
    static uint8_t current_display_segment = 0;

    const uint8_t segments[17][7] = {
        {0, 0, 0, 0, 0, 0, 1}, // 0
        {1, 0, 0, 1, 1, 1, 1}, // 1
        {0, 0, 1, 0, 0, 1, 0}, // 2
        {0, 0, 0, 0, 1, 1, 0}, // 3
        {1, 0, 0, 1, 1, 0, 0}, // 4
        {0, 1, 0, 0, 1, 0, 0}, // 5
        {0, 1, 0, 0, 0, 0, 0}, // 6
        {0, 0, 0, 1, 1, 1, 1}, // 7
        {0, 0, 0, 0, 0, 0, 0}, // 8
        {0, 0, 0, 0, 1, 0, 0}, // 9
        {0, 0, 0, 1, 0, 0, 0}, // A
        {1, 1, 0, 0, 0, 0, 0}, // b
        {1, 1, 1, 0, 0, 1, 0}, // c
        {1, 0, 0, 0, 0, 1, 0}, // d
        {0, 1, 1, 0, 0, 0, 0}, // E
        {0, 1, 1, 1, 0, 0, 0}, // F
        {1, 1, 1, 1, 1, 1, 1}, // NONE
    };

    uint8_t segment_index = output->segment[current_display_segment] & 0x0F;
    if ((output->segment[current_display_segment] & 0xFF) == 0xFF) {
       segment_index = 16;
       gpio_put(10, 1);
    } else if ((output->segment[current_display_segment] & 0x10) == 0x10) {
       gpio_put(10, 0);
    } else {
       gpio_put(10, 1);
    }
    gpio_put(3, segments[segment_index][0]);
    gpio_put(4, segments[segment_index][1]);
    gpio_put(5, segments[segment_index][2]);
    gpio_put(6, segments[segment_index][3]);
    gpio_put(7, segments[segment_index][4]);
    gpio_put(8, segments[segment_index][5]);
    gpio_put(9, segments[segment_index][6]);

    switch (current_display_segment++) {
        case 0:
            gpio_put(11, 1);
            gpio_put(12, 1);
            gpio_put(13, 0);
            break;
        case 1:
            gpio_put(11, 1);
            gpio_put(12, 0);
            gpio_put(13, 1);
            break;
        case 2:
            gpio_put(11, 0);
            gpio_put(12, 1);
            gpio_put(13, 1);
            current_display_segment = 0;
            break;
    }
}

extern critical_section_t critical_output;
static output_devices output_buffer;

void send_output(output_devices *output) {
    critical_section_enter_blocking(&critical_output);
    output_buffer = *output;
    output_buffer.was_updated = true;
    critical_section_exit(&critical_output);
}

bool get_output(output_devices *output, bool block) {
        bool return_val = false;
        critical_section_enter_blocking(&critical_output);
        if (output_buffer.was_updated) {
            *output = output_buffer;
            return_val = true;
            output_buffer.was_updated = false;
        }
        critical_section_exit(&critical_output);
        while (block && !return_val) {
            sleep_us(10);
            return_val = get_output(output, block);
        }
        return return_val;
}

extern critical_section_t critical_input;
static input_devices input_buffer;

void send_input(input_devices *input) {
    critical_section_enter_blocking(&critical_input);
    input_buffer = *input;
    input_buffer.was_updated = true;
    critical_section_exit(&critical_input);
}

bool get_input(input_devices *input, bool block) {
        bool return_val = false;
        critical_section_enter_blocking(&critical_input);
        if (input_buffer.was_updated) {
            *input = input_buffer;
            return_val = true;
            input_buffer.was_updated = false;
        }
        critical_section_exit(&critical_input);
        while (block && !return_val) {
            sleep_us(10);
            return_val = get_input(input, block);
        }
        return return_val;
}
