/**
  * @author: hxdyxd@gmail.com
  * @date: 2019 07 29
  */
#ifndef _HRTIM_4SWITCH_PWM_H
#define _HRTIM_4SWITCH_PWM_H

#include "hrtim.h"

#define  H4SPWM_PERIOD         (4000)
#define  H4SPWM_PERIOD_5PER    (H4SPWM_PERIOD/20)
#define  H4SPWM_PERIOD_90PER    (H4SPWM_PERIOD - 2*H4SPWM_PERIOD/20)
#define  H4SPWM_PERIOD_95PER    (H4SPWM_PERIOD - H4SPWM_PERIOD/20)
#define  H4SPWM_PERIOD_185PER    (H4SPWM_PERIOD_90PER*2 + H4SPWM_PERIOD_5PER)


/**
  * @brief  4 SWITCH PWM INIT
  */
void h4s_pwm_init(void);

/**
  * @brief  SET PWM DUTY
  * @retval None
  */
static inline void h4s_pwm_set_duty(uint8_t id, uint16_t duty);

/**
  * @brief  SET BUCK-BOOST PWM DUTY
  * @retval None
  */
static inline void h4s_buck_boost_pwm_set_duty(uint16_t duty);

#endif
/******************* (C) COPYRIGHT 2019 07 29 hxdyxd *****END OF FILE****/
