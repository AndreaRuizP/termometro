#include "dht22.h"
#include <stdio.h>

void DHT22_init(void){
    TRIS_DHT = 0; 
    PORT_DHT = 1;
    __delay_ms(1000);
}

uint8_t DHT22_read(float *dht_temperatura, float *dht_humedad){
    uint8_t bits[5];
    uint8_t i, j = 0;
    uint8_t timeout;

    TRIS_DHT = 0;
    PORT_DHT = 0; 
    __delay_ms(2);

    PORT_DHT = 1;  
    __delay_us(30);

    TRIS_DHT = 1;

    timeout = 0;
    while(PORT_DHT) {
        __delay_us(1);
        timeout++;
        if(timeout > 100) {
            TRIS_DHT = 0;
            PORT_DHT = 1;
            return 0;
        }
    }

    timeout = 0;
    while(!PORT_DHT) {
        __delay_us(1);
        timeout++;
        if(timeout > 100) {
            TRIS_DHT = 0;
            PORT_DHT = 1;
            return 0;
        }
    }

    timeout = 0;
    while(PORT_DHT) {
        __delay_us(1);
        timeout++;
        if(timeout > 100) {
            TRIS_DHT = 0;
            PORT_DHT = 1;
            return 0;
        }
    }

    for (j = 0; j < 5; j++){
        uint8_t result = 0;
        for (i = 0; i < 8; i++){
            timeout = 0;
            while (!PORT_DHT) {
                __delay_us(1);
                timeout++;
                if(timeout > 100) {
                    TRIS_DHT = 0;
                    PORT_DHT = 1;
                    return 0;
                }
            }
            
            __delay_us(30);
            
            if (PORT_DHT) {
                result |= (1 << (7 - i));
            }
            
            timeout = 0;
            while(PORT_DHT) {
                __delay_us(1);
                timeout++;
                if(timeout > 100) {
                    TRIS_DHT = 0;
                    PORT_DHT = 1;
                    return 0;
                }
            }
        }
        bits[j] = result;
    }

    TRIS_DHT = 0;
    PORT_DHT = 1;

    uint8_t checksum = bits[0] + bits[1] + bits[2] + bits[3];
    if (checksum != bits[4]){
        return 0;
    }

    uint16_t rawhumidity = (bits[0] << 8) | bits[1];
    *dht_humedad = (float)(rawhumidity) / 10.0;

    uint16_t rawtemperature = (bits[2] << 8) | bits[3];
    
    if (rawtemperature & 0x8000){
        *dht_temperatura = (float)((rawtemperature & 0x7FFF) / 10.0) * -1.0;
    } else {
        *dht_temperatura = (float)(rawtemperature) / 10.0;
    }

    return 1;
}