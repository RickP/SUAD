#ifndef CORE1_H
#define CORE1_H

#include "pico/stdlib.h"
#include "pico/multicore.h"

#define SHOW_LOOPS 0
#define SHOW_KEYPRESS 0

void core1_entry();

// Struct for output device states
typedef struct
{
  bool was_updated;
  uint8_t segment[3];
  uint32_t error_leds[3];
  uint32_t radio_module_state;
  uint32_t radio_module_blink;
  uint32_t button_module_state;
  uint32_t button_module_leds[4];
  uint32_t simon_module_state;
  uint32_t simon_module_blink;
  uint32_t dip_module_state;
  uint32_t dip_module_top[6];
  uint32_t dip_module_bottom[6];
  uint32_t maze_module_leds[5][5];
  uint32_t maze_module_state;
}  output_devices;

// Struct for input device states
typedef struct
{
  bool was_updated;
  bool serial_key;
  bool radio_receive_key;
  bool radio_transmit_key;
  bool button_key;
  bool simon_up_key;
  bool simon_down_key;
  bool simon_left_key;
  bool simon_right_key;
  bool matrix_up_key;
  bool matrix_down_key;
  bool matrix_left_key;
  bool matrix_right_key;
  bool dip_switches[6];
  uint16_t poti_pos;
  bool less_time_jumper;
  bool no_error_jumper;
}  input_devices;

void send_output(output_devices*);
bool get_output(output_devices*, bool);

void send_input(input_devices*);
bool get_input(input_devices*, bool);

#endif // CORE1_H
