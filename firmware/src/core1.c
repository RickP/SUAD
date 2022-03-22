#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/critical_section.h"
#include "hardware/adc.h"
#include "non_blocking_timer.h"
#include "core1.h"
#include "ws2812.pio.h"
#include "tusb.h"

static void set_pixels(output_devices *output);
static void drive_segment(output_devices *output);
static void check_input(input_devices* input);

static input_devices input;
static output_devices output;

#define WS2812_PIN 2
#define DISPLAY_MASK 0x3FF8
#define KEY_ROW_MASK 0xC400000
#define KEY_COL_MASK 0x3F0000
#define TIME_JUMPER 14
#define TRIALS_JUMPER 15

#define DEBOUNCE_LOOPS 50
#define DEBOUNCE_LOOPS_DIP DEBOUNCE_LOOPS * 10

void core1_entry() {

    tusb_init();

    // Init ws2812 pio
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, (const pio_program_t*) &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    bool output_initialized = false;

    // Initialize segment GPIOs
    gpio_init_mask(DISPLAY_MASK);
    gpio_set_dir_out_masked(DISPLAY_MASK);
    gpio_set_mask(DISPLAY_MASK);

    // Initialize key matrix GPIOs
    gpio_init_mask(KEY_ROW_MASK);
    gpio_set_dir_out_masked(KEY_ROW_MASK);
    gpio_set_mask(KEY_ROW_MASK);

    gpio_init_mask(KEY_COL_MASK);
    gpio_set_dir_in_masked(KEY_COL_MASK);
    for (int i=16; i<22; i++) {
        gpio_pull_up(i);
    }

    // Initialize ADC
    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);


    // Initialize jumpers
    gpio_init(TIME_JUMPER);
    gpio_set_dir(TIME_JUMPER, false);
    gpio_pull_up(TIME_JUMPER);
    gpio_init(TRIALS_JUMPER);
    gpio_set_dir(TRIALS_JUMPER, false);
    gpio_pull_up(TRIALS_JUMPER);

    uint32_t loop_counter = 0;
    non_blocking_timer_handler loops;
    if (SHOW_LOOPS) {
        init_non_blocking_timer(&loops, 1000);
        start_non_blocking_timer(&loops);
    }

    while (true) {
        tud_task();

        // Get current input GPIO states and send them to core0
        check_input(&input);
        send_input(&input);

        // Check if we have a config for the output GPIOs and set them
        if (get_output(&output, false)) {
            output_initialized = true;
            set_pixels(&output);
        }

        if (output_initialized) {
            drive_segment(&output);
        }

        if (SHOW_LOOPS) {
            loop_counter++;
            if (non_blocking_timer_expired(&loops)) {
                printf("Core1 made %d loops per second\n", loop_counter);
                start_non_blocking_timer(&loops);
                loop_counter = 0;
            }
        }
    }
}

