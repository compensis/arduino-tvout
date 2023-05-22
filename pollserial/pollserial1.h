#ifndef POLLSERIAL1_H
#define POLLSERIAL1_H

#include "pollserial.h"

class Pollserial1 : public Pollserial {
  public:
    pt2Funct begin(long);
    void end();
    uint8_t available(void);
    int read(void);
    void flush(void);
    size_t write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
};

void USART1_recieve();
#endif // #define POLLSERIAL1_H