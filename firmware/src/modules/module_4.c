#include <stdio.h>
#include <stdlib.h>
#include "modules.h"

#define MODULE_NUM 3

static bool module_initialized = false;

typedef struct
{
    uint32_t above_color;
    uint32_t below_color;
    bool switch_state;
}  switch_config;

static const switch_config switch_settings[16] = {
    {
        .above_color = RED,
        .below_color = RED,
        .switch_state = false,
    },
    {
        .above_color = RED,
        .below_color = GREEN,
        .switch_state = true,
    },
    {
        .above_color = RED,
        .below_color = BLUE,
        .switch_state = true,
    },
    {
        .above_color = RED,
        .below_color = 0,
        .switch_state = false,
    },
    {
        .above_color = GREEN,
        .below_color = RED,
        .switch_state = true,
    },
    {
        .above_color = GREEN,
        .below_color = GREEN,
        .switch_state = false,
    },
    {
        .above_color = GREEN,
        .below_color = BLUE,
        .switch_state = false,
    },
    {
        .above_color = GREEN,
        .below_color = 0,
        .switch_state = true,
    },
    {
        .above_color = BLUE,
        .below_color = RED,
        .switch_state = false,
    },
    {
        .above_color = BLUE,
        .below_color = GREEN,
        .switch_state = false,
    },
    {
        .above_color = BLUE,
        .below_color = BLUE,
        .switch_state = true,
    },
    {
        .above_color = BLUE,
        .below_color = 0,
        .switch_state = true,
    },
    {
        .above_color = 0,
        .below_color = RED,
        .switch_state = true,
    },
    {
        .above_color = 0,
        .below_color = GREEN,
        .switch_state = true,
    },
    {
        .above_color = 0,
        .below_color = BLUE,
        .switch_state = false,
    },
    {
        .above_color = 0,
        .below_color = 0,
        .switch_state = false,
    }
};

static switch_config target_riddles[6];

static uint8_t check_solved(input_devices *input) {
    uint8_t solved = 0;
    for (uint8_t i=0; i<6; i++) {
        if (target_riddles[i].switch_state == input->dip_switches[i]) {
            solved++;
        }
    }
    return solved;
}

static void init_module(output_devices *output, input_devices *input) {
    uint8_t solved = 0;
    do {
        for (uint8_t i=0; i<6; i++) {
            target_riddles[i] = switch_settings[rand() % 16];
            output->dip_module_top[i] = target_riddles[i].above_color;
            output->dip_module_bottom[i] = target_riddles[i].below_color;
        }
        solved = check_solved(input);
    } while (solved > 1);
}

void module4_process(input_devices *input, output_devices *output, modules_state_t *module_state) {
    static uint8_t num_solved = 0;
    static uint8_t last_solved = 0;

    if (!module_initialized) {
        init_module(output, input);
        module_initialized = true;
    }

    // @ToDo: check wrong toggles
    num_solved = check_solved(input);
    if (num_solved < last_solved) {
        module_state->error_count += 1;
    } else if (num_solved == 6) {
        module_state->module_solved[MODULE_NUM] = true;
        for (uint8_t i=0; i<6; i++) {
            output->dip_module_top[i] = 0;
            output->dip_module_bottom[i] = 0;
        }
    }

    last_solved = num_solved;
}
