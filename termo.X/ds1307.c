#include <xc.h>
#include "ds1307.h"
#include "i2c.h"

static uint8_t BCD_to_dec(uint8_t bcd){
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

void DS1307_startOscillator(void) {
    uint8_t sec;

    I2C_start(); I2C_write(0xD0); I2C_write(0x00);
    I2C_start(); I2C_write(0xD1);
    sec = I2C_read(1);
    I2C_stop();

    sec &= 0x7F;

    I2C_start();
    I2C_write(0xD0);
    I2C_write(0x00);
    I2C_write(sec);
    I2C_stop();
}

void DS1307_getTime(uint8_t *h, uint8_t *m, uint8_t *s){
    I2C_start();
    I2C_write(0xD0);
    I2C_write(0x00);

    I2C_start();
    I2C_write(0xD1);

    *s = BCD_to_dec(I2C_read(0));
    *m = BCD_to_dec(I2C_read(0));
    *h = BCD_to_dec(I2C_read(1));

    I2C_stop();
}

void DS1307_getDate(uint8_t *d, uint8_t *mo, uint8_t *y){
    I2C_start();
    I2C_write(0xD0);
    I2C_write(0x04);

    I2C_start();
    I2C_write(0xD1);

    *d  = BCD_to_dec(I2C_read(0));
    *mo = BCD_to_dec(I2C_read(0));
    *y  = BCD_to_dec(I2C_read(1));

    I2C_stop();
}
