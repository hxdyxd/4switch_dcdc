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
#include "spi.h"
#include "hrtim_4switch_pwm.h"
#include "qspi_flash.h"
#include "hal32_adc.h"
#include "lcd240x240.h"


//UART
#define UART_BUFFER_SIZE     (256)
#define USART_RX_TIMEOUT_MS  (10)


/* LEDS */
#define LED_OFF(id)  HAL_GPIO_WritePin(id, GPIO_PIN_SET)
#define LED_ON(id)   HAL_GPIO_WritePin(id, GPIO_PIN_RESET)
#define LED_HIGH(id)  HAL_GPIO_WritePin(id, GPIO_PIN_SET)
#define LED_LOW(id)   HAL_GPIO_WritePin(id, GPIO_PIN_RESET)
#define LED_REV(id)  HAL_GPIO_TogglePin(id)

#define LED0_BASE   BASE_LED_GPIO_Port , BASE_LED_Pin
#define LCD_DC      LCD_DC_GPIO_Port   , LCD_DC_Pin
#define LCD_RES     LCD_RES_GPIO_Port  , LCD_RES_Pin
#define LCD_CLK     GPIOB  , GPIO_PIN_10
#define LCD_MOSI    GPIOC  , GPIO_PIN_1

/*******************************************************************************
* Function Name  : data_interface_hal_init.
* Description    : Hardware adaptation layer initialization.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void data_interface_hal_init(void);


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
