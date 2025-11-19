#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include "lcd.h"
#include "i2c.h"
#include "ds1307.h"
#include "dht22.h"
#include "SD_SPI.h"

#define _XTAL_FREQ 20000000
#define BUTTON PORTAbits.RA2

#pragma config FOSC = HS
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config MCLRE = ON
#pragma config CP = OFF
#pragma config CPD = OFF
#pragma config BOREN = OFF
#pragma config IESO = OFF
#pragma config FCMEN = OFF
#pragma config LVP = OFF
#pragma config BOR4V = BOR40V
#pragma config WRT = OFF

void main(void) {
    ANSEL = 0;
    ANSELH = 0;
    TRISA = 0b00000100;
    TRISB = 0b00000100;
    TRISC = 0b00011000;
    TRISD = 0b00000001;
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;

    LCD_init();
    I2C_init();
    DS1307_startOscillator();
    DHT22_init();
    
    // DS1307_setTime(4, 25, 0);
    // DS1307_setDate(18, 11, 25);

    uint8_t h = 0, m = 0, s = 0;
    uint8_t d = 18, mo = 11, y = 25;
    int T = 0, H = 0;
    char buf[17];
    char line[45];
    uint8_t screen = 0;
    uint8_t lastBtn = 1;
    uint8_t dhtCnt = 0;
    uint8_t sd_ok = 0;
    uint8_t first = 1;
    uint8_t saved = 0;
    uint8_t timer = 0;
    uint8_t sd_removed = 0;
    uint16_t reg = 1;
    uint8_t ultimo_min = 255;
    uint8_t min_inicio = 0;
    uint8_t h_inicio = 0;
    uint8_t iniciado = 0;
    uint8_t min_rest = 0;

    LCD_cmd(0x01);
    __delay_ms(2);
    LCD_goto(0x00);
    LCD_print("Sistema Pronost.");
    LCD_goto(0x40);
    LCD_print("Meteorologico");
    __delay_ms(2500);

    LCD_cmd(0x01);
    __delay_ms(2);
    LCD_goto(0x00);
    LCD_print("Iniciando SD...");
    __delay_ms(1000);

    if(SD_Init()) {
        sd_ok = 1;
        LCD_cmd(0x01);
        __delay_ms(2);
        LCD_goto(0x00);
        LCD_print("SD OK!");
        __delay_ms(1500);
        SD_AppendText("ID,Fecha,Hora,TempC,Hum\r\n");
    } else {
        LCD_cmd(0x01);
        __delay_ms(2);
        LCD_goto(0x00);
        LCD_print("ERROR: SD");
        __delay_ms(2000);
    }

    LCD_cmd(0x01);
    __delay_ms(2);

    while(1) {
        DS1307_getTime(&h, &m, &s);
        DS1307_getDate(&d, &mo, &y);

        if(lastBtn == 1 && BUTTON == 0) {
            __delay_ms(50);
            if(BUTTON == 0) {
                screen++;
                if(screen > 2) screen = 0;
                LCD_cmd(0x01);
                __delay_ms(2);
            }
        }
        lastBtn = BUTTON;

        if(dhtCnt >= 100) {
            float tf, hf;
            if(DHT22_read(&tf, &hf)) {
                T = (int)(tf + 0.5);
                H = (int)(hf + 0.5);
                first = 0;
            } else {
                first = 1;
            }
            dhtCnt = 0;
        }
        dhtCnt++;

        if(sd_ok && !first && !sd_removed) {
            
            if(!iniciado) {
                min_inicio = m;
                h_inicio = h;
                ultimo_min = m;
                iniciado = 1;
            }
            
            uint16_t min_total = 0;
            if(h >= h_inicio) {
                min_total = ((h - h_inicio) * 60) + (m - min_inicio);
            } else {
                min_total = ((24 - h_inicio + h) * 60) + (m - min_inicio);
            }
            
            if(min_total >= 360) {
                sd_ok = 0;
                LCD_cmd(0x01);
                __delay_ms(2);
                LCD_goto(0x00);
                LCD_print("6h COMPLETAS");
                LCD_goto(0x40);
                LCD_print("Finalizado");
                __delay_ms(2000);
                LCD_cmd(0x01);
                __delay_ms(2);
            } else {
                int16_t diff = m - ultimo_min;
                if(diff < 0) diff += 60;
                
                if(diff >= 5) {
                    sprintf(line, "%u,%02u/%02u/20%02u,%02u:%02u:%02u,%d,%d\r\n",
                            reg, d, mo, y, h, m, s, T, H);
                    SD_AppendText(line);
                    reg++;
                    ultimo_min = m;
                    saved = 1;
                    timer = 0;
                }
            }
        }

        if(saved) {
            timer++;
            if(timer >= 50) {
                saved = 0;
                timer = 0;
            }
        }

        if(sd_ok && iniciado) {
            int16_t d2 = m - ultimo_min;
            if(d2 < 0) d2 += 60;
            min_rest = 5 - (uint8_t)d2;
            if(min_rest > 5) min_rest = 0;
        }

        if(screen == 0) {
            LCD_goto(0x00);
            sprintf(buf, "   %02u:%02u:%02u   ", h, m, s);
            LCD_print(buf);
            LCD_goto(0x40);
            sprintf(buf, "  %02u/%02u/20%02u  ", d, mo, y);
            LCD_print(buf);
            
        } else if(screen == 1) {
            LCD_goto(0x00);
            if(first) {
                LCD_print("T:--C  H:--%   ");
            } else {
                sprintf(buf, "T:%dC  H:%d%%   ", T, H);
                LCD_print(buf);
            }
            
            LCD_goto(0x40);
            if(sd_removed) {
                LCD_print("SD RETIRADA     ");
            } else if(!sd_ok) {
                LCD_print("Finalizado      ");
            } else {
                if(saved) {
                    LCD_print("   GUARDADO!   ");
                } else {
                    sprintf(buf, "Prox:%umin     ", min_rest);
                    LCD_print(buf);
                }
            }
            
        } else {
            LCD_goto(0x00);
            sprintf(buf, "Reg:%u/72     ", reg - 1);
            LCD_print(buf);
            
            LCD_goto(0x40);
            if(!sd_ok) {
                LCD_print("FIN            ");
            } else {
                sprintf(buf, "%02u:%02u T:%d H:%d  ", h, m, T, H);
                LCD_print(buf);
            }
        }

        __delay_ms(20);
    }
}