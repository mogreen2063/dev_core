#include <p18f24j50.h>
#include <usart.h>
#include <delays.h>
#include <string.h>
#include "stdint.h"
#include "wifi.h"
#include "uart.h"
#include "setup.h"
#include "cmd.h"

#pragma config WDTEN   = OFF
#pragma config T1DIG   = OFF
#pragma config PLLDIV  = 2
#pragma config CPUDIV  = OSC2_PLL2
#pragma config OSC     = INTOSCPLLO
#pragma config XINST   = OFF
#pragma config DEBUG   = OFF
#pragma config CP0     = OFF
#pragma config IOL1WAY = OFF

#pragma udata udata1
int8_t uart1_buffer[BUFFER1_SIZE];

#pragma udata udata2
int8_t uart2_buffer[BUFFER2_SIZE];

#pragma udata udata3
far int8_t ssid[128];
far int8_t pass[128];

#pragma udata
Buffer_t buffer1;
Buffer_t buffer2;
  
far int8_t http_key[16];

far int8_t time_string[5];
far uint16_t time;
far uint16_t pulse;
far uint8_t pulse_flag;
far uint8_t time_flag;

#pragma idata
far int8_t url[] = "http://api.myjson.com/bins/";

#pragma code isr = 0x08
#pragma interrupt isr
void isr(void)
{
  if(PIR1bits.RC1IF) {
    buffer1.mem[buffer1.head] = Read1USART();
    if(buffer1.mem[buffer1.head++] == '\n') {
      buffer1.flag++;
    }
  }
  
  if(PIR3bits.RC2IF) {
    buffer2.mem[buffer2.head] = Read2USART();
    if(buffer2.mem[buffer2.head++] == '\n') {
      buffer2.flag++;
    }
  }

  if(INTCON3bits.INT1IF) {
    if(!pulse_flag)
      {
	T0CONbits.TMR0ON = 1;	/* start timer */
	INTCON2bits.INTEDG1 = 0; /* change to falling */
	INTCON3bits.INT1IF = 0;
	pulse_flag = 1;
      }
    else
      {
	T0CONbits.TMR0ON = 0;	/* stop timer */
	INTCON2bits.INTEDG1 = 1; /* change to rising */
	INTCON3bits.INT1IF = 0;
	pulse_flag = 0;
	time_flag = 1;
      }
  }
}
  
#pragma code
void get_time(far int8_t * time_string)
{
  int8_t digit;
  
  digit  = (int8_t)((time & 0xf000) >> 12);
  digit += '0';
  if (digit > '9')
    {
      digit +=0x7;
    }
  time_string[0] = digit;
  
  digit  = (int8_t)((time & 0x0f00) >> 8);
  digit += '0';
  if (digit > '9')
    {
      digit +=0x7;
    }
  time_string[1] = digit;

  digit  = (int8_t)((time & 0x00f0) >> 4);
  digit += '0';
  if (digit > '9')
    {
      digit +=0x7;
    }
  time_string[2] = digit;

  digit  = (int8_t)(time & 0x000f);
  digit += '0';
  if (digit > '9')
    {
      digit +=0x7;
    }
  time_string[3] = digit;
}

void copy_msg(int8_t * mem, uint8_t index, far int8_t * msg)
{
  do {
    *msg++ = mem[index++];
  } while(mem[index] != '\r');
  *msg = '\0';
}

