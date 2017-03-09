#include "setup.h"
#include "wifi.h"

#pragma idata
SetupStates setup_state = SETUP_AT;
ConnectStates connect_state = CONNECT_AT;

#pragma udata
void handle_cmd(Buffer_t * buffer1, PortStates * port1_state, CmdStates * cmd_state, Wifi_t * wifi)
{
  switch(*cmd_state) {
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
	  wifi->state = 0;
	  wifi_cwmode(WIFI_CWMODE);
	  setup_state = SETUP_RST;
	}
      break;
    case(SETUP_RST):
      if(wifi->ok)
	{
	  wifi->state = 0;
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
	  *cmd_state = C_IDLE;
	}
      break;
    default:
      break;
    }
    break;
  case(C_CONNECT):
    if(wifi->state = W_CONNECTED)
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
	      wifi_cipsend(http_get_packet_length(wifi->mode));
	      connect_state = CONNECT_CIPSEND_DATA;
	    }
	  break;
	case(CONNECT_CIPSEND_DATA):
	  if(wifi->ok)
	    {
	      http_send_packet(wifi->mode);
	      connect_state = CONNECT_EXIT;
	    }
	  break;
	case(CONNECT_CLOSE):
	  wifi_close();
	  connect_state = CONNECT_EXIT;
	  break;
	case(CONNECT_EXIT):
	  connect_state = CONNECT_AT;
	  *cmd_state = C_IDLE;
	  break;
	default:
	  break;
	}
      }
    break;
  case(C_SSID):
    copy_msg(buffer1->mem, buffer1->tail += 5, wifi->ssid);
    *cmd_state = C_IDLE;
    *port1_state = P_FLUSH;
    break;
  case(C_PASS):
    copy_msg(buffer1->mem, buffer1->tail += 5, wifi->pass);
    *cmd_state = C_IDLE;
    *port1_state = P_FLUSH;
    break;
  case(C_URL):
    send_msg_ram(wifi->url,1);
    send_msg_ram(wifi->http_key,1);
    send_msg((const far rom int8_t *)"\r\n",1);
    *cmd_state = C_IDLE;
    *port1_state = P_FLUSH;
    break;
  default:
    break;
  }
}
