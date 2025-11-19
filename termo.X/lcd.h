#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void LCD_init(void);
void LCD_cmd(unsigned char cmd);
void LCD_data(unsigned char data);
void LCD_print(const char *s);
void LCD_goto(uint8_t pos);

#endif
