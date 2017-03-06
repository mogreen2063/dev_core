#include "stdint.h"

/* defines */
#define BUFFER1_SIZE 256
#define BUFFER2_SIZE 256

typedef enum {
  P_IDLE,
  P_READY,
  P_TRANSMIT,
  P_FLUSH
} PortStates;

typedef enum {
  W_IDLE,
  W_READY,
  W_OK,
  W_TRANSMIT,
  W_CONNECT,
  W_RECEIVE
} WifiStates;

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
void sendstr();
uint8_t buffer2_comp(uint8_t index, const far rom int8_t * msg);
uint8_t msg_check(int8_t * mem, uint8_t index,
		  const far rom int8_t * msg);
void send_msg(const far rom int8_t * msg, uint8_t dest);
void send_msg_ram(far int8_t * msg, uint8_t dest);
void send_char(const far rom int8_t * c, uint8_t dest);
void send_char_ram(int8_t * c, uint8_t dest);
void copy_msg(int8_t * mem, uint8_t index, far int8_t * msg);
void get_key(int8_t * mem, int8_t * key, uint8_t key_offset);
