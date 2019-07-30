/* 2019 04 10 */
/* By hxdyxd */

#include <string.h>
#include <stdlib.h>
#include "app_debug.h"
#include "user_main.h"
#include "function_task.h"
#include "data_interface_hal.h"
#include "tts.h"
#include <arm_math.h>
#include "adc_algorithm.h"
#include "soft_timer.h"


uint16_t adc_val[ADC1_CHANNEL_NUMBER];
float adc_real_value_array[ADC1_CHANNEL_NUMBER];

static uint16_t maxadc_v[ADC1_CHANNEL_NUMBER] = {0, };
static uint16_t minadc_v[ADC1_CHANNEL_NUMBER] = {0xffff, };

static pidc_t pid_ch1 = {
    .kp = 25,
    .ki = 0,
    .kd = 10,
};

static uint8_t u8_mode_set = MODE_CH1;
static uint8_t low_vin = 0;

static float vout_set_val = 16.0;

static int ctrl_counter = 0;

static int16_t s16_cur_duty_max = 0;
static int16_t s16_cur_duty_min = MAX_OUTPUT_DUTY;



void user_system_setup(void)
{

}


#define  TTS_ON      (1)

static char tts_buf[512];

void tts_player(void)
{
    SWITCH_TASK_INIT(task1);
    
    
#if TTS_ON
    SWITCH_TASK(task1) {
        int len = 0;
        if(low_vin) {
            len = snprintf(tts_buf, 512, "输入欠压  ");
        } else {
            len = snprintf(tts_buf, 512, "输入电压%.3f伏 输出电压%.3f伏  ", GET_VIN(), GET_VOUT());
        }
        
        int ret = tts_puts(tts_buf, len);
        if(ret > 0) {
            APP_WARN("TTS OUT: %s\r\n", tts_buf);
        }
    }
#endif
    
    SWITCH_TASK_END(task1);
}


void low_vin_timeout_proc(void)
{
    static float vout_set_step = 0;
    
    if(!low_vin)
        return;
    
    if(GET_VIN() >= 3.0) {
        low_vin++;
        vout_set_step += (vout_set_val - 0)/10;
        pid_set_value(&pid_ch1, vout_set_step);
        if(vout_set_step < vout_set_val) {
            APP_WARN("set vout = %.3f\r\n", vout_set_step);
            soft_timer_create(SOFT_TIMER_LOW_VIN_ID, 1, 0, low_vin_timeout_proc, 200);
        } else {
            APP_WARN("set vout = %.3f, low vin exit!\r\n", vout_set_step);
            low_vin = 0;
        }
    } else {
        vout_set_step = 0;
        APP_WARN("low vin wait\r\n");
        soft_timer_create(SOFT_TIMER_LOW_VIN_ID, 1, 0, low_vin_timeout_proc, 200);
    }
}


void pid_ch1_control_proc(void)
{
    float vvvvv = GET_VOUT();
    int16_t ctrl_duty = pid_ctrl(&pid_ch1, vvvvv );
    
    //欠压保护
    if(GET_VIN() < 2.0) {
        if(!low_vin) {
            low_vin = 1;
            soft_timer_create(SOFT_TIMER_LOW_VIN_ID, 1, 0, low_vin_timeout_proc, 200);
        }
        ctrl_duty = pid_ch1.output = H4SPWM_PERIOD_95PER;
    }
    
    
    h4s_buck_boost_pwm_set_duty(ctrl_duty);
    
     //debug 1s duty value
    if(ctrl_duty > s16_cur_duty_max)
        s16_cur_duty_max = ctrl_duty;
    if(ctrl_duty < s16_cur_duty_min)
        s16_cur_duty_min = ctrl_duty;
}



