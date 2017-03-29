#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "wifi.h"
#include "port.h"

uint8_t wifi_string[4];
uint8_t packet_length[4];
uint8_t content_length[3];
uint8_t data_string[6];

const uint8_t connection_mode[] = "\"TCP\",";
const uint8_t ip[] = "\"api.myjson.com\",";
const uint8_t port[] = "80";
const uint8_t wifi_cmds[][27] = {"\r\n",
				"AT\r\n",
				"AT+CWMODE=",
				"AT+RST\r\n",
				"AT+CWDHCP=",
				"AT+CWJAP=",
				"AT+CIPSTART=",
				"AT+CIPSEND=",
				"AT+CIPCLOSE\r\n"};
const uint8_t http_crlf[] = "\r\n";
const uint8_t http_requests[][6] = {"POST ",
				    "GET ",
				    "PUT "};
const uint8_t http_protocol[] = " HTTP/1.1";
const uint8_t http_headers[][17] = {"HOST: ",
				   "CONTENT-TYPE: ",
				   "CONTENT-LENGTH: ",
				   "CONNECTION: "};
const uint8_t http_hosts[][15] = {"api.myjson.com"};
const uint8_t http_content_types[][17] = {"application/json"};
const uint8_t http_connections[][11] = {"close",
				       "keep-alive"};
const uint8_t http_urls[][6] = {"/bins"};
const uint8_t http_body[] = "{\"0\":\"0\"}";
const uint8_t http_body_strings[][10] = {"{\"time\":\"",
					 "\"}"};
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
  send_msg((const uint8_t *)",",2);
  hex2dec(wifi_string, enable);
  send_msg_ram(wifi_string,2);
  send_msg(wifi_cmds[0],2);
}

void wifi_cwjap(uint8_t * ssid, uint8_t * pass)
{
  send_msg(wifi_cmds[5],2);
  send_msg((const uint8_t *)"\"",2);
  send_msg_ram(ssid,2);	
  send_msg((const uint8_t *)"\",\"",2);
  send_msg_ram(pass,2);
  send_msg((const uint8_t *)"\"",2);
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
  
void wifi_cipsend(void)
/* void wifi_cipsend(uint8_t n) */
{
  send_msg(wifi_cmds[7],2);
  send_msg_ram(packet_length,2);
  send_msg(wifi_cmds[0],2);
}

void wifi_close(void)
{
  send_msg(wifi_cmds[8],2);
}

void http_calc(Http_t * http)
{
  /* calc content-length */
  http->content_length = 0;

  if(http->request == 0)
    {
      http->content_length = strlen(http_body);
    }
  else if(http->request == 2)
    {
      http->content_length += strlen(http_body_strings[0]);
      hex2dec16(data_string,http->data);
      http->content_length += strlen(data_string);
      http->content_length += strlen(http_body_strings[1]);
    }
  /* update string */
  hex2dec(content_length,http->content_length);
  
  /* calc packet-length */
  http->packet_length = 0;

  /* request line */
  if(http->request == 0)
    {
      http->packet_length += strlen(http_requests[0]);
      http->packet_length += strlen(http_urls[0]);
      http->packet_length += strlen(http_protocol);
      http->packet_length += strlen(http_crlf);
    }
  else if(http->request == 2)
    {
      http->packet_length += strlen(http_requests[2]);
      http->packet_length += strlen(http_urls[0]);
      http->packet_length += strlen((const uint8_t *)"/");
      http->packet_length += strlen(http->key);
      http->packet_length += strlen(http_protocol);
      http->packet_length += strlen(http_crlf);
    }

  /* host */
  http->packet_length += strlen(http_headers[0]);
  http->packet_length += strlen(http_hosts[0]);
  http->packet_length += strlen(http_crlf);

  /* content-type */
  http->packet_length += strlen(http_headers[1]);
  http->packet_length += strlen(http_content_types[0]);
  http->packet_length += strlen(http_crlf);

  /* content-length */
  http->packet_length += strlen(http_headers[2]);
  http->packet_length += strlen(content_length);
  http->packet_length += strlen(http_crlf);

  /* connection */
  http->packet_length += strlen(http_headers[3]);
  http->packet_length += strlen(http_connections[0]);
  http->packet_length += strlen(http_crlf);
		  
  /* http_crlf */
  http->packet_length += strlen(http_crlf);
		  
  /* body */
  if(http->request == 0)
    {
      http->packet_length += strlen(http_body);
    }
  else if(http->request == 2)
    {
      http->packet_length += strlen(http_body_strings[0]);
      http->packet_length += strlen(data_string);
      http->packet_length += strlen(http_body_strings[1]);
    }
  http->packet_length += strlen(http_crlf);

  /* http_crlf */
  http->packet_length += strlen(http_crlf);
  http->packet_length += strlen(http_crlf);
  http->packet_length += strlen(http_crlf);

  /* store in string */
  hex2dec(packet_length,http->packet_length);
}

void http_send_packet(Http_t * http)
{
  /* request line */
  if(http->request == 0)
    {
      send_msg(http_requests[0],2);
      send_msg(http_urls[0],2);
      send_msg(http_protocol,2);
      send_msg(http_crlf,2);
    }
  else if(http->request == 2)
    {
      send_msg(http_requests[2],2);
      send_msg(http_urls[0],2);
      send_msg((const uint8_t *)"/",2);
      send_msg_ram(http->key,2);
      send_msg(http_protocol,2);
      send_msg(http_crlf,2);
    }

  /* host */
  send_msg(http_headers[0],2);
  send_msg(http_hosts[0],2);
  send_msg(http_crlf,2);

  /* content-type */
  send_msg(http_headers[1],2);
  send_msg(http_content_types[0],2);
  send_msg(http_crlf,2);

  /* content-length */
  send_msg(http_headers[2],2);
  send_msg_ram(content_length,2);
  send_msg(http_crlf,2);

  /* connection */
  send_msg(http_headers[3],2);
  send_msg(http_connections[0],2);
  send_msg(http_crlf,2);
		  
  /* http_crlf */
  send_msg(http_crlf,2);
		  
  /* body */
  if(http->request == 0)
    {
      send_msg(http_body,2);
    }
  else if(http->request == 2)
    {
      send_msg(http_body_strings[0],2);
      send_msg_ram(data_string,2);
      send_msg(http_body_strings[1],2);
    }
  send_msg(http_crlf,2);

  /* http_crlf */
  send_msg(http_crlf,2);
  send_msg(http_crlf,2);
  send_msg(http_crlf,2);

}


