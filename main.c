#include <p18f24j50.h>
#include <usart.h>
#include <delays.h>
#include <string.h>
#include "stdint.h"
#include "setup.h"

#pragma config WDTEN   = OFF
#pragma config T1DIG   = OFF
#pragma config PLLDIV  = 2
#pragma config CPUDIV  = OSC2_PLL2
#pragma config OSC     = INTOSCPLLO
#pragma config XINST   = OFF
#pragma config DEBUG   = OFF
#pragma config CP0     = OFF
#pragma config IOL1WAY = OFF

#pragma udata udata1=0x200
int8_t uart1_buffer[BUFFER1_SIZE];

#pragma udata udata2=0x300
int8_t uart2_buffer[BUFFER2_SIZE];

#pragma udata udata3=0x400
far int8_t ssid[128];
far int8_t pass[128];

#pragma udata
Buffer_t buffer1;
Buffer_t buffer2;
far int8_t key[16];
far int8_t time_string[5];
far uint16_t time;
far uint16_t pulse;
far uint8_t pulse_flag;
far uint8_t time_flag;

#pragma romdata
const far rom int8_t cmsg[][8] = {"setup",
				  "connect",
				  "upload",
				  "ssid",
				  "pass",
				  "url"};

const far rom int8_t connection_mode[] = "\"TCP\",";
const far rom int8_t ip[] = "\"api.myjson.com\",";
const far rom int8_t port[] = "80";

const far rom int8_t wmsg[][36] = {"OK\r\n",
				   "WIFI GOT IP\r\n",
				   "{\"uri\":\"http://api.myjson.com/bins/"};
const far rom int8_t url[] = "http://api.myjson.com/bins/";
const far rom int8_t wifi_cmds[][16] = {"\r\n",
					"AT\r\n",
					"AT+CWMODE=1\r\n",
					"AT+RST\r\n",
					"AT+CWDHCP=1,1\r\n",
					"AT+CWJAP=",
					"AT+CIPSTART=",
					"AT+CIPSEND=",
					"AT+CIPCLOSE\r\n"};

const far rom int8_t http_header[][33] = {"HOST: api.myjson.com",
					  "CONTENT-TYPE: application/json",
					  "CONTENT-LENGTH: ",
					  "CONNECTION: close"};

const far rom int8_t http_post[] = "POST /bins HTTP/1.1";
const far rom int8_t post_packet_length[]  = "132";
const far rom int8_t post_content_length[] = "9";
const far rom int8_t post_content[] = "{\"0\":\"0\"}";

const far rom int8_t http_put[][11] = {"PUT /bins/",
				       " HTTP/1.1"};
const far rom int8_t put_packet_length1[]   = "144";
const far rom int8_t put_packet_length2[]   = "145";

const far rom int8_t put_content_length[]  = "15";
const far rom int8_t put_content[][8] = {"{\"",
					 "time\":\"",
					 "\"}"};

#pragma code InterruptVectorHigh = 0x08
void InterruptVectorHigh(void)
{_asm goto InterruptHandlerHigh _endasm}

#pragma interrupt InterruptHandlerHigh
void InterruptHandlerHigh()
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
  
void get_key(int8_t * mem, int8_t * key, uint8_t key_offset)
{
  do {
    *key++ = mem[key_offset++];
  } while(mem[key_offset] != '\"');
  *key = '\0';
}

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

void send_char(const far rom int8_t * c, uint8_t dest)
{
  if(dest == 1)
    {
      while(Busy1USART());
      Write1USART(*c);
    }
  else if(dest == 2)
    {
      while(Busy2USART());
      Write2USART(*c);
    }
}

void send_char_ram(int8_t * c, uint8_t dest)
{
  if(dest == 1)
    {
      while(Busy1USART());
      Write1USART(*c);
    }
  else if(dest == 2)
    {
      while(Busy2USART());
      Write2USART(*c);
    }
}

