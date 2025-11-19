#ifndef DS1307_H
#define DS1307_H

#include <stdint.h>

void DS1307_startOscillator(void);
void DS1307_getTime(uint8_t *h, uint8_t *m, uint8_t *s);
void DS1307_getDate(uint8_t *d, uint8_t *mo, uint8_t *y);

#endif
