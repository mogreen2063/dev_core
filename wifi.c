#include <string.h>
#include "wifi.h"
#include "port.h"

#pragma udata
 int8_t wifi_string[4];	/* "0" - "999" */
 int8_t packet_length[4];
 /* far int8_t wifi_string[4];	/\* "0" - "999" *\/ */
 /* far int8_t packet_length[4]; */

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

const far rom int8_t http_crlf[] = "\r\n";
const far rom int8_t http_requests[][6] = {"POST ",
					   "GET ",
					   "PUT "};
const far rom int8_t http_protocol[] = " HTTP/1.1";
const far rom int8_t http_headers[][17] = {"HOST: ",
					   "CONTENT-TYPE: ",
					   "CONTENT-LENGTH: ",
					   "CONNECTION: "};
const far rom int8_t http_hosts[][15] = {"api.myjson.com"};
const far rom int8_t http_content_types[][17] = {"application/json"};
const far rom int8_t http_connections[][11] = {"close",
					       "keep-alive"};
const far rom int8_t http_urls[][6] = {"/bins"};
const far rom int8_t http_body[] = "{\"0\":\"0\"}";

#pragma code
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

void http_get_packet_length(Http_t * http)
{
  int8_t packet_length[3];
  int8_t content_length[3];

  http->content_length = strlenpgm(http_body);
  http->packet_length = 0;

  /* request line */
  if(http->request == 0)
    {
      http->packet_length += strlenpgm(http_requests[0]);
      http->packet_length += strlenpgm(http_urls[0]);
      http->packet_length += strlenpgm(http_protocol);
      http->packet_length += strlenpgm(http_crlf);
    }
  else if(http->request == 2)
    {
      http->packet_length += strlenpgm(http_requests[2]);
      http->packet_length += strlenpgm(http_urls[0]);
      http->packet_length += strlenpgm((const far rom int8_t *)"/");
      http->packet_length += strlen(http->key);
      http->packet_length += strlenpgm(http_protocol);
      http->packet_length += strlenpgm(http_crlf);
    }

  /* host */
  http->packet_length += strlenpgm(http_headers[0]);
  http->packet_length += strlenpgm(http_hosts[0]);
  http->packet_length += strlenpgm(http_crlf);

  /* content-type */
  http->packet_length += strlenpgm(http_headers[1]);
  http->packet_length += strlenpgm(http_content_types[0]);
  http->packet_length += strlenpgm(http_crlf);

  /* content-length */
  http->packet_length += strlenpgm(http_headers[2]);

  hex2dec(content_length,http->content_length);

  http->packet_length += strlen(content_length);
  http->packet_length += strlenpgm(http_crlf);

  /* connection */
  http->packet_length += strlenpgm(http_headers[3]);
  http->packet_length += strlenpgm(http_connections[0]);
  http->packet_length += strlenpgm(http_crlf);
		  
  /* http_crlf */
  http->packet_length += strlenpgm(http_crlf);
		  
  /* body */
  http->packet_length += strlenpgm(http_body); /* change to strlen */
  http->packet_length += strlenpgm(http_crlf);

  /* http_crlf */
  http->packet_length += strlenpgm(http_crlf);
  http->packet_length += strlenpgm(http_crlf);
  http->packet_length += strlenpgm(http_crlf);
}

void http_send_packet(Http_t * http)
{
  int8_t content_length[3];

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
      send_msg((const far rom int8_t *)"/",2);
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

  hex2dec(content_length,http->content_length);

  send_msg_ram(content_length,2);
  send_msg(http_crlf,2);

  /* connection */
  send_msg(http_headers[3],2);
  send_msg(http_connections[0],2);
  send_msg(http_crlf,2);
		  
  /* http_crlf */
  send_msg(http_crlf,2);
		  
  /* body */
  send_msg(http_body,2);
  send_msg(http_crlf,2);

  /* http_crlf */
  send_msg(http_crlf,2);
  send_msg(http_crlf,2);
  send_msg(http_crlf,2);
}

