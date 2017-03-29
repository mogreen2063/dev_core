#include <xc.h>
#include <stdint.h>
#include "port.h"
#include "wifi.h"
#include "setup.h"

SetupStates setup_state = SETUP_AT;
ConnectStates connect_state = CONNECT_AT;

uint8_t temp_string[3];
const uint8_t cmsg[][8] = {"setup",
			  "connect",
			  "upload",
			  "ssid",
			  "pass",
			  "url"};

const uint8_t wmsg[][36] = {"OK\r\n",
			   "WIFI GOT IP\r\n",
			   "{\"uri\":\"http://api.myjson.com/bins/",
			    "ready\r\n",
			    "CLOSED\r\n"};
const uint8_t url[] = "http://api.myjson.com/bins/";


void send_char(const uint8_t * c, uint8_t dest)
{
  if(dest == 1)
    {
      while(!TXSTA1bits.TRMT);
      TXREG1 = *c;
    }
  else if(dest == 2)
    {
      while(!TXSTA2bits.TRMT);
      TXREG2 = *c;
    }
}

void send_char_ram(volatile uint8_t * c, uint8_t dest)
{
  if(dest == 1)
    {
      while(!TXSTA1bits.TRMT);
      TXREG1 = *c;
    }
  else if(dest == 2)
    {
      while(!TXSTA2bits.TRMT);
      TXREG2 = *c;
    }
}

void send_msg_ram(uint8_t * msg, uint8_t dest)
{
  if(dest == 1)
    {
      do {
	while(!TXSTA1bits.TRMT);
	TXREG1 = *msg;
      } while(*(++msg));
    }
  else if(dest == 2)
    {
      do {
	while(!TXSTA2bits.TRMT);
	TXREG2 = *msg;
      } while(*(++msg));
    }
}

void send_msg(const uint8_t * msg, uint8_t dest)
{
  if(dest == 1)
    {
      do {
	while(!TXSTA1bits.TRMT);
	TXREG1 = *msg;
      } while(*(++msg));
    }
  else if(dest == 2)
    {
      do {
	while(!TXSTA2bits.TRMT);
	TXREG2 = *msg;
      } while(*(++msg));
    }
}

uint8_t msg_check(volatile uint8_t * mem, uint8_t index,
		  const uint8_t * msg)
{
  do {
    if(mem[index++] != *msg++)
      {
	return 0;
      }
  } while(*msg);

  return 1;
}

void hex2ascii(uint8_t * ascii_string, uint8_t n)
{
  *ascii_string = n;
  *ascii_string += '0';
  if(*ascii_string > '9')
    {
      *ascii_string += 7;
    }
}

void hex2dec(uint8_t * ascii_string, uint8_t n)
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

