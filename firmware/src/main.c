#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/critical_section.h"
#include "hardware/adc.h"
#include "non_blocking_timer.h"
#include "core1.h"
#include "modules.h"
#include "usb_service.h"

critical_section_t critical_input;
critical_section_t critical_output;

int main() {
    input_devices input;
    output_devices output = {
      .segment = {0xFF, 0xFF, 0xFF},
      .error_leds = {0, 0, 0},
      .radio_module_state = 0,
      .radio_module_blink = 0,
      .button_module_state = 0,
      .button_module_leds= {0, 0, 0, 0},
      .simon_module_state = 0,
      .simon_module_blink = 0,
      .dip_module_state = 0,
      .dip_module_top = {0, 0, 0, 0, 0, 0},
      .dip_module_bottom = {0, 0, 0, 0, 0, 0},
      .maze_module_leds = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
      .maze_module_state = 0
    };

    stdio_init_all();
    init_systick();
    usb_init();

    critical_section_init(&critical_input);
    critical_section_init(&critical_output);

    modules_state_t modules_state;
    uint8_t last_error_count = 0;
    uint8_t last_solved_count = 0;

    // Initialize RNG
    uint32_t randomseed;
    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);
    randomseed = adc_read() << 16;
    adc_gpio_init(27);
    adc_select_input(1);
    randomseed += adc_read() << 16;
    adc_select_input(3);
    randomseed += adc_read();
    srand(randomseed);

    // Make up a random serial
    modules_state.serial[0] = (rand() & 0x03)+0x0C;
    modules_state.serial[1] = (rand() & 0x03)+0x0C;
    modules_state.serial[2] = (rand() & 0x0C)+0x03;

    // Start core1
    sleep_ms(10);
    multicore_launch_core1(core1_entry);

    // Wait for input to settle
    sleep_ms(500);

    get_input(&input, true);

    modules_state.error_count = 0;
    modules_state.current_time = input.less_time_jumper ? 180 : 240;
    for (uint8_t i=0; i < 5; i++) {
        modules_state.module_solved[i] = false;
    }

    printf("Setup Done\n");

    non_blocking_timer_handler count_down;
    init_non_blocking_timer(&count_down, 1000);
    start_non_blocking_timer(&count_down);

    uint32_t loop_counter = 0;
    non_blocking_timer_handler loops;

    if (SHOW_LOOPS) {
        init_non_blocking_timer(&loops, 1000);
        start_non_blocking_timer(&loops);
    }

    while (true) {

        get_input(&input, false);

        // Count down timer
        if (non_blocking_timer_expired(&count_down) && modules_state.current_time > 0) {
           modules_state.current_time--;
           start_non_blocking_timer(&count_down);
        }
        output.segment[0] = modules_state.current_time / 100;
        output.segment[1] = modules_state.current_time % 100 / 10;
        output.segment[2] = modules_state.current_time % 10;

        // Display serial
        if (input.serial_key) {
            output.segment[0] = modules_state.serial[0];
            output.segment[1] = modules_state.serial[1];
            output.segment[2] = modules_state.serial[2];
        }

        module1_process(&input, &output, &modules_state);
        module2_process(&input, &output, &modules_state);
        module3_process(&input, &output, &modules_state);
        module4_process(&input, &output, &modules_state);
        module5_process(&input, &output, &modules_state);

        // Set module state LEDs
        output.radio_module_state = modules_state.module_solved[0] ? GREEN : RED;
        output.button_module_state = modules_state.module_solved[1] ? GREEN : RED;
        output.simon_module_state = modules_state.module_solved[2] ? GREEN : RED;
        output.dip_module_state = modules_state.module_solved[3] ? GREEN : RED;
        output.maze_module_state = modules_state.module_solved[4] ? GREEN : RED;

        // Check for fail
        if (modules_state.current_time == 0 || modules_state.error_count > (input.no_error_jumper ? 0 : 3)) {
            while (true) {
                fail_animation(&output);
                send_output(&output);
            }
        }

        // Blink red on new error
        if (modules_state.error_count > last_error_count) {
            output_devices output_buffer;
            memcpy(&output_buffer, &output, sizeof (output_devices));
            set_all_leds(&output, RED);
            send_output(&output);
            sleep_ms(500);
            memcpy(&output, &output_buffer, sizeof (output_devices));
        }
        last_error_count = modules_state.error_count;

        // Set error Leds
        for (uint8_t i=0; i<3; i++) {
            if (input.no_error_jumper) {
                output.error_leds[i] = 0;
            } else {
                output.error_leds[i] = modules_state.error_count > i ? RED : GREEN;
            }
        }

        send_output(&output);

        uint8_t num_solved = 0;
        for (uint8_t i=0; i<5; i++) {
            if (modules_state.module_solved[i]) {
                num_solved++;
            }
        }

        // Check for success
        if (num_solved == 5) {
            while (true) {
                success_animation(&output);
                send_output(&output);
            }
        }

        // Blink green on new solve
        if (num_solved > last_solved_count) {
            output_devices output_buffer;
            memcpy(&output_buffer, &output, sizeof (output_devices));
            set_all_leds(&output, GREEN);
            send_output(&output);
            sleep_ms(500);
            memcpy(&output, &output_buffer, sizeof (output_devices));
        }
        last_solved_count = num_solved;


        if (SHOW_LOOPS) {
            loop_counter++;
            if (non_blocking_timer_expired(&loops)) {
                printf("Core0 made %d loops per second\n", loop_counter);
                start_non_blocking_timer(&loops);
                loop_counter = 0;
            }
        }
    }
}
