/* 2019 04 10 */
/* By hxdyxd */
#ifndef _LCD_MANAGER_H
#define _LCD_MANAGER_H

#include "ugui.h"
#include "data_interface_hal.h"

extern char lcd_console_buffer[512];
extern uint8_t lcd_console_enable;


#define lcd_printf(...)  do {\
if(lcd_console_enable) {\
int len = snprintf(lcd_console_buffer, sizeof(lcd_console_buffer), __VA_ARGS__);\
if(len >20) {\
    len = 20;\
}\
lcd_console_buffer[len] = '\0';\
UG_ConsolePutString(lcd_console_buffer);\
}\
}while(0)


#define ug_printf(x,y,c,...)  do {\
int len = snprintf(lcd_console_buffer, sizeof(lcd_console_buffer), __VA_ARGS__);\
lcd_console_buffer[len] = '\0';\
UG_SetForecolor(c);\
UG_PutString(x, y, lcd_console_buffer);\
}while(0)



struct lcd_wave_t{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint16_t bgcolor;
    uint16_t color;
    uint8_t value;
    uint8_t *buffer;
};


void gui_init(void);
void gui_window_init(void);

void gui_wave_init(struct lcd_wave_t *hwav, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *buffer, uint16_t bgcolor);
uint8_t gui_wave_get(struct lcd_wave_t *hwav);
void gui_wave_set(struct lcd_wave_t *hwav, uint8_t value, uint16_t color);
void gui_wave_draw(struct lcd_wave_t **hwavs, uint8_t num);


#endif
