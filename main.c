#include <p18f24j50.h>
#include <usart.h>
#include <delays.h>
#include <string.h>
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
unsigned char uart1_buffer[BUFFER1_SIZE];

#pragma udata udata2=0x300
unsigned char uart2_buffer[BUFFER2_SIZE];

#pragma udata
Buffer_t buffer1;
Buffer_t buffer2;
far char key[6];
far char time_string[5];
far unsigned int time;

#pragma romdata
const far rom char cmsg[][8] = {"setup",
				"connect",
				"upload"};
const far rom char ssid[] = "\"164auburn-2.4\",";
const far rom char pass[] = "\"novultures2017\"";
const far rom char connection_mode[] = "\"TCP\",";
const far rom char ip[] = "\"api.myjson.com\",";
const far rom char port[] = "80";

const far rom char wmsg[][36] = {"OK\r\n",
				 "WIFI GOT IP\r\n",
				 "{\"uri\":\"http://api.myjson.com/bins/"};

const far rom char wifi_cmds[][16] = {"\r\n",
				      "AT\r\n",
				      "AT+CWMODE=1\r\n",
				      "AT+RST\r\n",
				      "AT+CWDHCP=1,1\r\n",
				      "AT+CWJAP=",
				      "AT+CIPSTART=",
				      "AT+CIPSEND=",
				      "AT+CIPCLOSE\r\n"};

const far rom char http_header[][33] = {"HOST: api.myjson.com",
					"CONTENT-TYPE: application/json",
					"CONTENT-LENGTH: ",
					"CONNECTION: close"};

const far rom char http_post[] = "POST /bins HTTP/1.1";
const far rom char post_packet_length[]  = "132";
const far rom char post_content_length[] = "9";
const far rom char post_content[] = "{\"0\":\"0\"}";

const far rom char http_put[][11] = {"PUT /bins/",
				     " HTTP/1.1"};
const far rom char put_packet_length[]   = "144";
const far rom char put_content_length[]  = "15";
const far rom char put_content[][8] = {"{\"",
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
  
  if (PIR3bits.RC2IF) {
      buffer2.mem[buffer2.head] = Read2USART();
      if(buffer2.mem[buffer2.head++] == '\n') {
	  buffer2.flag++;
	}
    }
}
				      
void get_time(far char * time_string)
{
  char digit;
  
  digit  = (char)((time & 0xf000) >> 12);
  digit += '0';
  if (digit > '9')
    {
      digit +=0x7;
    }
  time_string[0] = digit;
  
  digit  = (char)((time & 0x0f00) >> 8);
  digit += '0';
  if (digit > '9')
    {
      digit +=0x7;
    }
  time_string[1] = digit;

  digit  = (char)((time & 0x00f0) >> 4);
  digit += '0';
  if (digit > '9')
    {
      digit +=0x7;
    }
  time_string[2] = digit;

  digit  = (char)(time & 0x000f);
  digit += '0';
  if (digit > '9')
    {
      digit +=0x7;
    }
  time_string[3] = digit;
}

void send_msg_ram(far char * msg, unsigned char dest)
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

void send_msg(const far rom char * msg, unsigned char dest)
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

unsigned char msg_check(unsigned char * mem, unsigned char index,
			const far rom char * msg)
{
  unsigned char n = strlenpgm(msg);
  
  while(n--)
    {
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

  unsigned char wifi_ok;
  unsigned char wifi_connected;
  unsigned char wifi_ip;
  unsigned char wifi_send_ready;
  
  unsigned char cmd_busy = 0;
  unsigned char key_offset = 0;
  unsigned char connect_init = 1;

  unsigned int time_data = 0;

  char digit;

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
  key[0] = '\0';
  key[1] = '\0';
  key[2] = '\0';
  key[3] = '\0';
  key[4] = '\0';
  key[5] = '\0';

  time_string[0] = '0';
  time_string[1] = '0';
  time_string[2] = '0';
  time_string[3] = '0';
  time_string[4] = '\0';

  time = 0;
  while(1)
    {
      time++;
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
	if(msg_check(buffer1.mem,buffer1.tail,cmsg[0]))
	  {
	    if(!cmd_busy)
	      {
		cmd_state = C_SETUP;
		port1_state = P_FLUSH;
	      }
	  }
	else if(msg_check(buffer1.mem,buffer1.tail,cmsg[1]))
	  {
	    if(!cmd_busy)
	      {
		cmd_state = C_CONNECT;
		port1_state = P_FLUSH;
	      }
	  }
	else if(msg_check(buffer1.mem,buffer1.tail,cmsg[2]))
	  {
	    if(!cmd_busy)
	      {
		connect_init = 0;
		cmd_state = C_CONNECT;
		port1_state = P_FLUSH;
	      }
	  }
	else
	  {
	    port1_state = P_TRANSMIT;
	  }
	break;
      case(P_TRANSMIT):
	while(buffer1.mem[buffer1.tail] != '\n')
	  {
	    while(Busy2USART());
	    Write2USART(buffer1.mem[buffer1.tail++]);
	  }
	while(Busy2USART());
	Write2USART(buffer1.mem[buffer1.tail++]);
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
	      send_msg(ssid,2);
	      send_msg(pass,2);
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
		      send_msg(put_packet_length,2);
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

		    send_msg(put_content[0],1);
		    send_msg(put_content[1],1);
		    send_msg_ram(time_string,1);
		    send_msg(put_content[2],1);
		  }
		    
		  send_msg(wifi_cmds[0],2);
		  send_msg(wifi_cmds[0],2);
		  send_msg(wifi_cmds[0],2);
		  send_msg(wifi_cmds[0],2);

		  send_msg(wifi_cmds[0],1);
		  send_msg(wifi_cmds[0],1);
		  send_msg(wifi_cmds[0],1);
		  send_msg(wifi_cmds[0],1);
		  
		  connect_init = 0;
		  wifi_send_ready = 0;
		  connect_state = CONNECT_EXIT;
		}
	      break;
	    case(CONNECT_CLOSE):
	      send_msg(wifi_cmds[8],2);
	      connect_state = CONNECT_EXIT;
	      break;
	    case(CONNECT_EXIT):
	      cmd_busy = 0;
	      connect_state = CONNECT_AT;
	      cmd_state = C_IDLE;
	      break;
	    default:
	      break;
	    }
	  }
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

	    key[0] = buffer2.mem[key_offset++];
	    key[1] = buffer2.mem[key_offset++];
	    key[2] = buffer2.mem[key_offset++];
	    key[3] = buffer2.mem[key_offset++];
	    key[4] = buffer2.mem[key_offset++];
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
