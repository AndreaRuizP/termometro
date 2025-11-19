#include <xc.h>
#include "i2c.h"

#define _XTAL_FREQ 20000000

static void I2C_wait() {
    while ((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));
}

void I2C_init(void) {
    TRISC3 = 1;  
    TRISC4 = 1; 

    SSPCON = 0b00101000;  
    SSPCON2 = 0;
    SSPADD = (_XTAL_FREQ / (4 * 100000)) - 1;  
    SSPSTAT = 0;
}

void I2C_start() { I2C_wait(); SEN = 1; }
void I2C_stop()  { I2C_wait(); PEN = 1; }

void I2C_write(uint8_t d) {
    I2C_wait();
    SSPBUF = d;
}

uint8_t I2C_read(uint8_t ack) {
    uint8_t data;

    I2C_wait();
    RCEN = 1;
    I2C_wait();
    data = SSPBUF;
    I2C_wait();
    ACKDT = ack;
    ACKEN = 1;

    return data;
}
