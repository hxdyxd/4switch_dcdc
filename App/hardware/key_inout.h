/* 2019 08 05 */
/* By hxdyxd */
#ifndef _KEY_INOUT_H
#define _KEY_INOUT_H

#include <stdint.h>


typedef void(* KEY_INOUT_CB_T)(int8_t id);


void key_inout_init(void);
void key_inout_proc(KEY_INOUT_CB_T key_func);


#endif
/*****************************END OF FILE***************************/
