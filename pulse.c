#include <xc.h>
#include <stdint.h>
#include "pulse.h"

#define _XTAL_FREQ 24000000

void pulse_init(Pulse_t * pulse)
{
  pulse->time = 0;
  pulse->ready = 0;
  pulse->flag = 0;
  pulse->timeout = 0;
  pulse->state = PULSE_INIT;
}

void handle_pulse(Pulse_t * pulse)
{
  switch(pulse->state) {
  case(PULSE_IDLE):
    break;
  case(PULSE_INIT):
    if(!PORTBbits.RB4) {
      /* reset timer0 */
      T0CONbits.TMR0ON = 0;
      TMR0H = 0;
      TMR0L = 0;

      /* setup edge trigger */
      INTCON3bits.INT1IF = 0;
      INTCON2bits.INTEDG1 = 1;
      INTCON3bits.INT1IE = 1;

      /* pulse */
      LATCbits.LATC2 = 1;
      __delay_us(10);
      LATCbits.LATC2 = 0;

      /* set time for timeout */
      pulse->timeout = 0;

      /* T1CONbits.TMR1ON = 0; */
      /* TMR1H = 0x00; */
      /* TMR1L = 0x00; */
      /* PIR1bits.TMR1IF = 0; */
      /* PIE1bits.TMR1IE = 1; */
      /* T1CONbits.TMR1ON = 1; */
	
      pulse->state = PULSE_ON;
    }
    break;
  case(PULSE_ON):
    /* if(pulse->timeout) */
    /*   { */
    /* 	pulse->state = PULSE_IDLE; */
    /*   } */
    if(pulse->flag) {
      pulse->time = TMR0L;
      pulse->time = pulse->time | (TMR0H << 8);
      /* pulse->time = pulse->time << 8; */
      /* pulse->time |= TMR0L; */
      
      pulse->flag = 0;
      pulse->ready = 1;
      
      pulse->state = PULSE_IDLE;
    }
    break;
  default:
    break;
  }
}
      
