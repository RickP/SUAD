#include <stdio.h>
#include "modules.h"

#define MODULE_NUM 3

static bool module_initialized = false;

static void init_module(output_devices *output) {
    output->dip_module_top[0] = BLUE;
    output->dip_module_bottom[0] = BLUE;
    output->dip_module_top[1] = GREEN;
    output->dip_module_bottom[1] = 0;
    output->dip_module_top[2] = 0;
    output->dip_module_bottom[2] = RED;
    output->dip_module_top[3] = GREEN;
    output->dip_module_bottom[3] = BLUE;
    output->dip_module_top[4] = GREEN;
    output->dip_module_bottom[4] = RED;
    output->dip_module_top[5] = RED;
    output->dip_module_bottom[5] = 0;
}

void module4_process(input_devices *input, output_devices *output, modules_state_t *module_state) {
    if (!module_initialized) {
        init_module(output);
        module_initialized = true;
    }
}
