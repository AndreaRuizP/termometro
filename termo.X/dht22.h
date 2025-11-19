#ifndef DHT22_H
#define DHT22_H

#define _XTAL_FREQ 20000000  

#include <xc.h>
#include <stdint.h>

#define TRIS_DHT TRISDbits.TRISD0
#define PORT_DHT PORTDbits.RD0

void DHT22_init(void);
uint8_t DHT22_read(float *dht_temperatura, float *dht_humedad);

#endif