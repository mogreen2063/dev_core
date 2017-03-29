#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "setup.h"
#include "port.h"
#include "wifi.h"
#include "pulse.h"

#define _XTAL_FREQ 24000000
#define SLEEP_COUNT 2
#define TIMEOUT_COUNT 915

typedef enum {
  SYS_IDLE,
  SYS_SLEEP,
  SYS_RUN
} SystemStates;

typedef enum {
  RUN_PULSE,
  RUN_SEND,
  RUN_SLEEP
} RunStates;

void init_var(void);

/* #pragma config WDTEN   = OFF */
#pragma config WDTEN   = ON
#pragma config WDTPS   = 1024	/* 4 sec */
#pragma config T1DIG   = OFF
#pragma config PLLDIV  = 2
#pragma config CPUDIV  = OSC2_PLL2
#pragma config OSC     = INTOSCPLLO
#pragma config XINST   = OFF
#pragma config DEBUG   = OFF
#pragma config CP0     = OFF
#pragma config IOL1WAY = OFF
#pragma config DSWDTEN = OFF

extern SetupStates setup_state;
extern ConnectStates connect_state;

volatile uint8_t uart1_buffer[256] @ 0x100;
volatile uint8_t uart2_buffer[256] @ 0x200;

uint8_t ssid[128] @ 0x300;
uint8_t pass[128] @ 0x380;

Port_t port1;
Port_t port2;

Buffer_t buffer1;
Buffer_t buffer2;

Wifi_t wifi;
Http_t http;

uint8_t key[16];

Pulse_t pulse;
uint8_t time_string[5];
SystemStates sys_state;
RunStates run_state;
volatile uint16_t timeout_counter;
uint8_t timeout_flag;

/* uint16_t sleep_count; */

void interrupt isr(void)
{
  if(PIR1bits.RC1IF) {
    buffer1.mem[buffer1.head] = RCREG1;
    if(buffer1.mem[buffer1.head++] == '\n') {
      buffer1.flag++;
    }
  }

  if(PIR3bits.RC2IF) {
    buffer2.mem[buffer2.head] = RCREG2;
    if(buffer2.mem[buffer2.head++] == '\n') {
      buffer2.flag++;
    }
  }
  
  if(INTCON3bits.INT1IF) {
    if(!pulse.level) {
      T0CONbits.TMR0ON = 1;
      INTCON2bits.INTEDG1 = 0; /* change to falling */
      INTCON3bits.INT1IF = 0;
      pulse.level = 1;
    }
    else {
      T0CONbits.TMR0ON = 0;
      INTCON2bits.INTEDG1 = 1; /* change to rising */
      INTCON3bits.INT1IF = 0;
      INTCON3bits.INT1IE = 0;	/* disable interrupt */
      pulse.level = 0;
      pulse.flag = 1;
    }
  }

  if(PIR1bits.TMR1IF) {
    /* T1CONbits.TMR1ON = 0; */
    PIR1bits.TMR1IF = 0;
    /* PIE1bits.TMR1IE = 0; */
    if(!(--timeout_counter)) {
      timeout_flag = 1;
    }
  }
}
void init_var(void)
{
  /* init externs */
  setup_state = SETUP_AT;
  connect_state = CONNECT_AT;
  
  /* init port1 */
  port1.p_state = P_IDLE;
  port1.c_state = C_IDLE;

  /* init buffer1 */
  buffer1.mem = uart1_buffer;
  buffer1.head = 0;
  buffer1.tail = 0;
  buffer1.echo = 0;
  buffer1.flag = 0;

  /* init port2 */
  port2.p_state = P_IDLE;
  port2.c_state = C_IDLE;

  /* init buffer2 */
  buffer2.mem = uart2_buffer;
  buffer2.head = 0;
  buffer2.tail = 0;
  buffer2.echo = 0;
  buffer2.flag = 0;

  /* init wifi */
  wifi.state = W_IDLE;
  wifi.ok = 0;
  wifi.ready = 0;
  wifi.closed = 0;
  wifi.ssid = ssid;
  wifi.pass = pass;
  strcpy(wifi.ssid,(const uint8_t *)"164auburn-2.4");
  strcpy(wifi.pass,(const uint8_t *)"novultures2017");
  
  /* init http */
  http.request = 0;
  http.packet_length = 0;
  http.content_length = 0;
  http.key = key;
  http.data = 0;
  /* strcpy(http.key,(const uint8_t *)"0"); */

  /* init pulse */
  pulse.state = PULSE_IDLE;
  pulse.time = 0;
  pulse.level = 0;
  pulse.ready = 0;
  pulse.flag = 0;
  pulse.timeout = 0;

  /* init system state */
  sys_state = SYS_IDLE;
  run_state = RUN_PULSE;
  /* sleep_count = SLEEP_COUNT; */

  /* init timeout */
  T1CONbits.TMR1ON = 0;	  
  timeout_counter = TIMEOUT_COUNT;
  timeout_flag = 0;
  TMR1H = 0x00;
  TMR1L = 0x00;
  PIR1bits.TMR1IF = 0;
  PIE1bits.TMR1IE = 1;
}  

