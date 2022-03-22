#include <stdio.h>
#include "modules.h"

void success_animation(output_devices *output) {
    static bool toggle = false;

    for (uint8_t i=0; i<5; i++) {
        for (uint8_t j=0; j<5; j++) {
            output->maze_module_leds[i][j] = toggle ? RED : GREEN;
        }
    }

    if (toggle) {
        output->segment[0] = 0xFF;
        output->segment[1] = 0xFF;
        output->segment[2] = 0xFF;
    } else {
        output->segment[0] = 0x00;
        output->segment[1] = 0x00;
        output->segment[2] = 0x00;
    }
    sleep_ms(100);
    toggle = !toggle;
}


void fail_animation(output_devices *output) {
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
