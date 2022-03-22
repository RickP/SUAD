#include <stdio.h>
#include <string.h>
#include "modules.h"

void success_animation(output_devices *output) {
    static bool toggle = false;

    if (toggle) {
       set_all_leds(output, GREEN);
    } else {
       set_all_leds(output, 0);
    }

    sleep_ms(250);
    toggle = !toggle;
}


void fail_animation(output_devices *output) {
    static bool toggle = false;

    if (toggle) {
        output->segment[0] = 0xFF;
        output->segment[1] = 0xFF;
        output->segment[2] = 0xFF;
        set_all_leds(output, RED);
    } else {
        output->segment[0] = 0x00;
        output->segment[1] = 0x00;
        output->segment[2] = 0x00;
        set_all_leds(output, 0);
    }
    sleep_ms(250);
    toggle = !toggle;
}

static inline void set_array(uint32_t *array, uint32_t color, uint8_t length) {
    for (uint8_t i=0; i<length; i++) {
        array[i] = color;
    }
}

void set_all_leds(output_devices *output, uint32_t color) {
    set_array(output->error_leds, color, 3);
    output->radio_module_state = color;
    output->radio_module_blink = color;
    output->button_module_state = color;
    set_array(output->button_module_leds, color, 4);
    output->simon_module_state = color;
    output->simon_module_blink = color;
    output->dip_module_state = color;
    set_array(output->dip_module_top, color, 6);
    set_array(output->dip_module_bottom, color, 6);
    set_array(output->maze_module_leds[0], color, 5);
    set_array(output->maze_module_leds[1], color, 5);
    set_array(output->maze_module_leds[2], color, 5);
    set_array(output->maze_module_leds[3], color, 5);
    set_array(output->maze_module_leds[4], color, 5);
}

void fault_animation(output_devices *output) {
    static bool toggle = false;

    if (toggle) {
        output->segment[0] = 0xFF;
        output->segment[1] = 0xFF;
        output->segment[2] = 0xFF;
    } else {
        output->segment[0] = 0x00;
        output->segment[1] = 0x00;
        output->segment[2] = 0x00;
    }
    sleep_ms(50);
    toggle = !toggle;
}
