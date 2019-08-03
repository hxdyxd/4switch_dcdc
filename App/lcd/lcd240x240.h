/* 2019 08 01 */
/* By hxdyxd */

#ifndef _LCD240X240_H
#define _LCD240X240_H

#include <stdint.h>

#define USE_LCD_DMA  (1)


#define LCD_W        240
#define LCD_H        240
#define LCD_SIZE    (LCD_W*LCD_H)

//»­±ÊÑÕÉ«
#define WHITE            0xFFFF
#define BLACK            0x0000
#define BLUE             0x001F
#define BRED             0XF81F
#define GRED             0XFFE0
#define GBLUE            0X07FF
#define RED              0xF800
#define MAGENTA          0xF81F
#define GREEN            0x07E0
#define CYAN             0x7FFF
#define YELLOW           0xFFE0
#define BROWN            0XBC40 //×ØÉ«
#define BRRED            0XFC07 //×ØºìÉ«
#define GRAY             0X8430 //»ÒÉ«


#define  mdelay(t)   HAL_Delay(t)

void lcd240x240_init(void);
void lcd240x240_display(int on);
void lcd240x240_clear(uint16_t color);
int lcd240x240_fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);
void lcd240x240_drawpoint(uint16_t x, uint16_t y, uint16_t color);

#if USE_LCD_DMA
int lcd240x240_flush(void);
#else
#define lcd240x240_flush()
#endif

#endif
/*****************************END OF FILE***************************/
