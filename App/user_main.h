/* 2019 04 10 */
/* By hxdyxd */

#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#include "app_debug.h"


#define MODE_CH1         0

#define SOFT_TIMER_LED_DEBUG_ID        (0)
#define SOFT_TIMER_LOW_VIN_ID          (1)
#define SOFT_TIMER_UGUI_ID             (2)


#define GET_VOUT()         adc_real_value_array[0]
#define GET_VIN()          adc_real_value_array[1]
#define GET_VLED()         adc_real_value_array[2]

#define MAX_OUTPUT_DUTY    H4SPWM_PERIOD_185PER


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
