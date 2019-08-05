/* 
 *
 *2019 08 04 & hxdyxd
 *
 */

#ifndef _HAL32_ADC_H
#define _HAL32_ADC_H

#include "adc.h"
#include "tim.h"



#define ADC_DUAL_BUFFER      (1)

/* ADC */
#define ADC_16BIT_VOLTAGE_GET(v)    ((v)*3.0/0x10000)

#define ADC1_CONV_NUMBER      (25)
#define ADC1_CHANNEL_NUMBER   (5)
#define ADC1_BUFFER_SIZE     (ADC1_CONV_NUMBER*ADC1_CHANNEL_NUMBER*ADC_DUAL_BUFFER)

#define ADC3_CONV_NUMBER      (1000)
#define ADC3_CHANNEL_NUMBER   (1)
#define ADC3_BUFFER_SIZE     (ADC3_CONV_NUMBER*ADC3_CHANNEL_NUMBER*ADC_DUAL_BUFFER)


typedef void (*hal32_adc_cb_t)(int id, void *pbuf, int len);

void hal32_adc_init(void);
void adc_rx_proc(hal32_adc_cb_t func_cb);

#endif
/******************* (C) COPYRIGHT 2019 hxdyxd *****END OF FILE****/
