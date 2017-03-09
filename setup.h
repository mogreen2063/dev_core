#include "stdint.h"
#include "http.h"

/* defines */
#define BUFFER1_SIZE 256
#define BUFFER2_SIZE 256

typedef struct {
  PortStates state;
  Buffer_t * buffer;
} Port_t;

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
  int8_t * url;
} Wifi_t;

typedef enum {
  SETUP_AT,
  SETUP_CWMODE,
  SETUP_RST,
  SETUP_CWDHCP,
  SETUP_CWJAP
} SetupStates;

typedef enum {
  CONNECT_AT,
  CONNECT_CIPSTART,
  CONNECT_CIPSEND,
  CONNECT_CIPSEND_DATA,
  CONNECT_CLOSE,
  CONNECT_EXIT
} ConnectStates;

typedef enum {
  PULSE_INIT,
  PULSE_ON,
  PULSE_OFF
} PulseStates;

typedef struct {
  volatile uint8_t flag;
  volatile uint8_t head;
  uint8_t tail;
  uint8_t echo;
  int8_t * mem;
} Buffer_t;

#pragma code
void setup(void);
void InterruptHandlerHigh(void);
void copy_msg(int8_t * mem, uint8_t index, far int8_t * msg);

void send_msg(const far rom int8_t * msg, uint8_t dest);
void send_msg_ram(far int8_t * msg, uint8_t dest);
void send_char(const far rom int8_t * c, uint8_t dest);
void send_char_ram(int8_t * c, uint8_t dest);
void hex2ascii(int8_t * ascii_string, uint8_t n);
void hex2dec(int8_t * ascii_string, uint8_t n);
uint8_t msg_check(int8_t * mem, uint8_t index,
		  const far rom int8_t * msg);

void handle_port1(Buffer_t * buffer1, PortStates * port1_state, CmdStates * cmd_state, Wifi_t * wifi);
void handle_port2(Buffer_t * buffer2, PortStates * port2_state, Wifi_t * wifi, Http_t * http);
void get_key(int8_t * mem, int8_t * key, uint8_t key_offset);
