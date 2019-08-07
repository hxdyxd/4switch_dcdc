#include "wave_gen.h"
#include "arm_math.h"




static volatile uint8_t gflag = 1;
#define WAVE_BUFFER_SIZE   (50)
static volatile uint16_t ALIGN_32BYTES(wave_buffer[WAVE_BUFFER_SIZE]);


void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    
}

void HAL_DAC_ConvCpltCallbackCh2(DAC_HandleTypeDef* hdac)
{
    
}

void wave_gen_init(void)
{
    
    for(int i=0; i<WAVE_BUFFER_SIZE; i++) {
        wave_buffer[i] = (arm_sin_f32(2*PI*i/WAVE_BUFFER_SIZE) * 2047/10) + 2047;
        printf("%d, ", wave_buffer[i]);
    }
    printf("\r\n");
    
    SCB_CleanDCache_by_Addr((uint32_t *)wave_buffer, sizeof(wave_buffer));
    if(HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)wave_buffer, WAVE_BUFFER_SIZE, DAC_ALIGN_12B_R) == HAL_OK) {
        gflag = 0;
        APP_DEBUG("start dac1 dma at 0x%p \r\n", wave_buffer);
    } else {
        APP_ERROR("start dac1 dma error\r\n");
    }
    
//    if(HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_2, (uint32_t *)wave_buffer, WAVE_BUFFER_SIZE, DAC_ALIGN_12B_R) == HAL_OK) {
//        gflag = 0;
//        APP_DEBUG("start dac2 dma at 0x%p \r\n", wave_buffer);
//    } else {
//        APP_ERROR("start dac2 dma error\r\n");
//    }
    
    
    htim15.Init.Period = (200e6/5e6);
    if (HAL_TIM_Base_Init(&htim15) != HAL_OK) {
        APP_ERROR("timer init error\r\n");
    }
    
    
    HAL_TIM_Base_Start(&htim15);  //dac timer
}

