/* 
 *
 *2019 08 04 & hxdyxd
 *
 */


#include "hal32_adc.h"
#include <string.h>

#include "app_debug.h"
#define HAL32_ADC_ERR  APP_ERROR


/*********************************ADC**************************************************/

static uint16_t ALIGN_32BYTES( adc1_dma_buffer[ADC1_BUFFER_SIZE]);
static uint8_t ALIGN_32BYTES( adc3_dma_buffer[ADC3_BUFFER_SIZE]); //8bit


static volatile uint8_t adc1_ok = 0;
static volatile uint8_t adc3_ok = 0;


void adc_rx_proc(hal32_adc_cb_t func_cb)
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
        HAL32_ADC_ERR("unknow ConvCplt\r\n");
    }
}


#if 0
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc == &hadc1) {
        if(adc1_ok) {
            printf("ovr\r\n");
        }
        adc1_ok = 1;
    } else if(hadc == &hadc3) {
        if(adc3_ok) {
            printf("ovr\r\n");
        }
        adc3_ok = 1;
    } else {
        printf("unknow ConvCplt\r\n");
    }
}
#endif


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




void hal32_adc_init(void)
{
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) {
        HAL32_ADC_ERR("adc1 calibration error\r\n");
    }
    
    if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) {
        HAL32_ADC_ERR("adc3 calibration error\r\n");
    }
    
    //temp sensor
    ADC3_COMMON->CCR |= (1<<23);
    
    
    memset( (void *)adc1_dma_buffer, 0, sizeof(adc1_dma_buffer));
    memset( (void *)adc3_dma_buffer, 0, sizeof(adc3_dma_buffer));
    
    if(HAL_ADC_Start_DMA(&hadc1, (void *)adc1_dma_buffer, ADC1_BUFFER_SIZE) == HAL_OK) {
        printf("start adc1 dma at 0x%p %p\r\n", adc1_dma_buffer, &hadc1);
    } else {
        HAL32_ADC_ERR("start adc1 dma error %d\r\n", HAL_ADC_GetError(&hadc1));
    }

    if(HAL_ADC_Start_DMA(&hadc3, (void *)adc3_dma_buffer, ADC3_BUFFER_SIZE) == HAL_OK) {
        printf("start adc3 dma at 0x%p %p\r\n", adc3_dma_buffer, &hadc3);
    } else {
        HAL32_ADC_ERR("start adc3 dma error %d\r\n", HAL_ADC_GetError(&hadc3));
    }
    
    HAL_TIM_Base_Start(&htim4);  //adc timer
}

/******************* (C) COPYRIGHT 2019 hxdyxd *****END OF FILE****/
