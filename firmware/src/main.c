#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "non_blocking_timer.h"
#include "core1.h"

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

input_devices input_buffer;

int main() {
    stdio_init_all();
    init_systick();

    // Start core1
    sleep_ms(10);
    multicore_launch_core1(core1_entry);

    get_input(&input_buffer, true);

    // @ToDo: initialize modules

    printf("Setup Done\n");

    uint32_t current_timer = 300;
    non_blocking_timer_handler count_down;
    init_non_blocking_timer(&count_down, 1000);
    start_non_blocking_timer(&count_down);

    while (true) {
        input_devices input;
        if (get_input(&input, false)) {
            memcpy(&input_buffer, &input, sizeof(input_devices));
        }

        // Count down timer
        if (non_blocking_timer_expired(&count_down) && current_timer > 0) {
           current_timer--;
           start_non_blocking_timer(&count_down);
        }
        output.segment[0] = current_timer / 100;
        output.segment[1] = current_timer % 100 / 10;
        output.segment[2] = current_timer % 10;

        if (input.serial_key) {
            output.segment[0] = 0x0F;
            output.segment[1] = 0x03;
            output.segment[2] = 0x0A;
        }

        // @ToDo: process game modules

        send_output(&output);
    }
}
