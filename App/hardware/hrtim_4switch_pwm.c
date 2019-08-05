/**
  * @author: hxdyxd@gmail.com
  * @date: 2019 07 29
  */
#include "hrtim_4switch_pwm.h"

/**
  * @brief  4Í¨µÀPWMÊä³ö
  */
void h4s_pwm_init(void)
{
    //hrtim
    HAL_HRTIM_WaveformCountStart(&hhrtim, HRTIM_TIMERID_MASTER);
    
    HAL_HRTIM_WaveformOutputStart(&hhrtim, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    HAL_HRTIM_WaveformCountStart(&hhrtim, HRTIM_TIMERID_TIMER_A);
    
    HAL_HRTIM_WaveformOutputStart(&hhrtim, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
    HAL_HRTIM_WaveformCountStart(&hhrtim, HRTIM_TIMERID_TIMER_B);
    
    HAL_HRTIM_WaveformOutputStart(&hhrtim, HRTIM_OUTPUT_TD1);
    HAL_HRTIM_WaveformCountStart(&hhrtim, HRTIM_TIMERID_TIMER_D);  //ADC SINGLE DEBUG
    
    h4s_pwm_set_duty(2, H4SPWM_PERIOD/2);
    h4s_buck_boost_pwm_set_duty(H4SPWM_PERIOD_95PER);
}

/**
  * @brief  set Master Timer Compare 1 value
  * @retval None
  */
static inline void h4s_pwm_set_duty(uint8_t id, uint16_t duty)
{
    /* Master Timer Compare value */
    static __IO uint32_t * const HRTIM1_MCMPnR[4] = {
        &(HRTIM1->sMasterRegs.MCMP1R), 
        &(HRTIM1->sMasterRegs.MCMP2R), 
        &(HRTIM1->sMasterRegs.MCMP3R), 
        &(HRTIM1->sMasterRegs.MCMP4R),
    };
    if(id >= 4)
        return;
    *HRTIM1_MCMPnR[id] = (duty > H4SPWM_PERIOD) ? H4SPWM_PERIOD : duty;
}

/**
  * @brief  SET BUCK-BOOST PWM DUTY
  * @retval None
  */
void h4s_buck_boost_pwm_set_duty(uint16_t duty)
{
    if(duty <= H4SPWM_PERIOD_5PER){
        /* duty <= 5% */
        h4s_pwm_set_duty(0, H4SPWM_PERIOD_5PER);  //5%
        h4s_pwm_set_duty(1, H4SPWM_PERIOD_5PER);  //5%
    } else if(duty <= H4SPWM_PERIOD_95PER) {
        /* 5% < duty <= 95% */
        h4s_pwm_set_duty(0, duty);                //5% - 95%
        h4s_pwm_set_duty(1, H4SPWM_PERIOD_5PER);  //5%
    } else if(duty <= H4SPWM_PERIOD_185PER) {
        /* 95% < duty <= 185% */
        h4s_pwm_set_duty(0, H4SPWM_PERIOD_95PER);  //95%
        h4s_pwm_set_duty(1, duty - H4SPWM_PERIOD_90PER);  //5% - 95%
    } else {
        /* 185% <= duty */
        h4s_pwm_set_duty(0, H4SPWM_PERIOD_95PER);  //95%
        h4s_pwm_set_duty(1, H4SPWM_PERIOD_95PER);  //95%
    }
}


/******************* (C) COPYRIGHT 2019 07 29 hxdyxd *****END OF FILE****/
