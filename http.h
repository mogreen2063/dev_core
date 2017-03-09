#ifndef _HTTP_H
#define _HTTP_H

#include "stdint.h"

typedef struct {
  uint8_t request;
  uint8_t packet_length;
  uint8_t content_length;
  int8_t * key;
} Http_t;

void http_get_packet_length(Http_t * http);
void http_send_packet(Http_t * http);

#endif	/* _HTTP_H */
