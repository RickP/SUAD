#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "modules.h"

# define SHOW_SERIAL 1

#define set_led(position, color) output->maze_module_leds[position[0]][position[1]] = color
#define same(a, b) (a[0] == b[0] && a[1] == b[1])

static bool module_initialized = false;

typedef struct
{
    uint8_t barriers[6][2][2];
    uint8_t start_positions[3][2];
    uint8_t target_positions[3][2];
    uint8_t green_positions[2][2];
    uint8_t target_position[2];
    uint8_t current_position[2];
}  maze;

#define NUM_MAZES 6
static const maze mazes[NUM_MAZES] = {
    {
        .barriers = {{
                {0,1}, {0,2}
            },{
                {1,1}, {1,2}
            },{
                {2,1}, {2,2}
            },{
                {2,2}, {3,2}
            },{
                {2,3}, {3,3}
            },{
                {3,2}, {3,3}
            }
        },
        .start_positions = {
            {0,0},
            {0,1},
            {1,0},
        },
        .target_positions = {
            {0,3},
            {1,2},
            {1,3},
        },
        .green_positions = {
            {0,4},
            {4,0},
        }
    },{
        .barriers = {{
                {1,1}, {1,2}
            },{
                {2,1}, {2,2}
            },{
                {2,1}, {3,1}
            },{
                {2,2}, {3,2}
            },{
                {2,3}, {3,3}
            },{
                {3,0}, {3,1}
            }
        },
        .start_positions = {
            {2,4},
            {3,4},
            {4,4},
        },
        .target_positions = {
            {1,1},
            {2,0},
            {2,1},
        },
        .green_positions = {
            {2,2},
            {3,2},
        }
    },{
        .barriers = {{
                {0,3}, {1,3}
            },{
                {1,1}, {1,2}
            },{
                {1,1}, {2,1}
            },{
                {2,2}, {2,3}
            },{
                {2,3}, {3,3}
            },{
                {3,1}, {3,2}
            }
        },
        .start_positions = {
            {3,0},
            {4,0},
            {4,1},
        },
        .target_positions = {
            {1,3},
            {1,4},
            {2,3},
        },
        .green_positions = {
            {0,1},
            {4,3},
        }
    },
    {
        .barriers = {{
                {1,1}, {1,2}
            },{
                {2,1}, {2,2}
            },{
                {2,3}, {3,3}
            },{
                {2,4}, {3,4}
            },{
                {3,1}, {3,2}
            },{
                {4,3}, {4,3}
            }
        },
        .start_positions = {
            {2,1},
            {3,0},
            {3,1},
        },
        .target_positions = {
            {1,3},
            {2,3},
            {2,4},
        },
        .green_positions = {
            {0,0},
            {4,3},
        }
    },{
        .barriers = {{
                {1,1}, {2,1}
            },{
                {1,2}, {2,2}
            },{
                {1,3}, {2,3}
            },{
                {1,4}, {2,4}
            },{
                {3,1}, {3,2}
            },{
                {4,1}, {4,2}
            }
        },
        .start_positions = {
            {0,2},
            {0,3},
            {0,4},
        },
        .target_positions = {
            {3,3},
            {4,2},
            {4,3},
        },
        .green_positions = {
            {1,4},
            {4,1},
        },
    },{
        .barriers = {{
                {0,1}, {1,1}
            },{
                {1,0}, {1,1}
            },{
                {2,1}, {2,2}
            },{
                {2,1}, {3,1}
            },{
                {2,2}, {3,2}
            },{
                {2,3}, {3,3}
            }
        },
        .start_positions = {
            {1,1},
            {2,1},
            {2,2},
        },
        .target_positions = {
            {2,0},
            {3,0},
            {4,0},
        },
        .green_positions = {
            {0,4},
            {4,4},
        }
    }
};

static maze target_riddle;


static void print_playfield() {
#if SHOW_SERIAL
    for (uint8_t i=0; i< 5; i++) {
        for (uint8_t j=0; j< 5; j++) {
            uint8_t pos[2] = {i, j};
            if (same(target_riddle.current_position, pos)) {
                printf(" B");
            } else if (same(target_riddle.target_position, pos)) {
                printf(" R");
            } else if (same(target_riddle.green_positions[0], pos) || same(target_riddle.green_positions[1], pos)) {
                printf(" G");
            } else {
                printf(" *");
            }
        }
        printf("\n");
    }
    printf("\n---\n");
# endif
}

static void init_module() {
    target_riddle = mazes[rand() % NUM_MAZES];
    memcpy(target_riddle.target_position, target_riddle.target_positions[rand() % 3], 2);
    memcpy(target_riddle.current_position, target_riddle.start_positions[rand() % 3], 2);
    print_playfield();
}

void module5_process(input_devices *input, output_devices *output, modules_state_t *module_state) {
    static bool button_pressed = false;

    if (!module_initialized) {
        init_module();
        module_initialized = true;
    }


    if (!module_state->module_solved[4] && (input->matrix_down_key || input->matrix_up_key || input->matrix_left_key || input->matrix_right_key)) {
        if (!button_pressed) {
            button_pressed = true;
            int8_t new_pos[2];
            memcpy(new_pos, target_riddle.current_position, 2);
            if (input->matrix_down_key) {
                new_pos[0]++;
            } else if (input->matrix_up_key) {
                new_pos[0]--;
            } else if (input->matrix_left_key) {
                new_pos[1]--;
            } else {
                new_pos[1]++;
            }
            // Check validity of move
            bool move_valid = true;
            if (new_pos[0] < 0 || new_pos[0] > 4 || new_pos[1] < 0 || new_pos[1] > 4) {
                move_valid = false;
            } else {
                for (uint8_t i=0; i<NUM_MAZES; i++) {
                    if (same(target_riddle.current_position, target_riddle.barriers[i][0]) && same(new_pos, target_riddle.barriers[i][1])) {
                        move_valid = false;
                        break;
                    }
                    if (same(target_riddle.current_position, target_riddle.barriers[i][1]) && same(new_pos, target_riddle.barriers[i][0])) {
                        move_valid = false;
                        break;
                    }
                }
            }

            if (move_valid) {
                memcpy(target_riddle.current_position, new_pos, 2);
            } else {
                module_state->error_count += 1;
            }

            print_playfield();

            if (same(target_riddle.current_position, target_riddle.target_position)) {
                module_state->module_solved[4] = true;
            }

        }
    } else {
        button_pressed = false;
    }


    // Output positions
    set_led(target_riddle.green_positions[0], GREEN);
    set_led(target_riddle.green_positions[1], GREEN);
    set_led(target_riddle.target_position, RED);
    set_led(target_riddle.current_position, BLUE);
}