void hex2dec16(uint8_t * ascii_string, uint16_t n)
{
  uint8_t count;
  uint8_t first = 0;
  uint16_t mult[] = {1,10,100,1000,10000};
  uint8_t index = 4;

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

void handle_port1(Port_t * port, Buffer_t * buffer, Wifi_t * wifi, Http_t * http)
{
  /* computer port echo */
  if(buffer->head != buffer->echo)
    {
      send_char_ram(&buffer->mem[buffer->echo++],1);
    }

  /* computer port state machine */
  switch(port->p_state) {
  case(P_IDLE):
    if(buffer->flag)
      {
  	port->p_state = P_READY;
      }
    break;
  case(P_READY):
    if(port->c_state == C_IDLE)
      {
  	if(msg_check(buffer->mem,buffer->tail,cmsg[0]))
	  {
  	    setup_state = SETUP_AT;
  	    port->c_state = C_SETUP;
  	    port->p_state = P_FLUSH;
  	  }
  	else if(msg_check(buffer->mem,buffer->tail,cmsg[1]))
  	  {
  	    http->request = 0;
  	    connect_state = CONNECT_AT;
  	    port->c_state = C_CONNECT;
  	    port->p_state = P_FLUSH;
  	  }
  	else if(msg_check(buffer->mem,buffer->tail,cmsg[2]))
  	  {
  	    http->request = 2;
  	    connect_state = CONNECT_AT;
  	    port->c_state = C_CONNECT;
  	    port->p_state = P_FLUSH;
  	  }
  	else if(msg_check(buffer->mem,buffer->tail,cmsg[3]))
  	  {
  	    port->c_state = C_SSID;
  	  }
  	else if(msg_check(buffer->mem,buffer->tail,cmsg[4]))
  	  {
  	    port->c_state = C_PASS;
  	  }
  	else if(msg_check(buffer->mem,buffer->tail,cmsg[5]))
  	  {
  	    port->c_state = C_URL;
  	    port->p_state = P_FLUSH;
  	  }
  	else
  	  {
  	    port->p_state = P_TRANSMIT;
  	  }
      }
    break;
  case(P_TRANSMIT):
    do {
      send_char_ram(&buffer->mem[buffer->tail],2);
    } while(buffer->mem[buffer->tail++] != '\n');
    port->p_state = P_IDLE;
    buffer->flag--;
    break;
  case(P_FLUSH):
    while(buffer->mem[buffer->tail++] != '\n');
    port->p_state = P_IDLE;
    buffer->flag--;
    break;
  default:
    break;
  }
}

void handle_cmd(Port_t * port, Buffer_t * buffer, Wifi_t * wifi, Http_t * http)
{
  switch(port->c_state) {
  case(C_IDLE):
    setup_state = SETUP_AT;
    connect_state = CONNECT_AT;
    break;
  case(C_MON):
    break;
  case(C_SETUP):
    if(handle_cmd_setup(port,wifi))
      {
  	port->c_state = C_IDLE;
      }
    break;
  case(C_CONNECT):
    if(wifi->state == W_CONNECTED)
      {
	if(handle_cmd_connect(port,wifi,http,&connect_state))
	  {
	    wifi->init = 1;
	    port->c_state = C_IDLE;
	  }
      }
    else
      {
	handle_cmd_setup(port,wifi);
      }
    break;
  case(C_SSID):
    copy_msg(buffer->mem, buffer->tail += 5, wifi->ssid, (const uint8_t *)"\r");
    port->c_state = C_IDLE;
    port->p_state = P_FLUSH;
    break;
  case(C_PASS):
    copy_msg(buffer->mem, buffer->tail += 5, wifi->pass, (const uint8_t *)"\r");
    port->c_state = C_IDLE;
    port->p_state = P_FLUSH;
    break;
  case(C_URL):
    send_msg(url,1);
    send_msg_ram(http->key,1);
    send_msg((const uint8_t *)"\r\n",1);
    port->c_state = C_IDLE;
    port->p_state = P_FLUSH;
    break;
  default:
    break;
  }
}

void handle_port2(Port_t * port, Buffer_t * buffer, Wifi_t * wifi, Http_t * http)
{
  switch(port->p_state) {
  case(P_IDLE):
    if(buffer->flag)
      {
    	port->p_state = P_READY;
      }
    break;
  case(P_READY):
    if(msg_check(buffer->mem,buffer->tail,wmsg[0]))
      {
    	wifi->ok = 1;
      }
    else if(msg_check(buffer->mem,buffer->tail,wmsg[3]))
      {
    	wifi->ready = 1;
      }
    else if(msg_check(buffer->mem,buffer->tail,wmsg[4]))
      {
	wifi->closed = 1;
      }
    else if(msg_check(buffer->mem,buffer->tail,wmsg[1]))
      {
    	wifi->state = W_CONNECTED;
      }
    else if(msg_check(buffer->mem,buffer->tail,wmsg[2]))
      {
    	copy_msg(buffer->mem, buffer->tail + 35, http->key, (const uint8_t *)"\"");
      }
    port->p_state = P_FLUSH;
    break;
  case(P_TRANSMIT):
    break;
  case(P_FLUSH):
    do {
      send_char_ram(&buffer->mem[buffer->tail],1);
    } while(buffer->mem[buffer->tail++] != '\n');
    port->p_state = P_IDLE;
    buffer->flag--;
    break;
  default:
    break;
  }
}

uint8_t handle_cmd_setup(Port_t * port, Wifi_t * wifi)
{
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
	wifi->ready = 0;
  	wifi_rst();
  	setup_state = SETUP_CWDHCP;
      }
    break;
  case(SETUP_CWDHCP):
    if(wifi->ready)
      {
  	wifi->ok = 0;
  	wifi->state = W_IDLE;
  	wifi_cwdhcp(WIFI_DHCP_MODE,WIFI_DHCP_ENABLE);
  	setup_state = SETUP_CWJAP;
      }
    break;
  case(SETUP_CWJAP):
    if(wifi->ok)
      {
  	wifi->ok = 0;
  	wifi_cwjap(wifi->ssid,wifi->pass);
  	setup_state = SETUP_EXIT;
  	/* setup_state = SETUP_AT; */
  	/* port->c_state = C_IDLE; */
      }
    break;
  case(SETUP_EXIT):
    if(wifi->state == W_CONNECTED)
      {
	setup_state = SETUP_AT;
	return 1;
      }
  default:
    break;
  }
  return 0;
}

uint8_t handle_cmd_connect(Port_t * port, Wifi_t * wifi, Http_t * http, ConnectStates * connect_state)
{
  /* SetupStates setup_state = SETUP_IDLE; */
  /* if(wifi->state != W_CONNECTED) */
  /*   { */
  /*     handle_cmd_setup(port,wifi,http */

  /* if(wifi->state == W_CONNECTED) */
  /*   { */
  switch(*connect_state) {
  case(CONNECT_AT):
    wifi->ok = 0;
    wifi_status();
    *connect_state = CONNECT_CIPSTART;
    break;
  case(CONNECT_CIPSTART):
    if(wifi->ok)
      {
	wifi->ok = 0;
	wifi_cipstart();
	*connect_state = CONNECT_CIPSEND;
      }
    break;
  case(CONNECT_CIPSEND):
    if(wifi->ok)
      {
	wifi->ok = 0;
	http_calc(http);
	wifi_cipsend();
	*connect_state = CONNECT_CIPSEND_DATA;
      }
    break;
  case(CONNECT_CIPSEND_DATA):
    if(wifi->ok)
      {
	wifi->closed = 0;
	http_send_packet(http);
	*connect_state = CONNECT_EXIT;
      }
    break;
  case(CONNECT_CLOSE):
    wifi_close();
    *connect_state = CONNECT_EXIT;
    break;
  case(CONNECT_EXIT):
    if(wifi->closed)
      {
	/* send url */
	send_msg(url,1);
	send_msg_ram(http->key,1);
	send_msg((const uint8_t *)"\r\n",1);
	    
	*connect_state = CONNECT_AT;
	port->c_state = C_IDLE;
	return 1;
      }
    break;
  default:
    break;
  }
  return 0;
}

void copy_msg(volatile uint8_t * mem, uint8_t index, uint8_t * msg, const uint8_t * bk)
{
  do {
    *msg++ = mem[index++];
  } while(mem[index] != *bk);
  *msg = '\0';
}
