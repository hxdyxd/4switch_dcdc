/* 2019 04 10 */
/* By hxdyxd */

#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#include "app_debug.h"


#define MODE_BATIN         0
#define MODE_LEDOUT        1



#define SOFT_TIMER_LED_DEBUG_ID        (0)
#define SOFT_TIMER_LOW_VIN_ID          (1)
#define SOFT_TIMER_UGUI_ID             (2)



#define MAX_OUTPUT_DUTY    H4SPWM_PERIOD_185PER


#define SCREEN_BUFFER_MAX_SIZE          (20)



void user_system_setup(void);
void user_setup(void);
void user_loop(void);


#endif
/*****************************END OF FILE***************************/
