/* 
 *
 *2018 09 21 & hxdyxd
 *
 */


#ifndef __data_interface_hal_H__
#define __data_interface_hal_H__

#include <stdint.h>

#include "gpio.h"
#include "quadspi.h"
#include "adc.h"
#include "dac.h"
#include "tim.h"
#include "hrtim_4switch_pwm.h"
#include "qspi_flash.h"


#define ADC_DUAL_BUFFER      (1)

/* ADC */
#define ADC_16BIT_VOLTAGE_GET(v)    ((v)*3.0/0x10000)

#define ADC1_CONV_NUMBER      (50)
#define ADC1_CHANNEL_NUMBER   (3)
#define ADC1_BUFFER_SIZE     (ADC1_CONV_NUMBER*ADC1_CHANNEL_NUMBER*ADC_DUAL_BUFFER)

#define ADC3_CONV_NUMBER      (50)
#define ADC3_CHANNEL_NUMBER   (1)
#define ADC3_BUFFER_SIZE     (ADC3_CONV_NUMBER*ADC3_CHANNEL_NUMBER*ADC_DUAL_BUFFER)


//UART
#define UART_BUFFER_SIZE     (256)
#define USART_RX_TIMEOUT_MS  (10)


/* LEDS */
#define LED_OFF(id)  HAL_GPIO_WritePin(id, GPIO_PIN_SET)
#define LED_ON(id)   HAL_GPIO_WritePin(id, GPIO_PIN_RESET)
#define LED_HIGH(id)  HAL_GPIO_WritePin(id, GPIO_PIN_SET)
#define LED_LOW(id)   HAL_GPIO_WritePin(id, GPIO_PIN_RESET)
#define LED_REV(id)  HAL_GPIO_TogglePin(id)

#define LED0_BASE   GPIOC, GPIO_PIN_13


/*******************************************************************************
* Function Name  : data_interface_hal_init.
* Description    : Hardware adaptation layer initialization.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void data_interface_hal_init(void);


/*******************************************************************************
* Function Name  : adc_rx_proc.
* Description    : Hardware adaptation layer adc initialization.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void adc_rx_proc( void (*func_cb)(int id, void *, int len) );


/*******************************************************************************
* Function Name  : hal_read_TickCounter.
* Description    : Hardware adaptation layer TICK get.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
#define hal_read_TickCounter() HAL_GetTick()


#endif
/******************* (C) COPYRIGHT 2018 hxdyxd *****END OF FILE****/
