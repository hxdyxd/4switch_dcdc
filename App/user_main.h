/* 2019 04 10 */
/* By hxdyxd */

#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#include "app_debug.h"

#define AUTO_OLED_DISPLAY_OFF_TIME   (30000)


#define STIMER_FM_CTL_PROC_ID            (0)
#define STIMER_SHOW_PROC_ID              (1)
#define STIMER_OLED_AUTO_OFF_PROC_ID     (2)

#define SCREEN_BUFFER_MAX_SIZE          (20)


/* LEDS */
#define LED_OFF(id)  HAL_GPIO_WritePin(id, GPIO_PIN_SET)
#define LED_ON(id)   HAL_GPIO_WritePin(id, GPIO_PIN_RESET)
#define LED_REV(id)  HAL_GPIO_TogglePin(id)

#define LED_BASE   GPIOC, GPIO_PIN_13


void user_system_setup(void);
void user_setup(void);
void user_loop(void);


#endif
/*****************************END OF FILE***************************/