void main(void)
{
  /* setup to rcv a key from comp(could be "AT" but that assumes junk is NULL) */
  /* the key must be sent by the computer. if junk is not NULL then another */
  /* can be sent to verify, but the buffer would have been flushed by the first */

  Port_t port1;
  PortStates port1_state = P_IDLE; /* init all after setup() */
  PortStates port2_state = P_IDLE;
  CmdStates cmd_state = C_IDLE;
  Wifi_t wifi;
  Http_t http;
  PulseStates pulse_state = PULSE_INIT;

  uint32_t time_sum = 0;
  int8_t digit;
  uint8_t pulse_count;
  
  setup();

  port1.state = P_IDLE;
  port1.buffer = uart1_buffer;
  wifi.state = W_IDLE;
  wifi.ok = 0;
  wifi.ssid = ssid;
  wifi.pass = pass;
  wifi.url = url;

  http.key = http_key;		/* init rest of http */

  buffer1.mem = uart1_buffer;
  buffer1.head = 0;
  buffer1.tail = 0;
  buffer1.echo = 0;
  buffer1.flag = 0;

  buffer2.mem = uart2_buffer;
  buffer2.head = 0;
  buffer2.tail = 0;
  buffer2.echo = 0;
  buffer2.flag = 0;


  time_string[0] = '0';		/* use strcpy */
  time_string[1] = '0';
  time_string[2] = '0';
  time_string[3] = '0';
  time_string[4] = '\0';

  time = 0;

  /* ssid[0] = '0'; 		/\* use strcpy *\/ */
  /* ssid[1] = '\0'; */
 
  /* pass[0] = '0';		/\* use strcpy *\/ */
  /* pass[1] = '\0'; */

  while(1)
    {
      /* send_msg_ram(http_content_length,1); */
      /* send_msg(crlf,1); */

      /* http_packet_length = http_get_packet_length(0); */
      /* hex2dec(http_content_length,http_packet_length); */
      /* send_msg_ram(http_content_length,1); */
      /* send_msg(crlf,1); */
      
      /* while(1); */
      
      /* implement WDT */
      /* switch(pulse_state) { */
      /* case(PULSE_INIT): */
      /* 	if(!PORTBbits.RB0) { */
      /* 	  INTCON3bits.INT1IF = 0; */
      /* 	  INTCON2bits.INTEDG1 = 1; */
      /* 	  INTCON3bits.INT1IE = 1; */
      /* 	  time_flag = 0; */
      /* 	  time_sum = 0; */
      /* 	  pulse_count = 16; */
      /* 	  pulse_state = PULSE_ON; */
      /* 	  TMR0H = 0;		/\* reset timer0 *\/ */
      /* 	  TMR0L = 0; */
      /* 	  LATCbits.LATC2 = 1;	 /\* start pulse *\/ */
      /* 	  Delay10TCYx(6); */
      /* 	  LATCbits.LATC2 = 0; */
      /* 	} */
      /* 	break; */
      /* case(PULSE_ON): */
      /* 	if(pulse_count) */
      /* 	  { */
      /* 	    if(time_flag) { */
      /* 	      pulse_count--; */
      /* 	      time = TMR0L;		/\* read timer *\/ */
      /* 	      time = TMR0H; */
      /* 	      time = time << 8; */
      /* 	      time |= TMR0L; */

      /* 	      get_time(time_string); */
      /* 	      send_msg_ram(time_string,1); */
      /* 	      send_msg(wifi_cmds[0],1); */

      /* 	      time_sum = time_sum + ((uint32_t)time); */
      /* 	      time_flag = 0; */
      /* 	      TMR0H = 0;		/\* reset timer0 *\/ */
      /* 	      TMR0L = 0; */
      /* 	      LATCbits.LATC2 = 1;	 /\* start pulse *\/ */
      /* 	      Delay10TCYx(6); */
      /* 	      LATCbits.LATC2 = 0; */
      /* 	      Delay10KTCYx(60); */
      /* 	    } */
      /* 	  } */
      /* 	else */
      /* 	  { */
      /* 	    time_sum = time_sum >> 4; */
      /* 	    time = time_sum; */
      /* 	    get_time(time_string); */
      /* 	    send_msg(wifi_cmds[0],1); */
      /* 	    send_msg_ram(time_string,1); */
      /* 	    send_msg(wifi_cmds[0],1); */
      /* 	    pulse_state = PULSE_OFF; */
      /* 	  } */
      /* 	break; */
      /* case(PULSE_OFF): */
      /* 	/\* enter sleep *\/ */
      /* 	Delay10KTCYx(255); */
      /* 	pulse_state = PULSE_INIT; */
      /* 	break; */
      /* default: */
      /* 	break; */
      /* } */

      /* time++; */
      /* get_time(time_string); */

      handle_port1(&buffer1, &port1_state, &cmd_state, &wifi);
      /* handle_port1(&buffer1, &port1_state, &cmd_state, &wifi); */
      handle_port2(&buffer2, &port2_state, &wifi, &http);
      handle_cmd(&buffer1, &port1_state, &cmd_state, &wifi, &http);
    }
}
