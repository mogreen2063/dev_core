#include <p18f24j50.h>
#include <usart.h>
#include "setup.h"

#pragma code
void setup()
{
  INTCON = 0;
  
  // setup oscillator
  OSCTUNE = 0b11000000; //PLL enabled
  OSCCON  = 0b01110100;
  T1CON   = 0;
  
  // setup pin maps
  PPSCONbits.IOLOCK = 0;
  _asm
    MOVLB	0X0E
    BCF	        INTCON, 7, ACCESS
    MOVLW	0X55
    MOVWF	EECON2, ACCESS
    MOVLW	0XAA
    MOVWF	EECON2, ACCESS
    BCF	        PPSCON, 0, BANKED
    MOVLW	0X0C
    MOVWF	RPINR16, BANKED
    MOVLW	0X05
    MOVWF	RPOR11, BANKED
    MOVLW       0X07
    MOVWF       RPINR1, BANKED
    MOVLW	0X55
    MOVWF	EECON2, ACCESS
    MOVLW	0XAA
    MOVWF	EECON2, ACCESS
    BSF	        PPSCON, 0, BANKED
  _endasm

  // setup adc
  ADCON0 = 0;
  ADCON1 = 0;           // adjust acqt, adcs
  ANCON1bits.VBGEN = 0; // disable bandgap

  // setup comparator
  CVRCON = 0;
  CM1CON = 0;
  
  // setup porta
  LATA   = 0b00000000;
  ANCON0 = 0b11111111;
  TRISA  = 0b00111111;

  // setup portb
  LATB   = 0b00000000;
  ANCON1 = ANCON1 | 0x17;
  TRISB  = 0b00010000;

  // setup portc
  LATC   = 0b01000000;
  ANCON1bits.PCFG11 = 0;
  TRISC  = 0b10000010;
  
  // setup uart1
  Open1USART(USART_TX_INT_OFF &
	     USART_RX_INT_OFF &
	     USART_EIGHT_BIT &
  	     USART_BRGH_HIGH &
  	     USART_ASYNCH_MODE &
  	     USART_CONT_RX &
	     USART_ADDEN_OFF,
	     12); // ~115200 baud
  
  baud1USART(BAUD_IDLE_CLK_LOW &
	     BAUD_8_BIT_RATE &
	     BAUD_WAKEUP_OFF &
	     BAUD_AUTO_OFF);

  BAUDCON1bits.RXDTP = 0;

  // setup uart2
  Open2USART(USART_TX_INT_OFF &
	     USART_RX_INT_OFF &
	     USART_EIGHT_BIT &
  	     USART_BRGH_HIGH &
  	     USART_ASYNCH_MODE &
  	     USART_CONT_RX &
	     USART_ADDEN_OFF,
	     12); // ~115200 baud

  baud2USART(BAUD_IDLE_CLK_LOW &
	     BAUD_8_BIT_RATE &
	     BAUD_WAKEUP_OFF &
	     BAUD_AUTO_OFF);
  
  BAUDCON2bits.RXDTP = 0;
  
  // setup timer0
  T0CON = 0b00001000;
  TMR0H = 0;
  TMR0L = 0;

  // setup interrupts
  RCONbits.IPEN   = 0; //disable priority
  PIR1bits.RC1IF  = 0;
  PIE1bits.RC1IE  = 1;
  PIR3bits.RC2IF  = 0;
  PIE3bits.RC2IE  = 1;

  INTCONbits.PEIE = 1;
  INTCONbits.GIE  = 1;

}
