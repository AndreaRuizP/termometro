#ifndef SD_SPI_H
#define SD_SPI_H
#define _XTAL_FREQ 20000000
#include <xc.h>
#include <stdint.h>

#define SD_CS_PIN    PORTBbits.RB5
#define SD_CS_TRIS   TRISBbits.TRISB5
#define SD_SCK_PIN   PORTBbits.RB4
#define SD_SCK_TRIS  TRISBbits.TRISB4
#define SD_MOSI_PIN  PORTBbits.RB3
#define SD_MOSI_TRIS TRISBbits.TRISB3
#define SD_MISO_PIN  PORTBbits.RB2
#define SD_MISO_TRIS TRISBbits.TRISB2

#define CMD0    0x00
#define CMD8    0x08
#define CMD13   0x0D
#define CMD16   0x10
#define CMD24   0x18
#define CMD41   0x29
#define CMD55   0x37

void SD_SPI_Init(void);
uint8_t SD_Init(void);
uint8_t SD_CheckPresent(void);
void SD_AppendText(const char* text);

#endif