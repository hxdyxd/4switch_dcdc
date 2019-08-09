/* 
 *
 *2019 08 04 & hxdyxd
 *
 */


#include "hal32_adc.h"
#include <string.h>

#include "app_debug.h"
#define HAL32_ADC_DBG  APP_DEBUG
#define HAL32_ADC_ERR  APP_ERROR


/*********************************ADC**************************************************/

static uint16_t ALIGN_32BYTES( adc1_dma_buffer[ADC1_BUFFER_SIZE]);
static volatile uint8_t adc1_ok = 0;
static uint8_t cur_adc1_ch = 0;


#if ADC3_USE
    static uint8_t ALIGN_32BYTES( adc3_dma_buffer[ADC3_BUFFER_SIZE]); //8bit
    static volatile uint8_t adc3_ok = 0;
#endif

void adc_rx_proc(hal32_adc_cb_t func_cb)
{
    if(adc1_ok) {
#if ADC1_SINGLE
        if(func_cb) {
            func_cb(1, cur_adc1_ch, (void *)&adc1_dma_buffer[cur_adc1_ch*ADC1_CONV_NUMBER], ADC1_CONV_NUMBER*sizeof(adc1_dma_buffer[0]));
        }
        adc1_ok = 0;
        cur_adc1_ch = (cur_adc1_ch + 1) % ADC1_CHANNEL_NUMBER;
        adc1_channel_config(cur_adc1_ch);
        
        if(HAL_ADC_Start_DMA(&hadc1, (void *)&adc1_dma_buffer[cur_adc1_ch*ADC1_CONV_NUMBER], ADC1_CONV_NUMBER*sizeof(adc1_dma_buffer[0])) != HAL_OK) {
            HAL32_ADC_ERR("start adc1 dma error %d\r\n", HAL_ADC_GetError(&hadc1));
        }
#else
        if(func_cb) {
            func_cb(1, -1, (void *)adc1_dma_buffer, sizeof(adc1_dma_buffer));
        }
        adc1_ok = 0;
#endif
    }
#if ADC3_USE
    if(adc3_ok) {
        if(func_cb) {
            func_cb(3, -1, (void *)adc3_dma_buffer, sizeof(adc3_dma_buffer));
        }
        adc3_ok = 0;
    }
#endif
}




void adc1_channel_config(int id)
{
    static uint32_t channel[ADC1_CHANNEL_NUMBER] = {
        ADC_CHANNEL_3/*, ADC_CHANNEL_4, ADC_CHANNEL_5, */
    };
    
    hadc1.Init.NbrOfConversion = 3;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        HAL32_ADC_ERR("adc init error\n");
    }
    
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;
    
    sConfig.Channel = channel[id];
    sConfig.Rank = ADC_REGULAR_RANK_1;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        HAL32_ADC_ERR("adc init error\n");
    }
}





void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc == &hadc1) {
#if ADC1_SINGLE
        HAL_ADC_Stop_DMA(&hadc1);
#endif
        SCB_InvalidateDCache_by_Addr((uint32_t *)&adc1_dma_buffer[0], sizeof(adc1_dma_buffer));
        adc1_ok = 2;
#if ADC3_USE
    } else if(hadc == &hadc3) {
        SCB_InvalidateDCache_by_Addr((uint32_t *)&adc3_dma_buffer[0], sizeof(adc3_dma_buffer));
        adc3_ok = 2;
#endif
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
    } 
#if ADC3_USE
    else if(hadc == &hadc3) {
        adc3_ok = 1;
    }
#endif
}

/*********************************ADC**************************************************/




void hal32_adc_init(void)
{
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) {
        HAL32_ADC_ERR("adc1 calibration error\r\n");
    }
    
    
    memset( (void *)adc1_dma_buffer, 0, sizeof(adc1_dma_buffer));
    
#if ADC1_SINGLE
    adc1_channel_config(0);
    
    if(HAL_ADC_Start_DMA(&hadc1, (void *)&adc1_dma_buffer[0], ADC1_CONV_NUMBER*sizeof(adc1_dma_buffer[0])) == HAL_OK) {
        HAL32_ADC_DBG("start adc1 dma at 0x%p %p\r\n", adc1_dma_buffer, &hadc1);
    } else {
        HAL32_ADC_ERR("start adc1 dma error %d\r\n", HAL_ADC_GetError(&hadc1));
    }
#else
    
    
    if(HAL_ADC_Start_DMA(&hadc1, (void *)adc1_dma_buffer, ADC1_BUFFER_SIZE) == HAL_OK) {
        HAL32_ADC_DBG("start adc1 dma at 0x%p %p\r\n", adc1_dma_buffer, &hadc1);
    } else {
        HAL32_ADC_ERR("start adc1 dma error %d\r\n", HAL_ADC_GetError(&hadc1));
    }
#endif
    
    htim2.Init.Period = (200e6/ADC1_FREQ_SAMP);
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        APP_ERROR("timer2 init error\r\n");
    }
    
    HAL_TIM_Base_Start(&htim2);  //adc1 timer
    
    
#if ADC3_USE
    
    if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) {
        HAL32_ADC_ERR("adc3 calibration error\r\n");
    }
    
    //temp sensor
    ADC3_COMMON->CCR |= (1<<23);
    
    memset( (void *)adc3_dma_buffer, 0, sizeof(adc3_dma_buffer));
    
    if(HAL_ADC_Start_DMA(&hadc3, (void *)adc3_dma_buffer, ADC3_BUFFER_SIZE) == HAL_OK) {
        HAL32_ADC_DBG("start adc3 dma at 0x%p %p\r\n", adc3_dma_buffer, &hadc3);
    } else {
        HAL32_ADC_ERR("start adc3 dma error %d\r\n", HAL_ADC_GetError(&hadc3));
    }
    
    HAL_TIM_Base_Start(&htim8);  //adc3 timer
#endif
    
}

/******************* (C) COPYRIGHT 2019 hxdyxd *****END OF FILE****/
