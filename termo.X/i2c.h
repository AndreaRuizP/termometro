#ifndef I2C_H
#define I2C_H

#include <stdint.h>

void I2C_init(void);
void I2C_start(void);
void I2C_stop(void);
void I2C_write(uint8_t d);
uint8_t I2C_read(uint8_t ack);

#endif
