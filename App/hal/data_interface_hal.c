/* 
 *
 *2018 09 21 & hxdyxd
 *
 */

#include <stdio.h>
#include <string.h>

#include "data_interface_hal.h"



/* Complement */
#define TIM_CHANNEL_1N                      (0x0002U)
#define TIM_CHANNEL_2N                      (0x0006U)




static void flash_init(void)
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

    hal32_adc_init();
    
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Base_Start(&htim15);  //dac timer
    
    h4s_pwm_init();
    
    lcd240x240_init();
    lcd240x240_clear(0xffff);
    
    key_inout_init();
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
