#ifndef _WIFI_H
#define _WIFI_H

#include <xc.h>
#include <stdint.h>

#define WIFI_CWMODE 1
#define WIFI_DHCP_ENABLE 1
#define WIFI_DHCP_MODE 1

typedef struct {
  uint8_t request;
  uint8_t packet_length;	/* !! can only send 256 bytes */
  uint8_t content_length;
  uint16_t data;
  uint8_t * key;
} Http_t;

typedef enum {
  W_IDLE,
  W_CONNECTED,
  W_OPEN
} WifiStates;

typedef struct {
  WifiStates state;
  uint8_t ok;
  uint8_t ready;
  uint8_t mode;
  uint8_t init;
  uint8_t closed;
  uint8_t * ssid;
  uint8_t * pass;
} Wifi_t;

void wifi_post(void);
void wifi_status(void);
void wifi_cwmode(uint8_t mode);
void wifi_rst(void);
void wifi_cwdhcp(uint8_t mode, uint8_t enable);
void wifi_cwjap(uint8_t * ssid, uint8_t * pass);
void wifi_cipstart(void);
void wifi_cipsend(void);
/* void wifi_cipsend(uint8_t n); */
void wifi_close(void);

void http_calc(Http_t * http);
void http_send_packet(Http_t * http);

#endif	/* _WIFI_H */
