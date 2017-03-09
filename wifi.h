#include "stdint.h"

#define WIFI_CWMODE 1
#define WIFI_DHCP_ENABLE 1
#define WIFI_DHCP_MODE 1

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