int main(void)
{
  setup();
  init_var();
  wifi.init = 0;
  /*  /\* init port1 *\/ */
  /*  port1.p_state = P_IDLE; */
  /*  port1.c_state = C_IDLE; */

  /*  /\* init buffer1 *\/ */
  /*  buffer1.mem = uart1_buffer; */
  /*  buffer1.head = 0; */
  /*  buffer1.tail = 0; */
  /*  buffer1.echo = 0; */
  /*  buffer1.flag = 0; */

  /*  /\* init port2 *\/ */
  /*  port2.p_state = P_IDLE; */
  /*  port2.c_state = C_IDLE; */

  /*  /\* init buffer2 *\/ */
  /*  buffer2.mem = uart2_buffer; */
  /*  buffer2.head = 0; */
  /*  buffer2.tail = 0; */
  /*  buffer2.echo = 0; */
  /*  buffer2.flag = 0; */

  /*  /\* init wifi *\/ */
  /*  wifi.state = W_IDLE; */
  /*  wifi.ok = 0; */
  /*  wifi.ready = 0; */
  /*  wifi.closed = 0; */
  /*  wifi.ssid = ssid; */
  /*  wifi.pass = pass; */
  /*  strcpy(wifi.ssid,(const uint8_t *)"164auburn-2.4"); */
  /*  strcpy(wifi.pass,(const uint8_t *)"novultures2017"); */
  
  /*  /\* init http *\/ */
  /*  http.request = 0; */
  /*  http.packet_length = 0; */
  /*  http.content_length = 0; */
  /*  http.key = key; */
  /*  http.data = 0; */
  /*  strcpy(http.key,(const uint8_t *)"0"); */

  /*  /\* init pulse *\/ */
  /*  pulse.state = PULSE_IDLE; */
  /*  pulse.time = 0; */
  /*  pulse.level = 0; */
  /*  pulse.ready = 0; */
  /*  pulse.flag = 0; */
  /*  pulse.timeout = 0; */

  /*  /\* init system state *\/ */
  /*  sys_state = SYS_IDLE; */
  /*  run_state = RUN_PULSE; */
  /*  timeout_counter = TIMEOUT_COUNT; */
  /* /\* sleep_count = SLEEP_COUNT; *\/ */

  while(1)
    {
      ClrWdt();

      if(timeout_flag)
	{
	  init_var();
	  send_msg((const uint8_t *)"***SYSTEM RESET***",1);
	  /* reset power on wifi */
	}

      handle_port1(&port1, &buffer1, &wifi, &http);
      handle_port2(&port2, &buffer2, &wifi, &http);
      handle_cmd(&port1, &buffer1, &wifi, &http);

      switch(sys_state) {
      case(SYS_IDLE):
	if(wifi.init)
	  {
	    T1CONbits.TMR1ON = 0;
	    timeout_counter = TIMEOUT_COUNT;
	    TMR1H = 0x00;
	    TMR1L = 0x00;
	    PIR1bits.TMR1IF = 0;
	    PIE1bits.TMR1IE = 1;
	    T1CONbits.TMR1ON = 1;

	    pulse_init(&pulse);
	    sys_state = SYS_RUN;
	  }
	break;
      case(SYS_RUN):
	switch(run_state) {
	case(RUN_PULSE):
	  handle_pulse(&pulse);
	  /* if(pulse.timeout) */
	  /*   { */
	  /*     pulse.timeout = 0; */
	  /*     sys_state = SYS_IDLE; */
	  /*   } */
	  if(pulse.ready)
	    {
	      /* add to running average */
	      http.data = pulse.time;
	      pulse.ready = 0;
	      run_state = RUN_SEND;
	    }
	  break;
	case(RUN_SEND):
	  if(port1.c_state == C_IDLE)
	    {
	      http.request = 2;
	      port1.c_state = C_CONNECT;
	      run_state = RUN_SLEEP;
	    }
	  break;
	case(RUN_SLEEP):
	  if(port1.c_state == C_IDLE)
	    {
	      run_state = RUN_PULSE;
	      sys_state = SYS_SLEEP;
	    }
	  break;
	default:
	  break;
	}
	break;
      case(SYS_SLEEP):
	while(!TXSTA1bits.TRMT);
	while(!TXSTA2bits.TRMT);
	TRISCbits.TRISC0 = 1;
	T1CONbits.TMR1ON = 0;

	Sleep();
	/* if(!(--sleep_count)) */
	/*   { */
	/*     TRISCbits.TRISC0 = 0; */
	/*     sleep_count = SLEEP_COUNT; */
	/*     sys_state = SYS_RUN; */
	/*   } */
	sys_state = SYS_IDLE;
	break;
      default:
	break;
      }
    }
}
