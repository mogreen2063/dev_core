#include <string.h>
#include <usart.h>
#include "setup.h"
#include "stdint.h"

#pragma code
void send_char(const far rom int8_t * c, uint8_t dest)
{
  if(dest == 1)
    {
      while(Busy1USART());
      Write1USART(*c);
    }
  else if(dest == 2)
    {
      while(Busy2USART());
      Write2USART(*c);
    }
}

void send_char_ram(int8_t * c, uint8_t dest)
{
  if(dest == 1)
    {
      while(Busy1USART());
      Write1USART(*c);
    }
  else if(dest == 2)
    {
      while(Busy2USART());
      Write2USART(*c);
    }
}

void send_msg_ram(far int8_t * msg, uint8_t dest)
{
  if(dest == 1)
    {
      do {
	while(Busy1USART());
	Write1USART(*msg);
      } while(*(++msg));
    }
  else if(dest == 2)
    {
      do {
	while(Busy2USART());
	Write2USART(*msg);
      } while(*(++msg));
    }
}

void send_msg(const far rom int8_t * msg, uint8_t dest)
{
  if(dest == 1)
    {
      do {
	while(Busy1USART());
	Write1USART(*msg);
      } while(*(++msg));
    }
  else if(dest == 2)
    {
      do {
	while(Busy2USART());
	Write2USART(*msg);
      } while(*(++msg));
    }

}

uint8_t msg_check(int8_t * mem, uint8_t index,
		  const far rom int8_t * msg)
{
  uint8_t n = strlenpgm(msg);
  /* uint8_t n = 7; */

  while(n--)
    {
      /* send_char_ram(&mem[index],1); */
      /* send_char(msg,1); */
      if(mem[index++] != *msg++)
	{ return 0; }
    }
  return 1;
}

void hex2ascii(int8_t * ascii_string, uint8_t n)
{
  *ascii_string = n;
  *ascii_string += '0';
  if(*ascii_string > '9')
    {
      *ascii_string += 7;
    }
}

void hex2dec(int8_t * ascii_string, uint8_t n)
{
  uint8_t count;
  uint8_t first = 0;
  uint8_t mult[] = {1,10,100};
  uint8_t index = 2;

  if(n)
    {
      do {
	count = 0;
	while(n >= mult[index])
	  {
	    n -= mult[index];
	    count++;
	    first = 1;
	  }
	if(first)
	  {
	    hex2ascii(ascii_string,count);
	    ascii_string++;
	  }
      } while(index--);
      *ascii_string = '\0';
    }
  else
    {
      *ascii_string++ = '0';
      *ascii_string = '\0';
    }
}
