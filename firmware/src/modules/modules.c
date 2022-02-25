#include "modules.h"

void module1_process(input_devices *input, output_devices *output, modules_state_t *module_state) {

}

void module2_process(input_devices *input, output_devices *output, modules_state_t *module_state) {

}

void module3_process(input_devices *input, output_devices *output, modules_state_t *module_state) {

}

void module4_process(input_devices *input, output_devices *output, modules_state_t *module_state) {

}

void module5_process(input_devices *input, output_devices *output, modules_state_t *module_state) {

}

void success_animation(output_devices *output) {
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
