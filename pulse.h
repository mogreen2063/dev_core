#ifndef _PULSE_H
#define _PULSE_H

#include <xc.h>
#include <stdint.h>

typedef enum {
  PULSE_IDLE,
  PULSE_INIT,
  PULSE_ON
} PulseStates;

typedef struct {
  PulseStates state;
  uint16_t time;
  uint8_t level;
  uint8_t ready;
  volatile uint8_t flag;
  volatile uint8_t timeout;
} Pulse_t;

void pulse_init(Pulse_t * pulse);
void handle_pulse(Pulse_t * pulse);

#endif
