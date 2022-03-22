#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "modules.h"
#include "non_blocking_timer.h"

#define MODULE_NUM 1

static bool module_initialized = false;

const uint8_t order[5][4] = {
    {0, 1, 2, 3},
    {3, 2, 0, 1},
    {0, 3, 2, 1},
    {2, 3, 1, 0},
    {1, 2, 0, 3},
};

typedef struct
{
    uint32_t colors[4];
    uint8_t solution;
}  light_config;

#define NUM_RIDDLES 9
const light_config lights[NUM_RIDDLES] = {
    {
        .colors = {GREEN, GREEN, GREEN, RED},
        .solution = 9
    },
    {
        .colors = {GREEN, GREEN, RED, RED},
        .solution = 7
    },
    {
        .colors = {GREEN, GREEN, RED, BLUE},
        .solution = 0
    },
    {
        .colors = {BLUE, BLUE, RED, GREEN},
        .solution = 0
    },
    {
        .colors = {GREEN, GREEN, GREEN, BLUE },
        .solution = 3
    },
    {
        .colors = {RED, RED, RED, BLUE},
        .solution = 7
    },
    {
        .colors = {RED, RED, RED, GREEN},
        .solution = 7
    },
    {
        .colors = {GREEN, GREEN, RED, BLUE},
        .solution = 0xFF
    },
    {
        .colors = {RED, RED, GREEN, BLUE},
        .solution = 0xFF
    },
};

static light_config current_config;
static uint8_t current_order[4];

static bool check_display_num(uint8_t num, output_devices *output) {
    if (output->segment[0] == num || output->segment[1] == num || output->segment[2] == num ) {
        return true;
    }
    return false;
}

static void init_module(output_devices *output) {
    memcpy(&current_config, &lights[rand() % NUM_RIDDLES], sizeof(current_config));
    memcpy(current_order, order[rand() % 5], sizeof(current_order));

    for (uint8_t i=0; i<4; i++) {
        output->button_module_leds[i] = current_config.colors[current_order[i]];
    }
}

void module2_process(input_devices *input, output_devices *output, modules_state_t *module_state) {
    static bool is_pressed = false;
    static uint32_t pressed_at;

    if (!module_initialized) {
        init_module(output);
        module_initialized = true;
    }

    if (module_state->module_solved[MODULE_NUM]) {
        for (uint8_t i=0; i<4; i++) {
            output->button_module_leds[i] = 0;
        }
    }

    if (!module_state->module_solved[MODULE_NUM] && !is_pressed && input->button_key)  {
        is_pressed = true;
        // User pressed the button
        pressed_at = get_systick();
    } else if(is_pressed && !input->button_key) {
        is_pressed = false;
        // User let go of the button

        switch (current_config.solution) {
            case 0:
                if (get_systick()- pressed_at < 500) {
                    module_state->module_solved[MODULE_NUM] = true;
                } else {
                    module_state->error_count += 1;
                }
                break;
            case 3:
            case 7:
            case 9:
                if (check_display_num(current_config.solution, output)) {
                    module_state->module_solved[MODULE_NUM] = true;
                } else {
                    module_state->error_count += 1;
                }
                break;
            case 0xFF:
                if (output->segment[2] < 0x0A) {
                    if (get_systick()- pressed_at < 500) {
                        module_state->module_solved[MODULE_NUM] = true;
                    } else {
                        module_state->error_count += 1;
                    }
                } else {
                    if (check_display_num(7, output)) {
                        module_state->module_solved[MODULE_NUM] = true;
                    } else {
                        module_state->error_count += 1;
                    }
                }
                break;
        }
    }
}
