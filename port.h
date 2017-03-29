#ifndef _PORT_H
#define _PORT_H

#include <xc.h>
#include <stdint.h>
#include "wifi.h"


typedef enum {
  P_IDLE,
  P_READY,
  P_TRANSMIT,
  P_FLUSH
} PortStates;

typedef enum {
  C_IDLE,
  C_MON,
  C_SETUP,
  C_CONNECT,
  C_SSID,
  C_PASS,
  C_URL
} CmdStates;

typedef struct {
  PortStates p_state;
  CmdStates  c_state;
} Port_t;

typedef struct {
  volatile uint8_t * mem;
  volatile uint8_t flag;
  volatile uint8_t head;
  uint8_t tail;
  uint8_t echo;
} Buffer_t;


typedef enum {
  SETUP_AT,
  SETUP_CWMODE,
  SETUP_RST,
  SETUP_CWDHCP,
  SETUP_CWJAP,
  SETUP_EXIT
} SetupStates;

typedef enum {
  CONNECT_AT,
  CONNECT_CIPSTART,
  CONNECT_CIPSEND,
  CONNECT_CIPSEND_DATA,
  CONNECT_CLOSE,
  CONNECT_EXIT
} ConnectStates;

void send_msg(const uint8_t * msg, uint8_t dest);
void send_msg_ram(uint8_t * msg, uint8_t dest);
void send_char(const uint8_t * c, uint8_t dest);

void send_char_ram(volatile uint8_t * c, uint8_t dest);
void hex2ascii(uint8_t * ascii_string, uint8_t n);
void hex2dec(uint8_t * ascii_string, uint8_t n);
void hex2dec16(uint8_t * ascii_string, uint16_t n);

void copy_msg(volatile uint8_t * mem, uint8_t index, uint8_t * msg, const uint8_t * bk);
/* void copy_msg(volatile uint8_t * mem, uint8_t index, uint8_t * msg); */
uint8_t msg_check(volatile uint8_t * mem, uint8_t index, const uint8_t * msg);
void get_key(volatile uint8_t * mem, uint8_t * key, uint8_t key_offset);

void handle_port1(Port_t * port, Buffer_t * buffer, Wifi_t * wifi, Http_t * http);
void handle_port2(Port_t * port, Buffer_t * buffer, Wifi_t * wifi, Http_t * http);
void handle_cmd(Port_t * port, Buffer_t * buffer, Wifi_t * wifi, Http_t * http);
uint8_t handle_cmd_setup(Port_t * port, Wifi_t * wifi);
uint8_t handle_cmd_connect(Port_t * port, Wifi_t * wifi, Http_t * http, ConnectStates * connect_state);

#endif	/* _PORT_H */
