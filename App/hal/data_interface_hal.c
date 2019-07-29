/* 
 *
 *2018 09 21 & hxdyxd
 *
 */

#include <stdio.h>
#include <string.h>

#include "data_interface_hal.h"

#include "usart.h"



/* Complement */
#define TIM_CHANNEL_1N                      (0x0002U)
#define TIM_CHANNEL_2N                      (0x0006U)


/*********************************ADC**************************************************/

uint16_t ALIGN_32BYTES( adc1_dma_buffer[ADC1_BUFFER_SIZE]);
uint16_t ALIGN_32BYTES( adc3_dma_buffer[ADC3_BUFFER_SIZE]);


static volatile uint8_t adc1_ok = 0;
static volatile uint8_t adc3_ok = 0;


void adc_rx_proc( void (*func_cb)(int id, void *pbuf, int len) )
{
    if(adc1_ok) {
        if(func_cb) {
            func_cb(1, (void *)adc1_dma_buffer, sizeof(adc1_dma_buffer));
        }
        adc1_ok = 0;
    }
    if(adc3_ok) {
        if(func_cb) {
            func_cb(3, (void *)adc3_dma_buffer, sizeof(adc3_dma_buffer));
        }
        adc3_ok = 0;
    }
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc == &hadc1) {
        SCB_InvalidateDCache_by_Addr((uint32_t *)&adc1_dma_buffer[0], sizeof(adc1_dma_buffer));
        adc1_ok = 2;
    } else if(hadc == &hadc3) {
        SCB_InvalidateDCache_by_Addr((uint32_t *)&adc3_dma_buffer[0], sizeof(adc3_dma_buffer));
        adc3_ok = 2;
    } else {
        printf("unknow ConvCplt\r\n");
    }
}


//void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
//{
//    if(hadc == &hadc1) {
//        if(adc1_ok) {
//            printf("ovr\r\n");
//        }
//        adc1_ok = 1;
//    } else if(hadc == &hadc3) {
//        if(adc3_ok) {
//            printf("ovr\r\n");
//        }
//        adc3_ok = 1;
//    } else {
//        printf("unknow ConvCplt\r\n");
//    }
//}


void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{
    printf("LevelOut\r\n");
}


void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    printf("---------------HAL_ADC_ErrorCallback----------\r\n");
    if(hadc == &hadc1) {
        adc1_ok = 1;
    } else if(hadc == &hadc3) {
        adc3_ok = 1;
    }
}

/*********************************ADC**************************************************/



void flash_init(void)
{
    qspi_flash_reset();
    qspi_flash_jedec_id();
    qspi_flash_device_id();
    
    qspi_flash_enable_qe();
    qspi_flash_memory_maped_by_qspi();
}


/* some low level platform function */
/* public hal function */

void data_interface_hal_init(void)
{
    HAL_SYSCFG_DisableVREFBUF();
    HAL_SYSCFG_VREFBUF_HighImpedanceConfig(SYSCFG_VREFBUF_HIGH_IMPEDANCE_DISABLE);
    
    flash_init();

    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) {
        Error_Handler();
    }
    
    if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) {
        Error_Handler();
    }
    
    
    ADC3_COMMON->CCR |= (1<<23);
    
    
    memset( (void *)adc1_dma_buffer, 0, sizeof(adc1_dma_buffer));
    memset( (void *)adc3_dma_buffer, 0, sizeof(adc3_dma_buffer));
    
    if(HAL_ADC_Start_DMA(&hadc1, (void *)adc1_dma_buffer, ADC1_BUFFER_SIZE) == HAL_OK) {
        APP_DEBUG("start adc1 dma at 0x%p %p\r\n", adc1_dma_buffer, &hadc1);
    } else {
        APP_ERROR("start adc1 dma error %d\r\n", HAL_ADC_GetError(&hadc1));
    }

    if(HAL_ADC_Start_DMA(&hadc3, (void *)adc3_dma_buffer, ADC3_BUFFER_SIZE) == HAL_OK) {
        APP_DEBUG("start adc3 dma at 0x%p %p\r\n", adc3_dma_buffer, &hadc3);
    } else {
        APP_ERROR("start adc3 dma error %d\r\n", HAL_ADC_GetError(&hadc3));
    }
    
    HAL_TIM_Base_Start(&htim4);  //dac timer
    HAL_TIM_Base_Start(&htim3);  //adc timer
    
    h4s_pwm_init();
}


//uint32_t random_gen(void *pbuf, uint32_t length)
//{
//    uint32_t *u32_pbuf = (uint32_t *)pbuf;
//    uint32_t len = length/4;
//    if(length&3) {
//        return 0;
//    }
//    for(int i=0; i<len; i++) {
//        HAL_RNG_GenerateRandomNumber(&hrng, &u32_pbuf[i]);
//    }
//    return length;
//}




/******************* (C) COPYRIGHT 2018 hxdyxd *****END OF FILE****/
