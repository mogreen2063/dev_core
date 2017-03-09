#include "setup.h"

#pragma romdata
const far rom int8_t cmsg[][8] = {"setup",
				  "connect",
				  "upload",
				  "ssid",
				  "pass",
				  "url"};

#pragma code
void handle_port1(Buffer_t * buffer1, PortStates * port1_state, CmdStates * cmd_state, Wifi_t * wifi)
{
  /* computer port echo */
  if(buffer1->head != buffer1->echo)
    {
      send_char_ram(&buffer1->mem[buffer1->echo++],1);
    }

  /* computer port state machine */
  switch(*port1_state) {
  case(P_IDLE):
    if(buffer1->flag)
      {
	*port1_state = P_READY;
      }
    break;
  case(P_READY):
    if(*cmd_state == C_IDLE)
      {
	if(msg_check(buffer1->mem,buffer1->tail,cmsg[0]))
	  {
	    *cmd_state = C_SETUP;
	    *port1_state = P_FLUSH;
	  }
	else if(msg_check(buffer1->mem,buffer1->tail,cmsg[1]))
	  {
	    wifi->mode = 0;
	    send_msg((const far rom int8_t *)"here",1);
	    *cmd_state = C_CONNECT;
	    *port1_state = P_FLUSH;
	  }
	else if(msg_check(buffer1->mem,buffer1->tail,cmsg[2]))
	  {
	    wifi->mode = 2;
	    *cmd_state = C_CONNECT;
	    *port1_state = P_FLUSH;
	  }
	else if(msg_check(buffer1->mem,buffer1->tail,cmsg[3]))
	  {
	    *cmd_state = C_SSID;
	    /* *port1_state = P_FLUSH; */
	  }
	else if(msg_check(buffer1->mem,buffer1->tail,cmsg[4]))
	  {
	    *cmd_state = C_PASS;	
	    /* *port1_state = P_FLUSH; */
	  }
	else if(msg_check(buffer1->mem,buffer1->tail,cmsg[5]))
	  {
	    *cmd_state = C_URL;
	    *port1_state = P_FLUSH;
	  }
	else
	  {
	    *port1_state = P_TRANSMIT;
	  }
      }
    break;
  case(P_TRANSMIT):
    do {
      send_char_ram(&buffer1->mem[buffer1->tail],2);
    } while(buffer1->mem[buffer1->tail++] != '\n');
    *port1_state = P_IDLE;
    buffer1->flag--;
    break;
  case(P_FLUSH):
    while(buffer1->mem[buffer1->tail++] != '\n');
    *port1_state = P_IDLE;
    buffer1->flag--;
    break;
  default:
    break;
  }
}