static void check_input(input_devices* input) {

    // Query jumpers
    input->less_time_jumper = gpio_get(TIME_JUMPER);
    input->no_error_jumper = gpio_get(TRIALS_JUMPER);

    // Query key matrix

    static bool key_state[3][6];
    static uint16_t key_counter[3][6];
    static bool last_key_state[3][6];
    int row = 0;

    input->poti_pos = adc_read();

    bool button_pressed = false;

    // Check row 1
    gpio_put(27, 0);
    sleep_us(10);
    for (uint8_t i=16; i<22; i++) {
        bool val = !gpio_get(i);
        if (val) {
            key_counter[0][row] += key_counter[0][row] > DEBOUNCE_LOOPS ? 0 : 1;
            button_pressed = true;
        } else {
            key_counter[0][row] = 0;
        }
        row++;
    }
    gpio_put(27, 1);


    // Check row 2
    gpio_put(26, 0);
    sleep_us(10);
    row = 0;
    for (uint8_t i=16; i<22; i++) {
        bool val = !gpio_get(i);
        if (val) {
            key_counter[1][row] += key_counter[1][row] > DEBOUNCE_LOOPS ? 0 : 1;
            button_pressed = true;
        } else {
            key_counter[1][row] = 0;
        }
        row++;
    }
    gpio_put(26, 1);

    // Check row 3 if no button is currently pressed
    if (!button_pressed) {
        gpio_put(22, 0);
        sleep_us(10);
        row = 0;
        for (uint8_t i=16; i<22; i++) {
            bool val = !gpio_get(i);
            if (val) {
                key_counter[2][row] += key_counter[2][row] > DEBOUNCE_LOOPS_DIP ? 0 : 1;
            } else {
                key_counter[2][row] -= key_counter[2][row] > 0 ? 1 : 0;
            }
            row++;
        }
        gpio_put(22, 1);
    }


    for (uint8_t i = 0; i < 2; i++) {
         for (uint8_t j = 0; j < 6; j++) {
             if (key_counter[i][j] > DEBOUNCE_LOOPS) {
                 key_state[i][j] = 1;
             } else if (key_counter[i][j] == 0) {
                 key_state[i][j] = 0;
             }
         }
    }
    for (uint8_t j = 0; j < 6; j++) {
         if (key_counter[2][j] > DEBOUNCE_LOOPS_DIP) {
             key_state[2][j] = 1;
         } else if (key_counter[2][j] == 0) {
             key_state[2][j] = 0;
         }
     }

    if (SHOW_KEYPRESS) {
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
    }

    input->serial_key = key_state[0][0];
    input->radio_receive_key = key_state[1][0];
    input->radio_transmit_key = key_state[1][1];
    input->button_key = key_state[0][5];
    input->simon_up_key = key_state[0][2];
    input->simon_down_key = key_state[0][4];
    input->simon_left_key = key_state[0][1];
    input->simon_right_key = key_state[0][3];
    input->matrix_up_key = key_state[1][2];
    input->matrix_down_key = key_state[1][3];
    input->matrix_left_key = key_state[1][4];
    input->matrix_right_key = key_state[1][5];
    input->dip_switches[0] = key_state[2][0];
    input->dip_switches[1] = key_state[2][1];
    input->dip_switches[2] = key_state[2][2];
    input->dip_switches[3] = key_state[2][3];
    input->dip_switches[4] = key_state[2][4];
    input->dip_switches[5] = key_state[2][5];
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
    put_pixel_array(output->maze_module_leds[3], 5, false);
    put_pixel_array(output->maze_module_leds[2], 5, false);
    put_pixel_array(output->maze_module_leds[1], 5, false);
    put_pixel_array(output->maze_module_leds[0], 5, false);
    put_pixel(output->maze_module_state);

    sleep_us(400);
}

static void drive_segment(output_devices *output) {
    static uint8_t current_display_segment = 0;

    const uint32_t display_mask[3] = {0x1800, 0x2800, 0x3000};

    const uint32_t segment_mask[17] = {
        0x200, 0x3C8, 0x120, 0x180, 0xC8, 0x90, 0x10, 0x3C0, 0x0,
        0x80, 0x40, 0x18, 0x138, 0x108, 0x30, 0x70, 0x3F8
    };

    uint8_t segment_index = output->segment[current_display_segment] & 0x0F;
    uint32_t dot_mask = 0x400;
    if ((output->segment[current_display_segment] & 0xF0) == 0xF0) {
       segment_index = 16;
    } else if ((output->segment[current_display_segment] & 0x10) == 0x10) {
       dot_mask = 0x0;
       if ((output->segment[current_display_segment] & 0x20) == 0x20) {
          segment_index = 16;
       }
    }

    uint32_t switch_mask = display_mask[current_display_segment] | segment_mask[segment_index] | dot_mask;
    gpio_put_masked(DISPLAY_MASK, switch_mask);

    if (current_display_segment++ == 2) {
        current_display_segment = 0;
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
            sleep_ms(1);
            return_val = get_input(input, block);
        }
        return return_val;
}
