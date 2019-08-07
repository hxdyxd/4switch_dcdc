/* 2019 04 10 */
/* By hxdyxd */
#include <string.h>
#include "pwm_transfer.h"

#include "app_debug.h"
#define PWM_TRANSFER_DBG  APP_DEBUG
#define PWM_TRANSFER_ERR  APP_ERROR


#define PWM_DST_REGS_ADDR  ((uint32_t)&(HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR) )
#define PWM2_DST_REGS_ADDR  ((uint32_t)&(htim4.Instance->CCR1) )

ALIGN_32BYTES(static uint16_t pwm_dma_isr_buf[AUDIO_QUEYE_SIZE]);
static uint16_t *next_dstaddr = NULL;


void pwm_transfer_next_addr_set(void *addr)
{
    next_dstaddr = addr;
}

#if 1
extern const unsigned char sgwav[];
extern const int sgwav_size;
int playptr = 0;
int16_t *sample = (int16_t *)sgwav;


void test_wave_gen(void)
{
    for(int i = 0; i < AUDIO_QUEYE_SIZE; i++) {
        pwm_dma_isr_buf[i] = ((sample[playptr] + 32768) >> 6) & 0x3ff;
        playptr += 1;
        if(playptr >= sgwav_size) {
            playptr = 0;
            printf("restart \r\n");
        }
    }
}
#endif


void pwm_transfer_init(void)
{
    HAL_HRTIM_WaveformOutputStart(&hhrtim, HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2);
    HAL_HRTIM_WaveformCountStart(&hhrtim, HRTIM_TIMERID_TIMER_C);
    
    htim4.Init.Period = (200000000/8000)-1;
    if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
    {
        PWM_TRANSFER_ERR("htim4 init error\r\n");
    }
    
    for(int i=0; i<AUDIO_QUEYE_SIZE; i++) {
        pwm_dma_isr_buf[i] = 1000 * (sin(2*3.1415926*100*i/AUDIO_QUEYE_SIZE) * 0.4 + 0.5);
        printf("%d, ", pwm_dma_isr_buf[i]);
    }
    printf("\r\n");

    
    
    test_wave_gen();
    pwm_transfer_start(&htim4, PWM_DST_REGS_ADDR, (uint32_t *)pwm_dma_isr_buf, AUDIO_QUEYE_SIZE);
    PWM_TRANSFER_DBG("pwm_transfer init reg: 0x%8x buf: 0x%8x \r\n", PWM_DST_REGS_ADDR, (uint32_t)pwm_dma_isr_buf);
}


void TIM_DMAPeriodElapsedCplt_t(DMA_HandleTypeDef *hdma)
{
    /* Disable the TIM Update DMA request */
    __HAL_TIM_DISABLE_DMA(&htim4, TIM_DMA_UPDATE);
    
    
//    if(next_dstaddr != NULL) {
//        memcpy(pwm_dma_isr_buf, next_dstaddr, AUDIO_QUEYE_SIZE*2);
//        next_dstaddr = NULL;
//    } else {
        printf("tim NULL\r\n");
//    }
    test_wave_gen();
    __HAL_TIM_ENABLE_DMA(&htim4, TIM_DMA_UPDATE);
}

#if 0
void TIM_DMAPeriodElapsedHalfCplt_t(DMA_HandleTypeDef *hdma)
{
    
}
#endif

void TIM_DMAError_t(DMA_HandleTypeDef *hdma)
{
    /* Change the htim state */
    htim3.State = HAL_TIM_STATE_READY;
    
    PWM_TRANSFER_ERR("e%d \n", hdma->ErrorCode);
}


void pwm_transfer_stop(TIM_HandleTypeDef *htim)
{
    /* Check the parameters */
    assert_param(IS_TIM_DMA_INSTANCE(htim->Instance));

    /* Disable the TIM Update DMA request */
    __HAL_TIM_DISABLE_DMA(htim, TIM_DMA_UPDATE);

    HAL_DMA_Abort_IT(htim->hdma[TIM_DMA_ID_UPDATE]);


    /* Disable the Peripheral */
    __HAL_TIM_DISABLE(htim);

    /* Change the htim state */
    htim->State = HAL_TIM_STATE_READY;
}



/**
  * @brief  Starts the TIM Base generation in DMA mode.
  * @param  htim TIM Base handle
  * @param  pData The source Buffer address.
  * @param  Length The length of data to be transferred from memory to peripheral.
  * @retval HAL status
  */
int pwm_transfer_start(TIM_HandleTypeDef *htim, uint32_t dstaddr, uint32_t *pData, uint16_t Length)
{
    uint32_t tmpsmcr;

    /* Check the parameters */
    assert_param(IS_TIM_DMA_INSTANCE(htim->Instance));

    if (htim->State == HAL_TIM_STATE_BUSY) {
        PWM_TRANSFER_ERR("busy error\r\n");
        return -1;
    } else if (htim->State == HAL_TIM_STATE_READY) {
        if ((pData == NULL) && (Length > 0U)) {
            PWM_TRANSFER_ERR("buffer error\r\n");
            return -1;
        } else {
            htim->State = HAL_TIM_STATE_BUSY;
        }
    }

    /* Set the DMA Period elapsed callbacks TIM_DMA_ID_UPDATE */
    htim->hdma[TIM_DMA_ID_UPDATE]->XferCpltCallback = TIM_DMAPeriodElapsedCplt_t;
//    htim->hdma[TIM_DMA_ID_UPDATE]->XferHalfCpltCallback = TIM_DMAPeriodElapsedHalfCplt_t;

    /* Set the DMA error callback */
    htim->hdma[TIM_DMA_ID_UPDATE]->XferErrorCallback = TIM_DMAError_t ;

    /* Enable the DMA stream */
    if (HAL_DMA_Start_IT(htim->hdma[TIM_DMA_ID_UPDATE], (uint32_t)pData, dstaddr, Length) != HAL_OK) {
        PWM_TRANSFER_ERR("start IT error\r\n");
        return -1;
    }
    
    /* Enable the TIM Update DMA request */
    __HAL_TIM_ENABLE_DMA(htim, TIM_DMA_UPDATE);
    
    
    /* Enable the Peripheral, except in trigger mode where enable is automatically done with trigger */
    tmpsmcr = htim->Instance->SMCR & TIM_SMCR_SMS;
    if (!IS_TIM_SLAVEMODE_TRIGGER_ENABLED(tmpsmcr))
    {
        PWM_TRANSFER_DBG("tim init ok\r\n");
        __HAL_TIM_ENABLE(htim);
    }

    /* Return function status */
    return 0;
}



/*****************************END OF FILE***************************/
