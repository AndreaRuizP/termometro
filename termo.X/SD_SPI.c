#include "SD_SPI.h"
#include <string.h>

#define SD_CS_LOW()   SD_CS_PIN = 0
#define SD_CS_HIGH()  SD_CS_PIN = 1

static uint32_t current_sector = 100;
static uint16_t current_offset = 0;

void SD_SPI_Init(void){
    SD_CS_TRIS = 0;
    SD_SCK_TRIS = 0;
    SD_MOSI_TRIS = 0;
    SD_MISO_TRIS = 1;
    SD_CS_HIGH();
    SD_SCK_PIN = 0;
    SD_MOSI_PIN = 1;
}

static uint8_t SD_SPI_Transfer(uint8_t data){
    uint8_t i, result = 0;
    for(i = 0; i < 8; i++){
        SD_MOSI_PIN = (data & 0x80) ? 1 : 0;
        data <<= 1;
        SD_SCK_PIN = 1;
        __delay_us(2);
        if(SD_MISO_PIN) result |= (0x80 >> i);
        SD_SCK_PIN = 0;
        __delay_us(2);
    }
    return result;
}

static uint8_t SD_Cmd(uint8_t cmd, uint32_t arg){
    uint8_t r, i, crc = 0xFF;
    if(cmd == CMD0) crc = 0x95;
    if(cmd == CMD8) crc = 0x87;
    SD_SPI_Transfer(0xFF);
    SD_SPI_Transfer(cmd | 0x40);
    SD_SPI_Transfer((uint8_t)(arg >> 24));
    SD_SPI_Transfer((uint8_t)(arg >> 16));
    SD_SPI_Transfer((uint8_t)(arg >> 8));
    SD_SPI_Transfer((uint8_t)arg);
    SD_SPI_Transfer(crc);
    for(i = 0; i < 10; i++){
        r = SD_SPI_Transfer(0xFF);
        if(!(r & 0x80)) return r;
    }
    return 0xFF;
}

uint8_t SD_CheckPresent(void){
    uint8_t r;
    SD_CS_LOW();
    __delay_us(10);
    r = SD_Cmd(CMD13, 0);
    SD_SPI_Transfer(0xFF);
    SD_CS_HIGH();
    SD_SPI_Transfer(0xFF);
    return (r != 0xFF);
}

uint8_t SD_Init(void){
    uint8_t i, r;
    uint16_t retry;
    SD_SPI_Init();
    SD_CS_HIGH();
    for(i = 0; i < 10; i++) SD_SPI_Transfer(0xFF);
    SD_CS_LOW();
    __delay_ms(2);
    for(retry = 0; retry < 200; retry++){
        if(SD_Cmd(CMD0, 0) == 0x01) break;
        __delay_ms(10);
    }
    if(retry >= 200){ SD_CS_HIGH(); return 0; }
    r = SD_Cmd(CMD8, 0x000001AA);
    if(r == 0x01){
        for(i = 0; i < 4; i++) SD_SPI_Transfer(0xFF);
    }
    for(retry = 0; retry < 1000; retry++){
        SD_Cmd(CMD55, 0);
        if(SD_Cmd(CMD41, 0x40000000) == 0x00) break;
        __delay_ms(10);
    }
    if(retry >= 1000){ SD_CS_HIGH(); return 0; }
    if(SD_Cmd(CMD16, 512) != 0x00){ SD_CS_HIGH(); return 0; }
    SD_CS_HIGH();
    SD_SPI_Transfer(0xFF);
    current_sector = 100;
    current_offset = 0;
    return 1;
}

void SD_AppendText(const char* text){
    uint16_t len = strlen(text);
    uint16_t i, j;
    uint8_t resp;
    
    if(current_offset + len > 512){
        current_sector++;
        current_offset = 0;
    }
    
    SD_CS_HIGH();
    SD_SPI_Transfer(0xFF);
    __delay_ms(2);
    SD_CS_LOW();
    __delay_ms(2);
    
    resp = SD_Cmd(CMD24, current_sector);
    if(resp != 0x00){ SD_CS_HIGH(); return; }
    
    __delay_ms(2);
    SD_SPI_Transfer(0xFE);
    
    for(i = 0; i < 512; i++){
        if(i >= current_offset && i < current_offset + len){
            SD_SPI_Transfer(text[i - current_offset]);
        } else {
            SD_SPI_Transfer(0x00);
        }
    }
    
    SD_SPI_Transfer(0xFF);
    SD_SPI_Transfer(0xFF);
    resp = SD_SPI_Transfer(0xFF);
    if((resp & 0x1F) != 0x05){ SD_CS_HIGH(); return; }
    
    j = 0;
    while(j < 65000){
        resp = SD_SPI_Transfer(0xFF);
        if(resp == 0xFF) break;
        j++;
    }
    
    SD_CS_HIGH();
    SD_SPI_Transfer(0xFF);
    __delay_ms(50);
    
    current_offset += len;
}