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
  C_PASS
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

typedef struct {
  volatile unsigned char flag;
  volatile unsigned char head;
  unsigned char tail;
  unsigned char echo;
  unsigned char * mem;
} Buffer_t;


#pragma code
void setup(void);
void InterruptHandlerHigh(void);
void sendstr();
unsigned char buffer2_comp(unsigned char index, const far rom char * msg);
unsigned char msg_check(unsigned char * mem, unsigned char index,
			const far rom char * msg);
void send_msg(const far rom char * msg, unsigned char dest);
void send_msg_ram(far char * msg, unsigned char dest);


/* const rom char *str_data1 = */
/*   "PUT /bins/"; */
/* const rom char *str_data2 = */
/*   " HTTP/1.1\r\n" */
/*    "HOST: api.myjson.com\r\n" */
/*   "content-type: application/json\r\n" */
/*   "content-length: 15\r\n" */
/*   "connection: close\r\n" */
/*   "\r\n"; */
/* const rom char *str_data3 = */
/*   "\r\n"; */
/* const far rom char msg_strings[][8] = {"OK\r\n", */
/* 				       "ready\r\n"}; */
