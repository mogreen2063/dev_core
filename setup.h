#ifndef _SETUP_H
#define _SETUP_H

typedef enum {
  PULSE_INIT,
  PULSE_ON,
  PULSE_OFF
} PulseStates;

void setup(void);
void isr(void);

#endif	/* _SETUP_H */
