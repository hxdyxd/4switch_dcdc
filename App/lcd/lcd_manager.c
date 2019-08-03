/* 2019 04 10 */
/* By hxdyxd */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "lcd_manager.h"


char lcd_console_buffer[512];



void window1callback(UG_MESSAGE *msg)
{
    
}

#define  MAXOBJECTS  10

static UG_WINDOW window1 ;
static UG_BUTTON button1 ;
static UG_BUTTON button2 ;
static UG_BUTTON button3 ;
static UG_IMAGE   image1 ;


static UG_OBJECT objbuffwnd1 [MAXOBJECTS] ;

static UG_GUI gui ;//  Global GUI structure


void gui_init(void)
{
    UG_Init(&gui, (void *)lcd240x240_drawpoint, 240, 240);
    UG_SelectGUI(&gui);
    
    UG_DriverRegister(DRIVER_FILL_FRAME, (void *)lcd240x240_fill);
    
    UG_ConsoleSetArea(0, 120, 240-1, 240-1);
    UG_FontSelect ( &FONT_8X14 );
    UG_ConsoleSetBackcolor ( C_BLACK );
    UG_ConsoleSetForecolor ( C_WHITE );
    UG_ConsolePutString("console init\n");
    printf("console init \r\n");
}


void gui_window_init(void)
{
    UG_WindowCreate(&window1,  objbuffwnd1, MAXOBJECTS,  window1callback) ;
    
    UG_WindowSetStyle (&window1 , WND_STYLE_3D | WND_STYLE_SHOW_TITLE);
    UG_WindowSetXStart (&window1 , 0);
    UG_WindowSetYStart (&window1 , 0);
    UG_WindowSetXEnd (&window1 , 240-1);
    UG_WindowSetYEnd (&window1 , 120-1);
    
    UG_WindowSetForeColor(&window1 , C_RED) ;

    UG_WindowSetTitleText(&window1, "uGUI Demo Window");
    UG_WindowSetTitleTextFont(&window1, &FONT_12X20 );
    
    UG_ButtonCreate ( &window1 , &button1 ,  BTN_ID_0 ,  10 ,  10 ,  10 + 50 ,  10 + 50  ) ;
    UG_ButtonSetText ( &window1 ,  BTN_ID_0 , "Run");
    
    UG_ButtonCreate ( &window1 , &button2 ,  BTN_ID_1 ,  70 ,  10 ,  70 + 50 ,  10 + 50  ) ;
    UG_ButtonSetText ( &window1 ,  BTN_ID_1 , "Stop");
    
    UG_ButtonCreate ( &window1 , &button3 ,  BTN_ID_2 ,  130 ,  10 ,  130 + 50 ,  10 + 50  ) ;
    UG_ButtonSetText ( &window1 ,  BTN_ID_2 , "Exit");
    
    extern const unsigned char gImage_bmp[85*120*2];
    
    static UG_BMP logo ={
        (void *)gImage_bmp,
        85,
        120,
        BMP_BPP_16,
        BMP_RGB565,
    };
    
//    UG_ImageCreate (&window1, &image1, IMG_ID_0, 10, 120, 10 + logo.width - 1, 120 + logo.height - 1);
//    UG_ImageSetBMP(&window1, IMG_ID_0, &logo);
    
    UG_WindowHide(&window1);
    
//    UG_DrawBMP(0, 0, &logo);
    
    for(uint16_t i = 0; i < logo.height; i++) {
        for(uint16_t j = 0; j < logo.width; j++) {
            int local = i*logo.width + j;
            lcd240x240_drawpoint(j, i, gImage_bmp[local*2] | (gImage_bmp[local*2+1]<<8) );
        }
    }
    
}


void gui_wave_init(struct lcd_wave_t *hwav, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *buffer, uint16_t bgcolor)
{
    hwav->x = x;
    hwav->y = y;
    hwav->width = width;
    hwav->height = height;
    hwav->buffer = buffer;
    hwav->bgcolor = bgcolor;
    hwav->color = C_WHITE;
    hwav->value = 0;
}



void gui_wave_set(struct lcd_wave_t *hwav, uint8_t value, uint16_t color)
{
    hwav->color = color;
    memmove(hwav->buffer, hwav->buffer+1, hwav->width - 1);
    hwav->buffer[hwav->width-1] = hwav->height-1-value;
}


void gui_wave_draw(struct lcd_wave_t **hwavs, uint8_t num)
{
    struct lcd_wave_t *hwav = hwavs[0];
    
    for(uint16_t j=hwav->x; j < hwav->width; j++) {
        for(uint16_t i=hwav->y; i < hwav->height; i++) {
                if(hwavs[0]->buffer[j] == i || hwavs[0]->buffer[j] == 1+i) {
                    lcd240x240_drawpoint(j, i, hwavs[0]->color);
                } else if(hwavs[1]->buffer[j] == i || hwavs[1]->buffer[j] == 1+i) {
                    lcd240x240_drawpoint(j, i, hwavs[1]->color);
                } else {
                    lcd240x240_drawpoint(j, i, hwav->bgcolor);
                }
        }
    }
}

