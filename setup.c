#include <xc.h>
#include <stdint.h>
#include "setup.h"


void setup()
{
  INTCON = 0;

  /* setup oscillator */
  OSCTUNE = 0b11000000;		/* PLL enabled */
  OSCCON  = 0b01110100;
  T1CON   = 0;
  
  /* setup pin maps */
  PPSCONbits.IOLOCK = 0;
  
  asm("MOVLB 0X0E");
  asm("BCF INTCON, 7, C");
  asm("MOVLW 0X55");
  asm("MOVWF EECON2, C");
  asm("MOVLW 0XAA");
  asm("MOVWF EECON2, C");
  asm("BCF   PPSCON&0xff, 0, B");
  asm("MOVLW 0X0C");
  asm("MOVWF RPINR16&0xff, B");
  asm("MOVLW 0X05");
  asm("MOVWF RPOR11&0xff, B");
  asm("MOVLW 0X07");
  asm("MOVWF RPINR1&0xff, B");
  asm("MOVLW 0X55");
  asm("MOVWF EECON2, C");
  asm("MOVLW 0XAA");
  asm("MOVWF EECON2, C");
  asm("BSF   PPSCON&0xff, 0, B");
    
  /* setup adc */
  ADCON0 = 0;
  ADCON1 = 0;			/* adjust acqt, adcs */
  ANCON1bits.VBGEN = 0;		/* disable bandgap */
  
  /* setup comparator */
  CVRCON = 0;
  CM1CON = 0;
  
  /* setup porta */
  LATA   = 0b00000000;
  ANCON0 = 0b11111111;
  TRISA  = 0b00111111;

  /* setup portb */
  LATB   = 0b00000000;
  ANCON1 = ANCON1 | 0x17;
  TRISB  = 0b00010000;

  /* setup portc */
  LATC   = 0b01000000;
  ANCON1bits.PCFG11 = 0;
  TRISC  = 0b10000010;
  
  /* setup uart1 */
  TXSTA1   = 0b10100100;
  RCSTA1   = 0b10010000;
  BAUDCON1 = 0b00000000;
  SPBRGH1  = 0;
  SPBRG1   = 12;

  /* setup uart2 */
  TXSTA2   = 0b10100100;
  RCSTA2   = 0b10010000;
  BAUDCON2 = 0b00000000;
  SPBRGH2  = 0;
  SPBRG2   = 155;

  /* setup timer0 */
  T0CON = 0b00001000;
  TMR0H = 0;
  TMR0L = 0;

  /* setup timer1 */
  T1CON = 0b00000110;
  TMR1H = 0;
  TMR1L = 0;
  T1GCON = 0;
  TCLKCON = 0;

  /* setup interrupts */
  RCONbits.IPEN   = 0; 		/* disable priority */
  IPR1 = 0;
  IPR2 = 0;
  IPR3 = 0;

  PIR1bits.RC1IF  = 0;
  PIE1bits.RC1IE  = 1;
  PIR3bits.RC2IF  = 0;
  PIE3bits.RC2IE  = 1;

  INTCONbits.PEIE = 1;
  INTCONbits.GIE  = 1;

  /* setup watchdog */
  WDTCON = 0b0000000;
}
