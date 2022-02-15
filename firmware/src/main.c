#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "non_blocking_timer.h"
#include "core1.h"

output_devices output = {
  .segment = {9,9,9},
  .error_leds = {0,0,0},
  .radio_module_state = 0,
  .radio_module_blink = 0,
  .button_module_state = 0,
  .button_module_leds= {0,0,0,0},
  .simon_module_state = 0,
  .simon_module_blink = 0,
  .dip_module_state = 0,
  .dip_module_top = {0,0,0,0,0,0},
  .dip_module_bottom = {0,0,0,0,0,0},
  .maze_module_leds = {{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
  .maze_module_state = 0,
};

int main() {
    stdio_init_all();
    init_systick();

    // Start core1
    sleep_ms(10);
    multicore_launch_core1(core1_entry);

    input_devices input;

    printf("Setup Done\n");

    while (true) {
        get_input(&input, true);
        sleep_ms(1000);
        printf("Systick: %i\n", get_systick());
        send_output(&output);
    }
}
