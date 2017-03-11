#ifndef _WIFI_H
#define _WIFI_H

#include "stdint.h"

#define WIFI_CWMODE 1
#define WIFI_DHCP_ENABLE 1
#define WIFI_DHCP_MODE 1

typedef struct {
  uint8_t request;
  uint8_t packet_length;
  uint8_t content_length;
  int8_t * key;
} Http_t;

typedef enum {
  W_IDLE,
  W_CONNECTED,
  W_OPEN
} WifiStates;

typedef struct {
  WifiStates state;
  uint8_t ok;
  uint8_t mode;
  int8_t * ssid;
  int8_t * pass;
  Http_t * http;
} Wifi_t;

void get_key(int8_t * mem, int8_t * key, uint8_t key_offset);
void wifi_post(void);
void wifi_status(void);
void wifi_cwmode(uint8_t mode);
void wifi_rst(void);
void wifi_cwdhcp(uint8_t mode, uint8_t enable);
void wifi_cwjap(int8_t * ssid, int8_t * pass);
void wifi_cipstart(void);
void wifi_cipsend(uint8_t n);
void wifi_close(void);

void http_get_packet_length(Http_t * http);
void http_send_packet(Http_t * http);

#endif	/* _WIFI_H */
