#include <usart.h>
#include <string.h>
#include "port.h"

#pragma udata
 SetupStates setup_state;
 ConnectStates connect_state;

#pragma romdata
const far rom int8_t cmsg[][8] = {"setup",
				  "connect",
				  "upload",
				  "ssid",
				  "pass",
				  "url"};

const far rom int8_t wmsg[][36] = {"OK\r\n",
				   "WIFI GOT IP\r\n",
				   "{\"uri\":\"http://api.myjson.com/bins/"};
const far rom int8_t url[] = "http://api.myjson.com/bins/";

#pragma code
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
      if(mem[index++] != *msg++)
	{ return 0; }
    }
  return 1;
}

void hex2ascii(int8_t * ascii_string, uint8_t n)
{
  *ascii_string = n;
  *ascii_string += '0';
  if(*ascii_string > '9')
    {
      *ascii_string += 7;
    }
}

void hex2dec(int8_t * ascii_string, uint8_t n)
{
  uint8_t count;
  uint8_t first = 0;
  uint8_t mult[] = {1,10,100};
  uint8_t index = 2;

  if(n)
    {
      do {
	count = 0;
	while(n >= mult[index])
	  {
	    n -= mult[index];
	    count++;
	    first = 1;
	  }
	if(first)
	  {
	    hex2ascii(ascii_string,count);
	    ascii_string++;
	  }
      } while(index--);
      *ascii_string = '\0';
    }
  else
    {
      *ascii_string++ = '0';
      *ascii_string = '\0';
    }
}

void handle_port1(Port_t * port, Wifi_t * wifi)
{
  /* computer port echo */
  if(port->buffer->head != port->buffer->echo)
    {
      send_char_ram(&port->buffer->mem[port->buffer->echo++],1);
    }

  /* computer port state machine */
  switch(port->state) {
  case(P_IDLE):
    if(port->buffer->flag)
      {
  	port->state = P_READY;
      }
    break;
  case(P_READY):
    if(port->cmd_state == C_IDLE)
      {
  	if(msg_check(port->buffer->mem,port->buffer->tail,cmsg[0]))
  	  {
	    setup_state = SETUP_AT;
  	    port->cmd_state = C_SETUP;
  	    port->state = P_FLUSH;
  	  }
  	else if(msg_check(port->buffer->mem,port->buffer->tail,cmsg[1]))
  	  {
  	    wifi->mode = 0;
	    connect_state = CONNECT_AT;
  	    port->cmd_state = C_CONNECT;
  	    port->state = P_FLUSH;
  	  }
  	else if(msg_check(port->buffer->mem,port->buffer->tail,cmsg[2]))
  	  {
  	    wifi->mode = 2;
  	    port->cmd_state = C_CONNECT;
	    port->state = P_FLUSH;
  	  }
  	else if(msg_check(port->buffer->mem,port->buffer->tail,cmsg[3]))
  	  {
  	    port->cmd_state = C_SSID;
  	  }
  	else if(msg_check(port->buffer->mem,port->buffer->tail,cmsg[4]))
  	  {
  	    port->cmd_state = C_PASS;
  	  }
  	else if(msg_check(port->buffer->mem,port->buffer->tail,cmsg[5]))
  	  {
  	    port->cmd_state = C_URL;
  	    port->state = P_FLUSH;
  	  }
  	else
  	  {
  	    port->state = P_TRANSMIT;
  	  }
      }
    break;
  case(P_TRANSMIT):
    do {
      send_char_ram(&port->buffer->mem[port->buffer->tail],2);
    } while(port->buffer->mem[port->buffer->tail++] != '\n');
    port->state = P_IDLE;
    port->buffer->flag--;
    break;
  case(P_FLUSH):
    while(port->buffer->mem[port->buffer->tail++] != '\n');
    port->state = P_IDLE;
    port->buffer->flag--;
    break;
  default:
    break;
  }

  /* handle command */
  switch(port->cmd_state) {
  case(C_IDLE):
    break;
  case(C_MON):
    break;
  case(C_SETUP):
    switch(setup_state) {
    case(SETUP_AT):
      wifi->ok = 0;
      wifi_status();
      setup_state = SETUP_CWMODE;
      break;
    case(SETUP_CWMODE):
      if(wifi->ok)
  	{
  	  wifi->ok = 0;
  	  wifi_cwmode(WIFI_CWMODE);
  	  setup_state = SETUP_RST;
  	}
      break;
    case(SETUP_RST):
      if(wifi->ok)
  	{
  	  wifi->ok = 0;
  	  wifi_rst();
  	  setup_state = SETUP_CWDHCP;
  	}
      break;
    case(SETUP_CWDHCP):
      if(wifi->ok)
  	{
  	  wifi->ok = 0;
  	  wifi->state = W_CONNECTED;
  	  wifi_cwdhcp(WIFI_DHCP_MODE,WIFI_DHCP_ENABLE);
  	  setup_state = SETUP_CWJAP;
  	}
      break;
    case(SETUP_CWJAP):
      if(wifi->ok)
  	{
  	  wifi->ok = 0;
  	  wifi_cwjap(wifi->ssid,wifi->pass);
  	  setup_state = SETUP_AT;
  	  port->cmd_state = C_IDLE;
  	}
      break;
    default:
      break;
    }
    break;
  case(C_CONNECT):
    if(wifi->state == W_CONNECTED)
      {
  	switch(connect_state) {
  	case(CONNECT_AT):
  	  wifi->ok = 0;
  	  wifi_status();
  	  connect_state = CONNECT_CIPSTART;
  	  break;
  	case(CONNECT_CIPSTART):
  	  if(wifi->ok)
  	    {
  	      wifi->ok = 0;
  	      wifi_cipstart();
  	      connect_state = CONNECT_CIPSEND;
  	    }
  	  break;
  	case(CONNECT_CIPSEND):
  	  if(wifi->ok)
  	    {
  	      wifi->ok = 0;
  	      wifi->http->request = wifi->mode;
  	      http_get_packet_length(wifi->http);
  	      wifi_cipsend(wifi->http->packet_length);
  	      connect_state = CONNECT_CIPSEND_DATA;
  	    }
  	  break;
  	case(CONNECT_CIPSEND_DATA):
  	  if(wifi->ok)
  	    {
  	      http_send_packet(wifi->http);
  	      connect_state = CONNECT_EXIT;
  	    }
  	  break;
  	case(CONNECT_CLOSE):
  	  wifi_close();
  	  connect_state = CONNECT_EXIT;
  	  break;
  	case(CONNECT_EXIT):
  	  connect_state = CONNECT_AT;
  	  port->cmd_state = C_IDLE;
  	  break;
  	default:
  	  break;
  	}
      }
    break;
  case(C_SSID):
    copy_msg(port->buffer->mem, port->buffer->tail += 5, wifi->ssid);
    port->cmd_state = C_IDLE;
    port->state = P_FLUSH;
    break;
  case(C_PASS):
    copy_msg(port->buffer->mem, port->buffer->tail += 5, wifi->pass);
    port->cmd_state = C_IDLE;
    port->state = P_FLUSH;
    break;
  case(C_URL):
    send_msg(url,1);
    send_msg_ram(wifi->http->key,1);
    send_msg((const far rom int8_t *)"\r\n",1);
    port->cmd_state = C_IDLE;
    port->state = P_FLUSH;
    break;
  default:
    break;
  }
}

