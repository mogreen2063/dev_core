#include "setup.h"

#pragma romdata
const far rom int8_t wmsg[][36] = {"OK\r\n",
				   "WIFI GOT IP\r\n",
				   "{\"uri\":\"http://api.myjson.com/bins/"};

void get_key(int8_t * mem, int8_t * key, uint8_t key_offset)
{
  do {
    *key++ = mem[key_offset++];
  } while(mem[key_offset] != '\"');
  *key = '\0';
}

void handle_port2(Buffer_t * buffer2, PortStates * port2_state, Wifi_t * wifi, int8_t * http_key)
{
  /* wifi module terminal output */
  if(buffer2->head != buffer2->echo)
    {
      send_char_ram(&buffer2->mem[buffer2->echo++],1);
    }

  /* wifi port state machine */
  switch(*port2_state) {
  case(P_IDLE):
    if(buffer2->flag)
      {
	*port2_state = P_READY;
      }
    break;
  case(P_READY):
    if(msg_check(buffer2->mem,buffer2->tail,wmsg[0]))
      {
	wifi->ok = 1;
      }
    else if(msg_check(buffer2->mem,buffer2->tail,wmsg[1]))
      {
	wifi->state = W_CONNECTED;
      }
    else if(msg_check(buffer2->mem,buffer2->tail,wmsg[2]))
      {
	get_key(buffer2->mem, http_key, buffer2->tail + 35);
      }
    buffer2->flag--;
    *port2_state = P_FLUSH;
    break;
  case(P_TRANSMIT):
    break;
  case(P_FLUSH):
    while(buffer2->mem[buffer2->tail++] != '\n');
    *port2_state = P_IDLE;
    break;
  default:
    break;
  }
}
