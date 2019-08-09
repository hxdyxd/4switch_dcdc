/* 
 *
 *2019 07 19 & hxdyxd
 *
 */
#ifndef _TTS_H
#define _TTS_H

#include <stdint.h>
#include <string.h>
#include "data_interface_hal.h"

#define   USE_TTS   (0)


#if USE_TTS

#define tts_printf(...)  do {\
snprintf(ttsgbuf, sizeof(ttsgbuf), __VA_ARGS__);\
tts_puts(ttsgbuf, strlen(ttsgbuf));\
}while(0)
extern char ttsgbuf[512];


int tts_puts(char *str, int len);
void tts_init(void);
void tts_proc(void);

#else

#define tts_printf(...) printf(__VA_ARGS__)
#define tts_init()
#define tts_proc()

#endif


#endif
/*****************************END OF FILE***************************/
