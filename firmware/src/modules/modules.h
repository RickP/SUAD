#ifndef MODULES_H
#define MODULES_H

#include "pico/stdlib.h"
#include "core1.h"

#define RED 0x330000
#define GREEN 0x003300
#define BLUE 0x000033

// Struct for output device states
typedef struct
{
  uint8_t error_count;
  uint32_t current_time;
  uint8_t serial[3];
  bool module_solved[5];
}  modules_state_t;

void module1_process(input_devices *, output_devices *, modules_state_t *);
void module2_process(input_devices *, output_devices *, modules_state_t *);
void module3_process(input_devices *, output_devices *, modules_state_t *);
void module4_process(input_devices *, output_devices *, modules_state_t *);
void module5_process(input_devices *, output_devices *, modules_state_t *);

void success_animation(output_devices *);
void fail_animation(output_devices *);

#endif // MODULES_H
