#include <avr/io.h>
#include <stdlib.h>
#include "Pollserial1.h"

#define BUFFER_SIZE 64

namespace {
  rbuffer rxbuffer = {0,0,0};
}

void USART1_recieve() {
  if( UCSR1A & _BV(RXC1)) {
    uint8_t i = (rxbuffer.head + 1) & (BUFFER_SIZE - 1);
    if ( i != rxbuffer.tail) {
      rxbuffer.buffer[rxbuffer.head] = UDR1;
      rxbuffer.head = i;
    }
  }
}

pt2Funct Pollserial1::begin(long baud) {
  uint16_t baud_setting;
  bool use_u2x;
  
  rxbuffer.buffer = (unsigned char*)malloc(BUFFER_SIZE*sizeof(unsigned char));

  // U2X mode is needed for baud rates higher than (CPU Hz / 16)
  if (baud > F_CPU / 16) {
    use_u2x = true;
  }
  else {
    // figure out if U2X mode would allow for a better connection
    
    // calculate the percent difference between the baud-rate specified and
    // the real baud rate for both U2X and non-U2X mode (0-255 error percent)
    uint8_t nonu2x_baud_error = abs((int)(255-((F_CPU/(16*(((F_CPU/8/baud-1)/2)+1))*255)/baud)));
    uint8_t u2x_baud_error = abs((int)(255-((F_CPU/(8*(((F_CPU/4/baud-1)/2)+1))*255)/baud)));
    
    // prefer non-U2X mode because it handles clock skew better
    use_u2x = (nonu2x_baud_error > u2x_baud_error);
  }
  if (use_u2x) {
    UCSR1A = _BV(U2X1);
    baud_setting = (F_CPU / 4 / baud - 1) / 2;
  }
  else {
    UCSR1A = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }

  // assign the baud_setting, a.k.a. (USART Baud Rate Register)
  UBRR1 = baud_setting;
  UCSR1B = _BV(RXEN1) | _BV(TXEN1);

  return &USART1_recieve;
}

void Pollserial1::end() {
  UCSR1B &= ~(_BV(RXEN1) | _BV(TXEN1));
  free(rxbuffer.buffer);
}

uint8_t Pollserial1::available() {
  return (BUFFER_SIZE + rxbuffer.head - rxbuffer.tail) & (BUFFER_SIZE-1);
}

int Pollserial1::read() {
  if (rxbuffer.head == rxbuffer.tail)
    return -1;
  else {
    uint8_t c = rxbuffer.buffer[rxbuffer.tail];
    if (rxbuffer.tail == BUFFER_SIZE)
      rxbuffer.tail = 1;
    else
      rxbuffer.tail++;
    return c;
  }
}

void Pollserial1::flush() {
  rxbuffer.head = rxbuffer.tail;
}

size_t Pollserial1::write(uint8_t c) {
  while (!((UCSR1A) & _BV(UDRE1)));
  UDR1 = c;
  return 1;
}