void handle_port2(Port_t * port, Wifi_t * wifi)
{
  /* wifi module terminal output */
  if(port->buffer->head != port->buffer->echo)
    {
      send_char_ram(&port->buffer->mem[port->buffer->echo++],1);
    }

  /* wifi port state machine */
  switch(port->state) {
  case(P_IDLE):
    if(port->buffer->flag)
      {
    	port->state = P_READY;
      }
    break;
  case(P_READY):
    if(msg_check(port->buffer->mem,port->buffer->tail,wmsg[0]))
      {
    	wifi->ok = 1;
      }
    else if(msg_check(port->buffer->mem,port->buffer->tail,wmsg[1]))
      {
    	wifi->state = W_CONNECTED;
      }
    else if(msg_check(port->buffer->mem,port->buffer->tail,wmsg[2]))
      {
    	get_key(port->buffer->mem, wifi->http->key, port->buffer->tail + 35);
      }
    port->state = P_FLUSH;
    break;
  case(P_TRANSMIT):
    break;
  case(P_FLUSH):
    while(port->buffer->mem[port->buffer->tail++] != '\n');
    port->state = P_IDLE;
    port->buffer->flag--;
    break;
  default:
    break;
  }
}

void get_key(int8_t * mem, int8_t * key, uint8_t key_offset)
{
  do {
    *key++ = mem[key_offset++];
  } while(mem[key_offset] != '\"');
  *key = '\0';
}

void copy_msg(int8_t * mem, uint8_t index, far int8_t * msg)
{
  do {
    *msg++ = mem[index++];
  } while(mem[index] != '\r');
  *msg = '\0';
}
