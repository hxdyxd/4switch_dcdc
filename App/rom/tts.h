/* 
 *
 *2019 07 19 & hxdyxd
 *
 */
#ifndef _TTS_H
#define _TTS_H

#include <stdint.h>
#include "data_interface_hal.h"

int tts_puts(char *str, int len);
void tts_init(void);
void tts_proc(void);

#endif
/*****************************END OF FILE***************************/
