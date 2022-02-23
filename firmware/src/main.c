#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/critical_section.h"
#include "non_blocking_timer.h"
#include "core1.h"

critical_section_t critical_input;
critical_section_t critical_output;

static output_devices output = {
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

static input_devices input;

int main() {
    stdio_init_all();
    init_systick();

    critical_section_init(&critical_input);
    critical_section_init(&critical_output);

    // Start core1
    sleep_ms(10);
    multicore_launch_core1(core1_entry);

    get_input(&input, true);

    // @ToDo: initialize modules

    printf("Setup Done\n");

    uint32_t current_timer = 300;
    non_blocking_timer_handler count_down;
    init_non_blocking_timer(&count_down, 1000);
    start_non_blocking_timer(&count_down);

    while (true) {

        get_input(&input, false);

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
