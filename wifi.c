#include "wifi.h"
#include "setup.h"

#pragma udata
far int8_t wifi_string[4];	/* "0" - "999" */
far int8_t packet_length[4];

#pragma romdata
const far rom int8_t connection_mode[] = "\"TCP\",";
const far rom int8_t ip[] = "\"api.myjson.com\",";
const far rom int8_t port[] = "80";
const far rom int8_t wifi_cmds[][16] = {"\r\n",
					"AT\r\n",
					"AT+CWMODE=",
					"AT+RST\r\n",
					"AT+CWDHCP=",
					"AT+CWJAP=",
					"AT+CIPSTART=",
					"AT+CIPSEND=",
					"AT+CIPCLOSE\r\n"};

void wifi_status(void)
{
  send_msg(wifi_cmds[1],2);
}

void wifi_cwmode(uint8_t mode)
{
  send_msg(wifi_cmds[2],2);
  hex2dec(wifi_string, mode);
  send_msg_ram(wifi_string,2);
  send_msg(wifi_cmds[0],2);
}
  
void wifi_rst(void)
{
  send_msg(wifi_cmds[3],2);
}

void wifi_cwdhcp(uint8_t mode, uint8_t enable)
{
  send_msg(wifi_cmds[4],2);
  hex2dec(wifi_string, mode);
  send_msg_ram(wifi_string,2);
  send_msg((const far rom int8_t *)",",2);
  hex2dec(wifi_string, enable);
  send_msg_ram(wifi_string,2);
  send_msg(wifi_cmds[0],2);
}

void wifi_cwjap(int8_t * ssid, int8_t * pass)
{
  send_msg(wifi_cmds[5],2);
  send_msg((const far rom int8_t *)"\"",2);
  send_msg_ram(ssid,2);	
  send_msg((const far rom int8_t *)"\",\"",2);
  send_msg_ram(pass,2);
  send_msg((const far rom int8_t *)"\"",2);
  send_msg(wifi_cmds[0],2);
}
 
void wifi_cipstart(void)
{
  send_msg(wifi_cmds[6],2);
  send_msg(connection_mode,2);
  send_msg(ip,2);
  send_msg(port,2);
  send_msg(wifi_cmds[0],2);
} 
  
void wifi_cipsend(uint8_t n)
{
  hex2dec(packet_length,n);

  send_msg(wifi_cmds[7],2);
  send_msg_ram(packet_length,2);
  send_msg(wifi_cmds[0],2);
}

void wifi_close(void)
{
  send_msg(wifi_cmds[8],2);
}

