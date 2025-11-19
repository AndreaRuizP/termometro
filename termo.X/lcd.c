#include <xc.h>
#include "lcd.h"

#define _XTAL_FREQ 20000000

// Pines según tu configuración
#define RS RC5      // RS en RC5
#define EN RC6      // EN en RC6
#define D4 RD4      // D4 en RD4
#define D5 RD5      // D5 en RD5
#define D6 RD6      // D6 en RD6
#define D7 RD7      // D7 en RD7

static void LCD_nibble(unsigned char nib) {
    D4 = (nib >> 0) & 1;
    D5 = (nib >> 1) & 1;
    D6 = (nib >> 2) & 1;
    D7 = (nib >> 3) & 1;

    EN = 1; 
    __delay_us(50);
    EN = 0; 
    __delay_us(50);
}

void LCD_cmd(unsigned char cmd) {
    RS = 0;
    LCD_nibble(cmd >> 4);
    LCD_nibble(cmd & 0x0F);
    __delay_ms(2);
}

void LCD_data(unsigned char data) {
    RS = 1;
    LCD_nibble(data >> 4);
    LCD_nibble(data & 0x0F);
    __delay_ms(2);
}

void LCD_init(void) {
    // Configurar pines como salida
    TRISCbits.TRISC5 = 0;  // RS
    TRISCbits.TRISC6 = 0;  // EN
    TRISDbits.TRISD4 = 0;  // D4
    TRISDbits.TRISD5 = 0;  // D5
    TRISDbits.TRISD6 = 0;  // D6
    TRISDbits.TRISD7 = 0;  // D7
    
    RS = 0;
    EN = 0;
    __delay_ms(20);

    LCD_nibble(0x03); __delay_ms(5);
    LCD_nibble(0x03); __delay_ms(5);
    LCD_nibble(0x03); __delay_ms(5);
    LCD_nibble(0x02);

    LCD_cmd(0x28);  // 4 bits, 2 líneas, 5x8
    LCD_cmd(0x0C);  // Display ON, cursor OFF
    LCD_cmd(0x06);  // Increment cursor
    LCD_cmd(0x01);  // Clear display
    __delay_ms(2);
}

void LCD_goto(uint8_t pos) {
    LCD_cmd(0x80 + pos);
}

void LCD_print(const char *s) {
    while(*s) LCD_data(*s++);
}