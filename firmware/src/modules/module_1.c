#include <stdio.h>
#include <stdlib.h>
#include "modules.h"
#include "non_blocking_timer.h"

static bool module_initialized = false;

#define BLINK_COLOR 0x1F1F1F
#define MORSE_SHORT 100
#define MORSE_LONG 500
#define MORSE_PAUSE 500

const uint8_t num_freqs = 8;
const uint16_t radio_pos[] = {3900, 3550, 3050, 2500, 1950, 1300, 850, 700};
const uint16_t radio_freq[] = {500, 510, 520, 530, 540, 550, 560, 570};

uint16_t target_freq = 0;

const uint8_t riddles[8][6] = {
    "slick",
    "trick",
    "leaks",
    "break",
    "boxes",
    "brick",
    "steak",
    "beats"
};

const char characters[13] = "slicktreabox";

const uint8_t morse_code[12][4] = {
    {1, 1, 1, 0},
    {1, 2, 1, 1},
    {1, 1, 0, 0},
    {2, 1, 2, 1},
    {2, 1, 2, 0},
    {2, 0, 0, 0},
    {1, 2, 1, 0},
    {1, 0, 0, 0},
    {1, 2, 0, 0},
    {2, 1, 1, 1},
    {2, 2, 2, 0},
    {2, 1, 1, 2}
};

uint8_t target_riddle;

bool is_morsing = false;

non_blocking_timer_handler morse_timer;

static void init_module(output_devices *output) {
    target_riddle = rand() % 8;
}

void module1_process(input_devices *input, output_devices *output, modules_state_t *module_state) {
    uint8_t chosen_frequency = 0;
    static bool button_pressed = false;
    static bool is_morsing = false;
    static uint32_t current_morse_pos = 0;
    static uint32_t morse_pause = 0;

    if (!module_initialized) {
        init_module(output);
        module_initialized = true;
    }

    if (!module_state->module_solved[0] && !is_morsing && input->radio_receive_key) {
        is_morsing = true;
        current_morse_pos = 0;
        morse_pause = MORSE_PAUSE;
    }

    if (is_morsing && non_blocking_timer_expired(&morse_timer)) {
        if (morse_pause) {
            output->radio_module_blink = 0;
            init_non_blocking_timer(&morse_timer, morse_pause);
            start_non_blocking_timer(&morse_timer);
            morse_pause = 0;
        } else if (current_morse_pos >= 5 * 4) {
            is_morsing = false;
        } else {
            char current_character = riddles[target_riddle][current_morse_pos / 4];
            uint8_t character_index;
            for (uint8_t i = 0; i < 13; i++) {
                if (current_character == characters[i]) character_index = i;
            }
            uint8_t morse_part = morse_code[character_index][current_morse_pos % 4];

            switch (morse_part) {
                case 1:
                    output->radio_module_blink = BLINK_COLOR;
                    init_non_blocking_timer(&morse_timer, MORSE_SHORT);
                    start_non_blocking_timer(&morse_timer);
                    break;
                case 2:
                    output->radio_module_blink = BLINK_COLOR;
                    init_non_blocking_timer(&morse_timer, MORSE_LONG);
                    start_non_blocking_timer(&morse_timer);
                    break;
            }

            if (current_morse_pos % 4 == 3) {
                morse_pause = MORSE_PAUSE*4;
            } else if (morse_part) {
                morse_pause = MORSE_PAUSE;
            }

            current_morse_pos++;
        }
    }

    if (!button_pressed && input->radio_transmit_key) {
        button_pressed = true;
        for (uint8_t i=0; i < num_freqs; i++) {
            if (input->poti_pos > radio_pos[i]) {
                chosen_frequency = i;
                break;
            }
        }

        if (chosen_frequency == target_riddle) {
            module_state->module_solved[0] = true;
        } else if (!module_state->module_solved[0]) {
           module_state->error_count += 1;
        }
    } else if (button_pressed && !input->radio_transmit_key) {
        button_pressed = false;
    }
}
