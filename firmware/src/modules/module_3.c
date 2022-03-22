#include <stdio.h>
#include <stdlib.h>
#include "modules.h"
#include "non_blocking_timer.h"

#define MODULE_NUM 2
#define BLINK_PERIOD 300
#define SEQUENCE_LENGTH 5

static uint32_t sequence[SEQUENCE_LENGTH];
static uint32_t up_color;
static uint32_t down_color;
static uint32_t left_color;
static uint32_t right_color;

static bool module_initialized = false;
static non_blocking_timer_handler blink;

static void init_module(output_devices *output) {
    // generate a random sequence with 6 phases
    for (uint8_t i=0; i<SEQUENCE_LENGTH; i++) {
        switch (rand() % 4) {
            case 0:
                sequence[i] = GREEN;
                break;
            case 1:
                sequence[i] = RED;
                break;
            case 2:
                sequence[i] = BLUE;
                break;
            case 3:
                sequence[i] = YELLOW;
                break;
        }
    }

    if (output->segment[0] == 0x0A || output->segment[1] == 0x0A || output->segment[2] == 0x0A) {
        up_color = RED;
        down_color = GREEN;
        left_color = YELLOW;
        right_color = BLUE;
    } else {
        up_color = YELLOW;
        down_color = RED;
        left_color = BLUE;
        right_color = GREEN;
    }
    init_and_start_non_blocking_timer(&blink, BLINK_PERIOD*5);
}

void module3_process(input_devices *input, output_devices *output, modules_state_t *module_state) {
    static bool key_pressed = false;
    static bool toggle_led = true;
    static uint8_t correct_keypresses = 0;
    static uint8_t display_loop = 0;
    static uint8_t current_sequence_num = 0;

    if (!module_initialized) {
        init_module(output);
        module_initialized = true;
    }

    if (!module_state->module_solved[MODULE_NUM]) {
        if (!key_pressed && (input->simon_up_key || input->simon_down_key || input->simon_left_key || input->simon_right_key)) {
            key_pressed = true;
            if (input->simon_up_key) {
                if (sequence[correct_keypresses] == up_color) {
                   correct_keypresses++;
                } else {
                   module_state->error_count += 1;
                   correct_keypresses = 0;
                }
            } else if (input->simon_down_key) {
                if (sequence[correct_keypresses] == down_color) {
                   correct_keypresses++;
                } else {
                   module_state->error_count += 1;
                   correct_keypresses = 0;
                }
            } else if (input->simon_left_key) {
                if (sequence[correct_keypresses] == left_color) {
                   correct_keypresses++;
                } else {
                   module_state->error_count += 1;
                   correct_keypresses = 0;
                }
            } else if (input->simon_right_key) {
                if (sequence[correct_keypresses] == right_color) {
                   correct_keypresses++;
                } else {
                   module_state->error_count += 1;
                   correct_keypresses = 0;
                }
            }

            if (correct_keypresses > current_sequence_num) {
                current_sequence_num++;
                display_loop = current_sequence_num + 1;
                toggle_led = false;
                correct_keypresses = 0;
            }

        } else if (key_pressed && !input->simon_up_key && !input->simon_down_key && !input->simon_left_key && !input->simon_right_key) {
            key_pressed = false;
        }

        if (current_sequence_num == SEQUENCE_LENGTH) {
            module_state->module_solved[MODULE_NUM] = true;
            output->simon_module_blink = 0;
        } else  {
            // Blink the LED
            if (non_blocking_timer_expired(&blink)) {
                if (toggle_led) {
                    output->simon_module_blink = sequence[display_loop];
                    init_and_start_non_blocking_timer(&blink, BLINK_PERIOD);
                    display_loop++;
                } else {
                    output->simon_module_blink = 0;
                    if (display_loop > current_sequence_num) {
                        display_loop = 0;
                        init_and_start_non_blocking_timer(&blink, BLINK_PERIOD*5);
                    } else {
                        init_and_start_non_blocking_timer(&blink, BLINK_PERIOD);
                    }
                }

                toggle_led = !toggle_led;
            }
        }
    }
}
