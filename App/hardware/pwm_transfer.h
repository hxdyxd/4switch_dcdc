#ifndef _PWM_TRANSFER_H
#define _PWM_TRANSFER_H

#include "hrtim.h"
#include "tim.h"

#define AUDIO_QUEYE_SIZE  (1000)


void pwm_device_set_rate(int rate, int chls);
void pwm_transfer_init(void);
void pwm_transfer_stop(TIM_HandleTypeDef *htim);

int pwm_transfer_start(TIM_HandleTypeDef *htim, uint32_t dstaddr, uint32_t *pData, uint16_t Length);
void pwm_transfer_next_addr_set(void *addr);

#endif
