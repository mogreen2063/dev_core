#include <p18f24j50.h>
#include <usart.h>
#include <delays.h>
#include <string.h>

#include "stdint.h"
#include "setup.h"
#include "wifi.h"
#include "port.h"

#pragma config WDTEN   = OFF
#pragma config T1DIG   = OFF
#pragma config PLLDIV  = 2
#pragma config CPUDIV  = OSC2_PLL2
#pragma config OSC     = INTOSCPLLO
#pragma config XINST   = OFF
#pragma config DEBUG   = OFF
#pragma config CP0     = OFF
#pragma config IOL1WAY = OFF

#pragma udata udata1
int8_t uart1_buffer[BUFFER1_SIZE];

#pragma udata udata2
int8_t uart2_buffer[BUFFER2_SIZE];

#pragma udata udata3
int8_t ssid[128];
int8_t pass[128];

#pragma udata
Port_t port1;
Port_t port2;
Wifi_t wifi;
int8_t key[16];

#pragma code isr = 0x08
#pragma interrupt isr
void isr(void)
{
  if(PIR1bits.RC1IF) {
    port1.buffer->mem[port1.buffer->head] = Read1USART();
    if(port1.buffer->mem[port1.buffer->head++] == '\n') {
      port1.buffer->flag++;
    }
  }
  
  if(PIR3bits.RC2IF) {
    port2.buffer->mem[port2.buffer->head] = Read2USART();
    if(port2.buffer->mem[port2.buffer->head++] == '\n') {
      port2.buffer->flag++;
    }
  }

  /* if(INTCON3bits.INT1IF) { */
  /*   if(!pulse_flag) { */
  /*     T0CONbits.TMR0ON = 1;    /\* start timer *\/ */
  /*     INTCON2bits.INTEDG1 = 0; /\* change to falling *\/ */
  /*     INTCON3bits.INT1IF = 0; */
  /*     pulse_flag = 1; */
  /*   } */
  /*   else { */
  /*     T0CONbits.TMR0ON = 0;    /\* stop timer *\/ */
  /*     INTCON2bits.INTEDG1 = 1; /\* change to rising *\/ */
  /*     INTCON3bits.INT1IF = 0; */
  /*     pulse_flag = 0; */
  /*     time_flag = 1; */
  /*   } */
  /* } */
}
  
#pragma code
void main(void)
{
  /* setup to rcv a key from comp(could be "AT" but that assumes junk is NULL) */
  /* the key must be sent by the computer. if junk is not NULL then another */
  /* can be sent to verify, but the buffer would have been flushed by the first */
  
  setup();

  /* init port1 */
  port1.state = P_IDLE;
  port1.cmd_state = C_IDLE;
  port1.buffer->mem = uart1_buffer;
  port1.buffer->head = 0;
  port1.buffer->tail = 0;
  port1.buffer->echo = 0;
  port1.buffer->flag = 0;


  /* init port2 */
  port2.state = P_IDLE;
  port2.cmd_state = C_IDLE;
  port2.buffer->mem = uart2_buffer;
  port2.buffer->head = 0;
  port2.buffer->tail = 0;
  port2.buffer->echo = 0;
  port2.buffer->flag = 0;

  /* init wifi */
  wifi.state = W_IDLE;
  wifi.ok = 0;
  wifi.ssid = ssid;
  wifi.pass = pass;
  wifi.http->request = 0;
  wifi.http->packet_length = 0;
  wifi.http->content_length = 0;
  wifi.http->key = key;
  strcpypgm2ram(ssid,(const far rom int8_t *)"0"); 
  strcpypgm2ram(pass,(const far rom int8_t *)"0"); 
  strcpypgm2ram(key,(const far rom int8_t *)"0"); 

  while(1)
    {
      handle_port1(&port1, &wifi);
      handle_port2(&port2, &wifi);
    }
}
