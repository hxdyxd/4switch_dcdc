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
#define SOFT_TIMER_FREQSCAN_ID         (3)
#define SOFT_TIMER_OR_ID               (4)
#define SOFT_TIMER_FD_ID               (5)

#define MAX_OUTPUT_DUTY    H4SPWM_PERIOD_185PER


#define CHANNEL_IN1          (0)
#define CHANNEL_IN2          (1)
#define CHANNEL_OUT          (2)
#define CHANNEL_OUT_LOAD     (3)


/*1*/
#define KEY_MODE_QUARE           (0)
#define KEY_MODE_GAIN           (1)
#define KEY_MODE_FREQ_SCAN      (2)
#define KEY_MODE_RESET_1K       (3)

/*2*/
#define KEY_MODE_OR             (4)
#define KEY_MODE_IR             (5)
#define KEY_MODE_KADD           (6)
#define KEY_MODE_KSUB           (7)

/*3*/
#define KEY_MODE_SWITCH_PAGE    (8)
#define KEY_MODE_ADD            (9)
#define KEY_MODE_OK             (10)
#define KEY_MODE_SAVE           (11)

/*4*/
#define KEY_MODE_CONSOLE        (12)


#define LCD_MODE_DEFAULT0       (0)
#define LCD_MODE_FAULT          (1)
#define LCD_MODE_CHANNEL        (2)
#define LCD_MODE_NUMBER         (LCD_MODE_CHANNEL+1)

#define LCD_MODE_CAL            (50)
#define LCD_MODE_CAL_OK         (51)


//LCD_MODE_FAULT
#define LCD_MODE_FAULT_SHOW     (0)
#define LCD_MODE_FAULT_RI       (1)
#define LCD_MODE_FAULT_DCI      (2)
#define LCD_MODE_FAULT_DCO      (3)
#define LCD_MODE_FAULT_UP       (4)
#define LCD_MODE_FAULT_LP       (5)
#define LCD_MODE_FAULT_NUMBER   (LCD_MODE_FAULT_LP+1)



#define SCREEN_BUFFER_MAX_SIZE          (20)



void user_system_setup(void);
void user_setup(void);
void user_loop(void);


#endif
/*****************************END OF FILE***************************/