void adc3_receive_proc(int id, void *pbuf, int len)
{
    uint16_t *adc_data = (uint16_t *)pbuf;
    
    
    if(id == 1) {
        TIMER_TASK(timer0, 3, 1) {
            for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
                adc_val[i] = No_Max_Min_Filter(adc_data, ADC1_CONV_NUMBER, ADC1_CHANNEL_NUMBER, i);
                if(adc_val[i] > maxadc_v[i]) {
                    maxadc_v[i] = adc_val[i];
                }
                if(adc_val[i] < minadc_v[i]) {
                    minadc_v[i] = adc_val[i];
                }
                adc_real_value_array[i] = value_adc_physical_get( ADC_16BIT_VOLTAGE_GET(adc_val[i]), i);
            }
            
            pid_ch1_control_proc();
            ctrl_counter++;
            
            TIMER_TASK(timer1, 1000, 1) {
                for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
                    PRINTF("[%s] = %d(dt:%d) %.3f V\r\n", value_adc_info(i), adc_val[i], maxadc_v[i] - minadc_v[i], ADC_16BIT_VOLTAGE_GET(adc_val[i]) );
                    
                    PRINTF("%s: %.3fV \r\n", value_adc_info(i), adc_real_value_array[i]);

                    maxadc_v[i] = 0;
                    minadc_v[i] = 0xffff;
                    //mat();
                }
                APP_DEBUG(" ctl= %d cnt/s, duty= (%5d)%.1f%% - (%5d)%.1f%% \r\n",
                    ctrl_counter,
                    s16_cur_duty_min, 
                    s16_cur_duty_min*100.0/H4SPWM_PERIOD_185PER,
                    s16_cur_duty_max,
                    s16_cur_duty_max*100.0/H4SPWM_PERIOD_185PER
                );
                ctrl_counter = 0;
                s16_cur_duty_max = 0;
                s16_cur_duty_min = MAX_OUTPUT_DUTY;
            }
        }
    }
    
    if(id == 3) {
        TIMER_TASK(timer2, 1000, 1) {

            float Vtemp_sensor;
            uint16_t TS_CAL1;
            uint16_t TS_CAL2;

            uint32_t   uwConvertedValue;

            TS_CAL1 = *(__IO uint16_t *)(0x1FF1E820);
            TS_CAL2 = *(__IO uint16_t *)(0x1FF1E840);

            uwConvertedValue = adc_data[0];  /* 读取数值 */
            Vtemp_sensor = uwConvertedValue;
            Vtemp_sensor = (110.0 - 30.0) * Vtemp_sensor/ (TS_CAL2 - TS_CAL1) + (110.0 * TS_CAL1 - 30.0 * TS_CAL2)/(TS_CAL1 - TS_CAL2);   /* 转换 */
            APP_DEBUG("TS_CAL1(30C) = %d, TS_CAL2(110C) = %d, cpu temp:  (%d) %.2f C \r\n", TS_CAL1, TS_CAL2, uwConvertedValue, Vtemp_sensor);
            
    #if TTS_ON && 0
            int len = snprintf(tts_buf, 512, "当前温度%.2f  ", Vtemp_sensor);
            int ret = tts_puts(tts_buf, len);
            if(ret > 0) {
                APP_WARN("TTS OUT: %s\r\n", tts_buf);
            }
    #endif

        }
    }
}


void led_debug_proc(void)
{
    LED_REV(LED0_BASE);
    tts_player();
}


void user_setup(void)
{
    PRINTF("\r\n\r\n[H7] Build , %s %s \r\n", __DATE__, __TIME__);
    
    data_interface_hal_init();
    
    param_default_value_init();
    
    pid_set_output_limit(&pid_ch1, MAX_OUTPUT_DUTY, 0);
    pid_set_value(&pid_ch1, vout_set_val);
    
    soft_timer_init();
    
    soft_timer_create(0, 1, 1, led_debug_proc,200);

#if TTS_ON
    tts_init();
#endif
}




void user_loop(void)
{
    soft_timer_proc();

#if TTS_ON
    TIMER_TASK(time3, 50, 1) {
        tts_proc();
    }
#endif

    adc_rx_proc(adc3_receive_proc);
}


/*****************************END OF FILE***************************/