void send_msg_ram(far int8_t * msg, uint8_t dest)
{
  if(dest == 1)
    {
      do {
	while(Busy1USART());
	Write1USART(*msg);
      } while(*(++msg));
    }
  else if(dest == 2)
    {
      do {
	while(Busy2USART());
	Write2USART(*msg);
      } while(*(++msg));
    }
}

void send_msg(const far rom int8_t * msg, uint8_t dest)
{
  if(dest == 1)
    {
      do {
	while(Busy1USART());
	Write1USART(*msg);
      } while(*(++msg));
    }
  else if(dest == 2)
    {
      do {
	while(Busy2USART());
	Write2USART(*msg);
      } while(*(++msg));
    }

}

uint8_t msg_check(int8_t * mem, uint8_t index,
		  const far rom int8_t * msg)
{
  uint8_t n = strlenpgm(msg);
 
  while(n--)
    {
      /* while(Busy1USART()); */
      /* Write1USART(mem[index]); */
      /* while(Busy1USART()); */
      /* Write1USART(*msg); */

      if(mem[index++] != *msg++)
	{ return 0; }
   }
  return 1;
}

void main(void)
{

  /* setup to rcv a key from comp(could be "AT" but that assumes junk is NULL) */
  /* the key must be sent by the computer. if junk is not NULL then another */
  /* can be sent to verify, but the buffer would have been flushed by the first */
  PortStates port1_state = P_IDLE;
  PortStates port2_state = P_IDLE;
  WifiStates wifi_state = W_IDLE;
  CmdStates cmd_state = C_IDLE;
  SetupStates setup_state = SETUP_AT;
  ConnectStates connect_state = CONNECT_AT;
  PulseStates pulse_state = PULSE_INIT;

  uint8_t wifi_ok;
  uint8_t wifi_connected;
  uint8_t wifi_ip;
  uint8_t wifi_send_ready;
  
  uint8_t cmd_busy = 0;
  uint8_t key_offset = 0;
  uint8_t connect_init = 1;

  uint32_t time_sum = 0;
  int8_t digit;
  uint8_t pulse_count;

  setup();

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

  wifi_ok = 0;
  wifi_connected = 0;
  wifi_ip = 0;
  wifi_send_ready = 0;

  key_offset = 0;

  time_string[0] = '0';		/* use strcpy */
  time_string[1] = '0';
  time_string[2] = '0';
  time_string[3] = '0';
  time_string[4] = '\0';

  time = 0;

  ssid[0] = '0'; 		/* use strcpy */
  ssid[1] = '\0';
 
  pass[0] = '0';		/* use strcpy */
  pass[1] = '\0';

  while(1)
    {
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
      get_time(time_string);

      /* computer port echo */
      if(buffer1.head != buffer1.echo)
	{
	  while(Busy1USART());	/* use  send_msg */
	  Write1USART(buffer1.mem[buffer1.echo++]);
	}

      /* computer port state machine */
      switch(port1_state) {
      case(P_IDLE):
      	if(buffer1.flag)
	  {
	  port1_state = P_READY;
	  }
	break;
      case(P_READY):
	if(!cmd_busy)
	  {
	    if(msg_check(buffer1.mem,buffer1.tail,cmsg[0]))
	      {
	    	cmd_state = C_SETUP;
	    	port1_state = P_FLUSH;
	      }
	    else if(msg_check(buffer1.mem,buffer1.tail,cmsg[1]))
	      {
	    	cmd_state = C_CONNECT;
	    	port1_state = P_FLUSH;
	      }
	    else if(msg_check(buffer1.mem,buffer1.tail,cmsg[2]))
	      {
	    	connect_init = 0;
	    	cmd_state = C_CONNECT;
	    	port1_state = P_FLUSH;
	      }
	    else if(msg_check(buffer1.mem,buffer1.tail,cmsg[3]))
	      {
	    	cmd_state = C_SSID;
	      }
	    else if(msg_check(buffer1.mem,buffer1.tail,cmsg[4]))
	      {
	    	cmd_state = C_PASS;
	      }
	    else if(msg_check(buffer1.mem,buffer1.tail,cmsg[5]))
	      {
	    	cmd_state = C_URL;
	      }
	    else
	      {
		port1_state = P_TRANSMIT;
	      }
	  }
	break;
      case(P_TRANSMIT):
	do {
	  while(Busy2USART());
	  Write2USART(buffer1.mem[buffer1.tail]);
	  /* send_char_ram(buffer1.mem[buffer1.tail],2); */
	} while(buffer1.mem[buffer1.tail++] != '\n');
	/* while(buffer1.mem[buffer1.tail] != '\n') */
	/*   { */
	/*     send_char(buffer1.mem[buffer1.tail++],2); */
	/*   } */
	/* while(Busy2USART()); */
	/* Write2USART(buffer1.mem[buffer1.tail++]); */
	port1_state = P_IDLE;
	buffer1.flag--;

	break;
      case(P_FLUSH):
	while(buffer1.mem[buffer1.tail++] != '\n');
	port1_state = P_IDLE;
	buffer1.flag--;
	break;
      default:
	break;
      }

      switch(cmd_state) {
      case(C_IDLE):
	break;
      case(C_MON):
	break;
      case(C_SETUP):
	switch(setup_state) {
	case(SETUP_AT):
	  cmd_busy = 1;
	  wifi_ok = 0;
	  send_msg(wifi_cmds[1],2);
	  setup_state = SETUP_CWMODE;
	  break;
	case(SETUP_CWMODE):
	  if(wifi_ok)
	    {
	      wifi_ok = 0;
	      send_msg(wifi_cmds[2],2);
	      setup_state = SETUP_RST;
	    }
	  break;
	case(SETUP_RST):
	  if(wifi_ok)
	    {
	      wifi_ok = 0;
	      send_msg(wifi_cmds[3],2);
	      setup_state = SETUP_CWDHCP;
	    }
	  break;
	case(SETUP_CWDHCP):
	  if(wifi_ok)
	    {
	      wifi_ok = 0;
	      wifi_connected = 0;
	      send_msg(wifi_cmds[4],2);
	      setup_state = SETUP_CWJAP;
	    }
	  break;
	case(SETUP_CWJAP):
	  if(wifi_ok)
	    {
	      wifi_ok = 0;
	      send_msg(wifi_cmds[5],2);
	      send_msg((const far rom int8_t *)"\"",2);
	      send_msg_ram(ssid,2);	
	      send_msg((const far rom int8_t *)"\",\"",2);
	      send_msg_ram(pass,2);
	      send_msg((const far rom int8_t *)"\"",2);
	      send_msg(wifi_cmds[0],2);
	      setup_state = SETUP_AT;
	      cmd_state = C_IDLE;
	      cmd_busy = 0;
	    }
	  break;
	default:
	  break;
	}
	break;
      case(C_CONNECT):
	if(wifi_connected)
	  {
	    switch(connect_state) {
	    case(CONNECT_AT):
	      wifi_ok = 0;
	      cmd_busy = 1;
	      send_msg(wifi_cmds[1],2);
	      connect_state = CONNECT_CIPSTART;
	      break;
	    case(CONNECT_CIPSTART):
	      if(wifi_ok)
		{
 		  wifi_ok = 0;
		  send_msg(wifi_cmds[6],2);
		  send_msg(connection_mode,2);
		  send_msg(ip,2);
		  send_msg(port,2);
		  send_msg(wifi_cmds[0],2);
		  connect_state = CONNECT_CIPSEND;
		}
	      break;
	    case(CONNECT_CIPSEND):
	      if(wifi_ok)
		{
 		  wifi_ok = 0;
		  wifi_send_ready = 0;
		  send_msg(wifi_cmds[7],2);
		  if(connect_init)
		    {
		      send_msg(post_packet_length,2);
		    }
		  else
		    {
		      if (strlen(key) == 5)
			{
			  send_msg(put_packet_length1,2);
			}
		      else if (strlen(key) == 6)
			{
			  send_msg(put_packet_length2,2);
			}
		    }		      
		  send_msg(wifi_cmds[0],2);
		  connect_state = CONNECT_CIPSEND_DATA;
		}
	      break;
	    case(CONNECT_CIPSEND_DATA):
	      if(wifi_ok)
		{
		  if(connect_init) {
		    send_msg(http_post,2);
		  }
		  else {
		    send_msg(http_put[0],2);
		    send_msg_ram(key,2);
		    send_msg(http_put[1],2);
		  }
		  send_msg(wifi_cmds[0],2);
		  
		  send_msg(http_header[0],2);
		  send_msg(wifi_cmds[0],2);

		  send_msg(http_header[1],2);
		  send_msg(wifi_cmds[0],2);

		  send_msg(http_header[2],2);
		  if(connect_init) {
		    send_msg(post_content_length,2);
		  }
		  else {
		    send_msg(put_content_length,2);
		  }
		  send_msg(wifi_cmds[0],2);

		  send_msg(http_header[3],2);
		  send_msg(wifi_cmds[0],2);
		  send_msg(wifi_cmds[0],2);
		  
		  if(connect_init) {
		    send_msg(post_content,2);
		  }
		  else {
		    send_msg(put_content[0],2);
		    send_msg(put_content[1],2);
		    send_msg_ram(time_string,2);
		    send_msg(put_content[2],2);
		  }
		  send_msg(wifi_cmds[0],2);
		  send_msg(wifi_cmds[0],2);
		  send_msg(wifi_cmds[0],2);
		  send_msg(wifi_cmds[0],2);
		  
		  connect_state = CONNECT_EXIT;
		}
	      break;
	    case(CONNECT_CLOSE):
	      send_msg(wifi_cmds[8],2);
	      connect_state = CONNECT_EXIT;
	      break;
	    case(CONNECT_EXIT):
	      cmd_busy = 0;
	      connect_init = 0;
	      wifi_send_ready = 0;
	      connect_state = CONNECT_AT;
	      cmd_state = C_IDLE;
	      break;
	    default:
	      break;
	    }
	  }
	break;
      case(C_SSID):
	copy_msg(buffer1.mem, buffer1.tail += 5, ssid);
	cmd_state = C_IDLE;
	port1_state = P_FLUSH;
	break;
      case(C_PASS):
	copy_msg(buffer1.mem, buffer1.tail += 5, pass);
	cmd_state = C_IDLE;
	port1_state = P_FLUSH;
	break;
      case(C_URL):
	send_msg(url,1);
	send_msg_ram(key,1);
	send_msg(wifi_cmds[0],1);
	cmd_state = C_IDLE;
	port1_state = P_FLUSH;
	break;
      default:
	break;
      }

      /* wifi module terminal output */
      if(buffer2.head != buffer2.echo)
	{
	  while(Busy1USART());
	  Write1USART(buffer2.mem[buffer2.echo++]);
	}

      /* wifi port state machine */
      switch(port2_state) {
      case(P_IDLE):
      	if(buffer2.flag)
	  {
	  port2_state = P_READY;
	  }
	break;
      case(P_READY):
	if(msg_check(buffer2.mem,buffer2.tail,wmsg[0]))
	  {
	    wifi_ok = 1;
	  }
	else if(msg_check(buffer2.mem,buffer2.tail,wmsg[1]))
	  {
	    wifi_connected = 1;
	  }
	else if(msg_check(buffer2.mem,buffer2.tail,wmsg[2]))
	  {
	    key_offset = buffer2.tail;
	    key_offset += 35;
	    get_key(buffer2.mem, key, key_offset);
	  }
	buffer2.flag--;
	port2_state = P_FLUSH;
	break;
      case(P_TRANSMIT):
	break;
      case(P_FLUSH):
	while(buffer2.mem[buffer2.tail++] != '\n');
	port2_state = P_IDLE;
	break;
      default:
	break;
      }
    }
}
