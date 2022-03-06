#include <stdio.h>
#include "modules.h"

static bool module_initialized = false;

static void init_module(output_devices *output) {

}

void module4_process(input_devices *input, output_devices *output, modules_state_t *module_state) {
    if (!module_initialized) {
        init_module(output);
        module_initialized = true;
    }
}
