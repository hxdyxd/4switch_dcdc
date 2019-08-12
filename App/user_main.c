/* 2019 04 10 */
/* By hxdyxd */

#include <string.h>
#include <stdlib.h>
#include "app_debug.h"
#include "user_main.h"
#include "function_task.h"
#include "data_interface_hal.h"


#include "adc_algorithm.h"
#include "soft_timer.h"
#include "lcd_manager.h"
#include "AD9833.h"
#include "math.h"
#include "tts.h"
#include "eeprom.h"

void user_system_setup(void)
{

}


static uint8_t buffer_wave_1[240];
static uint8_t buffer_wave_2[240];
static uint8_t buffer_wave_3[240];
static struct lcd_wave_t gwav1, gwav2, gwav3;
static struct lcd_wave_t *gwavs[3] = {
    &gwav1, &gwav2, &gwav3, 
};
static uint8_t wave_on = 0;

//40k - 160k
static float freq_table_40k[ADC1_CHANNEL_NUMBER][2] = {
    {1, 0, },
     {0.4340 ,   5.1136},
    { 1.2484 , -184.6287},
};
static float freq_table_160k[ADC1_CHANNEL_NUMBER][2] = {
    {1, 0, },
    { 0.0810,  4.9054,},
    {  1.0e+03 * -0.0013 ,  1.0e+03 * 1.1748,}, 
};


float freq_gain_table_cal(float freq, float in, float out)
{
    float freq_in_base_amp = 0;
    float freq_out_base_amp = 0;
    if(freq > 160e3) {
        freq_in_base_amp = freq_table_160k[CHANNEL_IN2][0] * in + freq_table_160k[CHANNEL_IN2][1];
        freq_out_base_amp = freq_table_160k[CHANNEL_OUT][0] * out + freq_table_160k[CHANNEL_OUT][1];
    } else if(freq > 15e3) {
        freq_in_base_amp = freq_table_40k[CHANNEL_IN2][0] * in + freq_table_40k[CHANNEL_IN2][1];
        freq_out_base_amp = freq_table_40k[CHANNEL_OUT][0] * out + freq_table_40k[CHANNEL_OUT][1];
    } else {
        freq_in_base_amp = in;
        freq_out_base_amp = out;
    }
    return freq_out_base_amp/freq_in_base_amp;
}



uint8_t dds_output_amp = 100;
int dds_output_freq = 1000;



float abs_outputbuf[FFT_LENGTH];

uint16_t dma_buf_div[ADC1_CHANNEL_NUMBER][FFT_LENGTH];
uint16_t dma_buf_tmp[FFT_LENGTH];

float gs_base_freq_hz[ADC1_CHANNEL_NUMBER] = {0};
float gs_base_amp[ADC1_CHANNEL_NUMBER] = {0};
float gs_zero_val[ADC1_CHANNEL_NUMBER]= {0};


#define  BASE_AMP_AVG_NUMBER    (10)
float gs_base_amp_avg[ADC1_CHANNEL_NUMBER] = {0};

static float value_base_amp[ADC1_CHANNEL_NUMBER + 1] = {0};
static float value_base_amp_fast[ADC1_CHANNEL_NUMBER] = {0};




float rin = 0;
float outgain = 0;
float outgain_fast = 0;


int gs_vin_dc = 0;
int gs_vout_dc = 0;

uint32_t fft_cnt = 0;
uint32_t vdc_cnt = 0;

void adc3_receive_proc(int id, int channel, void *pbuf, int len)
{
    uint16_t *adc_data = (uint16_t *)pbuf;
    //return;
    
    
    if(id == 1) {
        static int flag = 0;
        fft_cnt++;
        flag++;
        if(flag >= 244) {
            flag = 0;
            printf("adc 1s \r\n");
        }
        
        for(int i=0; i<FFT_LENGTH; i++) {
            for(int j=0; j<ADC1_CHANNEL_NUMBER; j++) {
                dma_buf_div[j][i] = adc_data[i*ADC1_CHANNEL_NUMBER + j];
            }
        }
        
#if 0
        TIMER_TASK(timer1, 1000, 1) {

            //用于MATLAB绘制频域图像
            for(int i= 0;i< FFT_LENGTH ;i++) {
                
                printf("%d ",  dma_buf_div[0][i] );
            }
            printf("\r\n");

        }
#endif
        for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
            //FFT及基波幅度分析
            fft_fast_real_u16_to_float(dma_buf_div[i], abs_outputbuf);    //实数FFT运算
            gs_zero_val[i] = abs_outputbuf[0]/FFT_LENGTH;               //直流分量
            
            fft_hann_get(dma_buf_tmp, dma_buf_div[i], gs_zero_val[i]);         //hann window
            
            fft_fast_real_u16_to_float(dma_buf_tmp, abs_outputbuf);    //实数FFT运算,with hann window
            
            uint32_t base_freq = find_fft_max_freq_index(abs_outputbuf, FFT_LENGTH/2);   //找出幅度最大值，前面一半数据有效
            gs_base_freq_hz[i]  = FFT_INDEX_TO_FREQ(base_freq, ADC1_FREQ_SAMP);   //转化为真实频率
            gs_base_amp[i] = ADC_12BIT_VOLTAGE_GET(FFT_ASSI(abs_outputbuf[base_freq]));
            
            value_base_amp_fast[i] = get_param_value(gs_base_amp[i], i);
        }
        
        
        outgain_fast = freq_gain_table_cal(gs_base_freq_hz[1], value_base_amp_fast[CHANNEL_IN2], value_base_amp_fast[CHANNEL_OUT]);
        if(UABS(gs_vin_dc - gs_vout_dc) < 500) {
            float out_a = gs_base_amp[CHANNEL_OUT]*2.33/3.5;
            float in_a = gs_base_amp[CHANNEL_IN2]*2.88/24.6;
            outgain_fast = out_a/in_a;
        }
        
        
        {
            static float gs_base_amp_near[ADC1_CHANNEL_NUMBER][BASE_AMP_AVG_NUMBER] = {0};
            static uint32_t gs_base_amp_avg_cnt = 0;
            for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
                gs_base_amp_near[i][gs_base_amp_avg_cnt] = gs_base_amp[i];
                
                //no max min filter
                gs_base_amp_avg[i] = no_max_min_filter_float(gs_base_amp_near[i], BASE_AMP_AVG_NUMBER);
                
                //get real value
                value_base_amp[i] = get_param_value(gs_base_amp_avg[i], i);
                if(i == CHANNEL_OUT) {
                    value_base_amp[CHANNEL_OUT_LOAD] = get_param_value(gs_base_amp_avg[i], i+1);
                }
                
                //get input resister
                rin = 2000*(value_base_amp[1])/(value_base_amp[0] - value_base_amp[1]);
                
                //fix freq
                outgain = freq_gain_table_cal(gs_base_freq_hz[1], value_base_amp[CHANNEL_IN2], value_base_amp[CHANNEL_OUT]);
                if(UABS(gs_vin_dc - gs_vout_dc) < 500) {
                    float out_a = gs_base_amp_avg[CHANNEL_OUT]*2.33/3.5;
                    float in_a = gs_base_amp_avg[CHANNEL_IN2]*2.88/24.6;
                    outgain = out_a/in_a;
                }
                
            }
            gs_base_amp_avg_cnt = (gs_base_amp_avg_cnt + 1) % BASE_AMP_AVG_NUMBER;
        }
        
    }
    
    if(id == 3) {
        vdc_cnt++;
        static int vdc_cnt_last = 0;
        gs_vin_dc = no_max_min_filter_uint16_mult(adc_data, ADC3_CONV_NUMBER, ADC3_CHANNEL_NUMBER, 0);
        gs_vin_dc = ADC_16BIT_VOLTAGE_GET(gs_vin_dc)*1000*4.3;
        
        
        
        printf("gs_vin_dc: %d  inc: %d\r\n", gs_vin_dc, gs_vin_dc - vdc_cnt_last);
        vdc_cnt_last = gs_vin_dc;
        
        gs_vout_dc = no_max_min_filter_uint16_mult(adc_data, ADC3_CONV_NUMBER, ADC3_CHANNEL_NUMBER, 1);
        gs_vout_dc = ADC_16BIT_VOLTAGE_GET(gs_vout_dc)*1000*4.3;
    }
}


#define F_START_FREQ (200)
#define F_END_FREQ   ((int)ADC1_FREQ_SAMP/2)
#define F_COUNTER    (240)


#define FS_STOP         (0)
#define FS_START        (1)
#define FS_START_SCAN   (2)
#define FS_NOT_DETECT   (3)
#define FS_DETECTED     (4)



uint8_t freq_scan_status = FS_STOP;
static float cur_freq_set = 0;
static int freq_point = 0;
static uint32_t real_freq = 0;
static float up_freq = 0;
static float lp_freq = 0;
static uint8_t up_freq_find = 0;
static uint8_t lp_freq_find = 0;
static float gain_1k = 0;

static float outgain_fast_last = 0;

void freq_scan_proc(void)
{
   
    switch(freq_scan_status) {
    case FS_START:
        //记录初始增益
        up_freq_find = 0;
        lp_freq_find = 0;
        setWave(SINE, 1000);
        for(int i=0; i<240; i++) {
            gui_wave_set(&gwav1, 0, C_GREEN);
            gui_wave_set(&gwav2, 0, C_BLUE);
            gui_wave_set(&gwav3, 0, C_ORANGE);
        }
        soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 300);
        freq_scan_status = FS_START_SCAN;
    case FS_START_SCAN:
        //使用精确增益
        gain_1k = outgain / 1.41421;
        APP_DEBUG("start freq scan, set 1k, gain = %.1f\r\n", gain_1k);
        
        //开始扫频
        freq_point = 0;
        cur_freq_set = log10(F_START_FREQ);
        real_freq = pow(10, cur_freq_set);
        //cur_freq_set = F_START_FREQ;
        //real_freq = cur_freq_set;
        setWave(SINE, real_freq);
        soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 300);
        APP_DEBUG("start freq scan, cur_freq_set = %dHz\r\n", real_freq);
        lcd_printf("[%d] freq = %dHz\n", freq_point, real_freq);
        freq_scan_status = FS_NOT_DETECT;
        break;
    case FS_NOT_DETECT:
        //扫频中
    {
        float scan_gain = outgain_fast;
        if(fabs(scan_gain - outgain_fast_last) > 5) {
            scan_gain = outgain_fast_last + ((scan_gain - outgain_fast_last > 0)?5:-5);
        }
        
        outgain_fast_last = scan_gain;
        gui_wave_set(&gwav1, EASY_LR( ((up_freq_find == 0)&&(lp_freq_find == 1))?(gain_1k):(0), 0, 0, 300, 120.0), C_GREEN);
        gui_wave_set(&gwav2, EASY_LR(gs_base_freq_hz[CHANNEL_IN2], F_START_FREQ, 0, F_END_FREQ, 120.0), C_BLUE);
        gui_wave_set(&gwav3, EASY_LR(scan_gain, 0, 0, 300, 120.0), C_ORANGE);
        
        freq_point++;
        if(freq_point >= F_COUNTER || cur_freq_set >= log10(F_END_FREQ)) {
            //扫频结束
            APP_DEBUG("[%d] success freq scan, %d\r\n", freq_point, real_freq);
            lcd_printf("[%d] scan success, %d\n", freq_point, real_freq);
            freq_scan_status = FS_STOP;
            setWave(SINE, 1000);
            return;
        }
        
        if(scan_gain <= gain_1k && real_freq > 10000) {
            up_freq_find = 1;
            tts_printf("上限频率%.1f千赫兹", (up_freq/1000));
        } else if(scan_gain >= gain_1k) {
            lp_freq_find = 1;
        }
        if(!up_freq_find) {
            up_freq = real_freq;
        }
        if(!lp_freq_find) {
            lp_freq = real_freq;
        }
        cur_freq_set += (log10(F_END_FREQ) - log10(F_START_FREQ))/F_COUNTER;
        real_freq = pow(10, cur_freq_set);
        //cur_freq_set += (F_END_FREQ - F_START_FREQ)*1.0/F_COUNTER;
        //real_freq = cur_freq_set;
        setWave(SINE, real_freq);
        soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 50);
        APP_DEBUG("[%d] run freq scan, cur_freq_set = %dHz, GAIN = %.1f\r\n", freq_point, real_freq, scan_gain);
        lcd_printf("[%d] freq = %dHz\n", freq_point, real_freq);
        break;
    }
    default:
        APP_DEBUG("default ?\r\n");
        break;
    }
    //gui_wave_set(&gwav1, EASY_LR(real_freq, 0, 0, F_END_FREQ, 120.0), C_MAGENTA);
}


#define FD_NOT_RUN   (255)
#define FD_START     (0)
#define FD_RIN       (1)
#define FD_UP_START  (2)
#define FD_UP        (3)


#define R1_OC    (1)
#define R1_SC    (2)
#define R2_OC    (3)
#define R2_SC    (4)
#define R3_OC    (5)
#define R3_SC    (6)
#define R4_OC    (7)
#define R4_SC    (8)

#define C1_OC    (9)
#define C2_OC    (10)
#define C3_OC    (11)

#define C1_SC    (12)
#define C2_SC    (13)
#define C3_SC    (14)

const char *fault_string[15] = {
    "Normal",
    
    "R1 open circuit", //Rin > 4K
    "R1 short circuit", //Rin > 4K
    "R2 open circuit",  //Rin < 500
    "R2 short circuit",  //Rin < 500
    
    "R3 open circuit",  //Rin < 500
    "R3 short circuit", //Rin < 500
    "R4 open circuit",
    "R4 short circuit",
    
    "C1 open circuit",
    "C2 open circuit",
    "C3 open circuit",
    
    "C1 Increase",
    "C2 Increase",
    "C3 Increase",
};

const char *fault_tts_string[15] = {
    "1",
    
    "电阻1开路",
    "电阻1短路",
    "电阻2开路",
    "电阻2短路",
    
    "电阻3开路",
    "电阻3短路",
    "电阻4开路",
    "电阻4短路",
    
    "电容1开路",
    "电容2开路",
    "电容3开路",
    
    "电容1增大",
    "电容2增大",
    "电容3增大",
};


static uint8_t fault_id_temp_last = 0;
static uint8_t fault_id_temp = 0;

static uint8_t fault_id = 0;

static uint8_t fault_static_point = 0;

static float mp_gain = 0;
static float up_gain = 0;
static float lp_gain = 0;
//
static uint8_t up_fault_counter = 0;
static uint8_t lp_fault_counter = 0;
static float rin_1k = 0;

static int vidc_last = 0;
static int vidc_max = 0;
static int vidc_inc = 0;

static int fault_vout_para_val[] = {
    7000, //gs_vout_dc
    3600, //rin
    0, //up_gain
    0, //lp_gain
};


void fault_detection_static(void)
{
    rin_1k = rin;
    if(gs_vout_dc >= 7000) {
        if(rin >= 12000) {
            fault_id_temp = R1_OC;
        } else if(rin >= 4000) {
            fault_id_temp = R4_OC;
        } else if(rin >= 1000) {
            fault_id_temp = R3_SC;
        } else {
            if(gs_vout_dc > 8300) {
                fault_id_temp = R1_SC;
            } else {
                fault_id_temp = R2_SC;
            }
        }
    } else if(gs_vout_dc <= 3600 && gs_vout_dc >= 1000 && rin <= 600 && rin > 0) {
        fault_id_temp = R2_OC;
    } else if(gs_vout_dc <= 500 && rin <= 600 && rin > 0) {
        if( rin > 140) {
            fault_id_temp = R3_OC;
        } else {
            fault_id_temp = R4_SC;
        }
    } else if(gs_vout_dc >= 3000 && gs_vout_dc <= 7000 && (rin > 12000 || rin < 0)) {
        fault_id_temp = C1_OC;
    } else if(gs_vout_dc >= 3000 && gs_vout_dc <= 7000 && rin >= 4000 && rin <= 12000) {
        fault_id_temp = C2_OC;
    } else {
        fault_id_temp = 0;
    }
    
    
    if(fault_id_temp == fault_id_temp_last) {
        fault_id = fault_id_temp;
    }
    fault_id_temp_last = fault_id_temp;
    if(fault_id == 0) {
        fault_static_point = 0;
    } else {
        fault_static_point = 1;
    }
    
    vidc_inc = gs_vin_dc - vidc_last;
    if(vidc_inc > 300  && vidc_inc < 1200 && vidc_last > 500 && !fault_static_point && !fault_id_temp) {
        vidc_max = gs_vin_dc;
        fault_id = C1_SC;
    }
    vidc_last = gs_vin_dc;
}


void fault_detection(void)
{
    SWITCH_TASK_INIT(sfault);
    
    
    SWITCH_TASK(sfault) {
        APP_DEBUG("freq = %.1f, gain %.1f\r\n", gs_base_freq_hz[2], outgain);
        mp_gain = outgain;
        fault_detection_static();
        if(!fault_static_point && gs_vout_dc >= 3600 && gs_vout_dc <= 7000) {
            setWave(SINE, 140000);
        }
        
    }
    

    SWITCH_TASK(sfault) {
        
        if(!fault_static_point && gs_vout_dc >= 3600 && gs_vout_dc <= 7000) {
            APP_DEBUG("freq = %.1f, gain %.1f\r\n", gs_base_freq_hz[2], outgain);
            up_gain = outgain;
            
            //HF
            if(up_gain > 130) {
                
                up_fault_counter++;
                if(up_fault_counter >= 2) {
                    up_fault_counter = 0;
                    fault_id = C3_OC;
                }
            } else if(up_gain < 95) {
                up_fault_counter++;
                
                if(up_fault_counter >= 2) {
                    up_fault_counter = 0;
                    fault_id = C3_SC;
                }
            } else {
                up_fault_counter = 0;
            }
        }
        
        
        setWave(SINE, 1000);
    }

    
    #if 1
    SWITCH_TASK(sfault) {
        APP_DEBUG("freq = %.1f, gain %.1f\r\n", gs_base_freq_hz[2], outgain);
        mp_gain = outgain;
        fault_detection_static();
        if(!fault_static_point && gs_vout_dc >= 3600 && gs_vout_dc <= 7000) {
            setWave(SINE, 210);
        }
        
    }
    
    
    SWITCH_TASK(sfault) {
        
        if(!fault_static_point && gs_vout_dc >= 3600 && gs_vout_dc <= 7000) {
            APP_DEBUG("freq = %.1f, gain %.1f\r\n", gs_base_freq_hz[2], outgain);
            lp_gain = outgain;
            
            //LF
            
            if(lp_gain > 105) {
                
                lp_fault_counter++;
                
                if(lp_fault_counter >= 2) {
                    lp_fault_counter = 0;
                    fault_id = C2_SC;
                }
            } else {
                lp_fault_counter = 0;
            }
            
        }
        
        setWave(SINE, 1000);
    }
    #endif
    
    
    SWITCH_TASK_END(sfault);
    
    
    if(fault_id != 0) {
        tts_printf("%s", fault_tts_string[fault_id]);
    }
}




uint32_t fps_inc = 0;

static uint8_t or_status = 0;
static float rout = 0;


/***********/
uint8_t gs_lcd_mode = LCD_MODE_DEFAULT0;
uint8_t gs_lcd_mode_fault = LCD_MODE_FAULT_SHOW;

/**cal**/
uint8_t gs_lcd_cal_index = 0;
uint8_t gs_lcd_cal_ch = 0;
//float gs_para_y_div[PARA_CHANNEL_NUMBER][PARA_NUM] = {
//    {20.5, 18.5, 17.5, 15.5, 14.5, 12.5, 10.5, 8.5, 5.5, 3.5}, //VIN1
//    {11.5, 10.5, 9.5,  8.5,   7.5,  6.5,  5.5, 4.5, 3.5, 2.5}, //VIN2
//    {2005, 1805, 1705, 1505, 1405, 1205, 1005, 805, 505, 305},  //VOUT
//    {2005, 1805, 1705, 1505, 1405, 1205, 1005, 805, 505, 305},  //VOUT_LOAD
//};
float gs_para_y_div[PARA_NUM] = {0};
float gs_para_x_div[PARA_NUM] = {0};



void led_debug_proc(void)
{
    
    LED_REV(LED0_BASE);
    
    
    INIT_TASK(init_tts) {
        tts_printf("欢迎使用电路特性测试仪");
    }
    
    /* encoder */
    short encoder_cnt = ENCODER_CNT;
    ENCODER_CNT = 0;
    
    if(encoder_cnt != 0) {
        APP_DEBUG("ENCODER_CNT = %d \r\n", encoder_cnt);
        lcd_printf("ENCODER_CNT = %d \r\n", encoder_cnt);
        float point = encoder_cnt * 0.25f;
        /* --> */
        if(dds_output_freq >= 10000) {
             dds_output_freq += (int)point*1000;
        } else if(dds_output_freq >= 1000) {
            dds_output_freq += (int)point*100;
        } else if(dds_output_freq >= 100) {
            dds_output_freq += (int)point*10;
        } else {
            dds_output_freq += (int)point;
        }
        if(dds_output_freq < 0.0) {
            dds_output_freq = 0;
        }
        if(dds_output_freq > 1.0e6) {
            dds_output_freq = 1.0e6;
        }
        
        setWave(SINE, dds_output_freq);
        lcd_printf("SET DDS FREQ = %d\n", dds_output_freq);
    }
    /***********************************************/
    TIMER_TASK(timer3, 1000, 1) {
        for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
            APP_DEBUG( "[%d] zero_val = (%.1f) %.1fmV \r\n", i, gs_zero_val[i], ADC_12BIT_VOLTAGE_GET(gs_zero_val[i])*1000 );
            APP_DEBUG("[%d] base_freq_hz = %.1f, vpp = %.1fmV\r\n", i, gs_base_freq_hz[i], gs_base_amp[i]*1000);
            
            APP_DEBUG("real: %.3fmV \r\n", value_base_amp[i]);
        }
        
        APP_DEBUG("RI: %.1f\r\n", rin );
        APP_DEBUG("GAIN: %.1f %.1f\r\n", outgain , outgain_fast);
        
        
        static uint32_t last_timer = 0;
        APP_DEBUG("FFT CNT: %d\r\n", fft_cnt*1000/(hal_read_TickCounter() - last_timer) );
        APP_DEBUG("VDC CNT: %d\r\n", vdc_cnt*1000/(hal_read_TickCounter() - last_timer) );
        vdc_cnt = 0;
        fft_cnt = 0;
        last_timer = hal_read_TickCounter();
    }
    
    TIMER_TASK(timer0, 500, (lcd_console_enable && !wave_on)) {
        lcd_printf("ZERO = %.1f\n", ADC_12BIT_VOLTAGE_GET(gs_zero_val[0]) );
        lcd_printf("DDS F %d, A %d\n", dds_output_freq, dds_output_amp);
        TIMER_TASK(timer0, 1000, 1) {
            static uint32_t last_timer = 0;
            APP_DEBUG("fps: %d\r\n", fps_inc*1000/(hal_read_TickCounter() - last_timer) );
            fps_inc = 0;
            last_timer = hal_read_TickCounter();
        }
    }
    
    TIMER_TASK(timer1, 250, (!wave_on && LCD_MODE_FAULT == gs_lcd_mode)) {
        fault_detection();
    }
    
    TIMER_TASK(timer2, 500, (!lcd_console_enable && !wave_on)) {
        UG_FillFrame(0, 0, 240-1, 240-1, C_GRAY);
        UG_FontSelect ( &FONT_12X20 );
        UG_SetBackcolor(C_GRAY);
        switch(gs_lcd_mode) {
        case LCD_MODE_DEFAULT0:
            ug_printf(0, 0 + 14, C_YELLOW, "Ri: %d", (int)rin);
            if(or_status == 1) {
                ug_printf(0, 0 + 14 + 20, C_YELLOW, "Ro: !Wait load");
            } else if(or_status == 2) {
                ug_printf(0, 0 + 14 + 20, C_YELLOW, "Ro: !Wait No-load");
            } else {
                ug_printf(0, 0 + 14 + 20, C_YELLOW, "Ro: %d !manual", (int)rout);
            }
            ug_printf(0, 0 + 14 + 40, C_YELLOW, "GAIN: %d.%02d", (int)outgain, (int)(outgain*100)%100);
            ug_printf(0, 0 + 14 + 60, C_YELLOW, "FREQ: %d", dds_output_freq);
            ug_printf(0, 0 + 14 + 80, C_BLUE, "IN1: %duV", (int)(value_base_amp[CHANNEL_IN1]*1000));
            ug_printf(0, 0 + 14 + 100, C_BLUE, "IN2: %duV", (int)(value_base_amp[CHANNEL_IN2]*1000));
            ug_printf(0, 0 + 14 + 120, C_BLUE, "OUT: %dmV", (int)value_base_amp[CHANNEL_OUT]);
            ug_printf(0, 0 + 14 + 140, C_BLUE, "OUTL: %dmV", (int)value_base_amp[CHANNEL_OUT_LOAD]);
            ug_printf(0, 0 + 14 + 160, C_BLUE, "VIDC: %dmV", gs_vin_dc);
            ug_printf(0, 0 + 14 + 180, C_BLUE, "VODC: %dmV", gs_vout_dc);
            
            break;
        case LCD_MODE_FAULT:
            ug_printf(0, 0 + 14, C_GREEN, "Fault detection");
            ug_printf(0, 0 + 14 + 20, C_RED, "[%d]", fault_id);
            ug_printf(0, 0 + 14 + 40, C_RED, "%s", fault_string[fault_id]);
            switch(gs_lcd_mode_fault) {
            case LCD_MODE_FAULT_SHOW:
                ug_printf(0, 0 + 14 + 80, C_BLUE, "RI: %d", (int)rin_1k);
                ug_printf(0, 0 + 14 + 100, C_BLUE, "VIDC: %dmV(%d)", gs_vin_dc, vidc_inc);
                ug_printf(0, 0 + 14 + 120, C_BLUE, "VODC: %dmV", gs_vout_dc);
                
                ug_printf(0, 0 + 14 + 140, C_BLUE, "UpGain: %d", (int)up_gain);
                ug_printf(0, 0 + 14 + 160, C_BLUE, "MpGain: %d", (int)mp_gain);
                ug_printf(0, 0 + 14 + 180, C_BLUE, "LpGain: %d", (int)lp_gain);
                ug_printf(0, 0 + 14 + 200, C_BLUE, "IN2: %duV", (int)(value_base_amp[CHANNEL_IN2]*1000));
                break;
            case LCD_MODE_FAULT_RI:
                ug_printf(0, 0 + 14 + 80, C_BLUE, "RI: %d", (int)rin_1k);
                break;
            case LCD_MODE_FAULT_DCI:
                ug_printf(0, 0 + 14 + 80, C_BLUE, "VIDC: %dmV(%d)", gs_vin_dc, vidc_inc);
                break;
            case LCD_MODE_FAULT_DCO:
                ug_printf(0, 0 + 14 + 80, C_BLUE, "VODC: %dmV", gs_vout_dc);
                break;
            case LCD_MODE_FAULT_UP:
                ug_printf(0, 0 + 14 + 80, C_BLUE, "UpGain: %d", (int)up_gain);
                break;
            case LCD_MODE_FAULT_LP:
                ug_printf(0, 0 + 14 + 80, C_BLUE, "LpGain: %d", (int)lp_gain);
                break;
            }
            
            break;
        /*cal mode*/
        case LCD_MODE_CHANNEL:
            ug_printf(0, 0 + 14, C_ORANGE, "Cal mode");
            ug_printf(0, 0 + 14 + 20, C_ORANGE, "Ch select, CH%d", gs_lcd_cal_ch);
        
            if(gs_lcd_cal_ch == 0 || gs_lcd_cal_ch == 1) {
                ug_printf(0, 0 + 14 + 60, C_ORANGE, "AMP:%duV", (int)(value_base_amp[gs_lcd_cal_ch]*1000));
                ug_printf(0, 0 + 14 + 80, C_ORANGE, "RI: %d", (int)rin);
                ug_printf(0, 0 + 14 + 100, C_ORANGE, "GAIN: %d", (int)outgain);
            } else {
                ug_printf(0, 0 + 14 + 60, C_ORANGE, "AMP:%dmV", (int)value_base_amp[gs_lcd_cal_ch]);
                ug_printf(0, 0 + 14 + 80, C_ORANGE, "GAIN: %d", (int)outgain);
            }
            break;
        case LCD_MODE_CAL:
            ug_printf(0, 0 + 14, C_RED, "Cal mode, CH%d", gs_lcd_cal_ch);
            if(gs_lcd_cal_ch == 0 || gs_lcd_cal_ch == 1) {
                ug_printf(0, 0 + 14 + 20, C_RED, "[%d]set %duV", gs_lcd_cal_index, (int)(gs_para_y_div[gs_lcd_cal_index]*1000));
            } else {
                ug_printf(0, 0 + 14 + 20, C_RED, "[%d]set %dmV", gs_lcd_cal_index, (int)gs_para_y_div[gs_lcd_cal_index]);
            }
            
            ug_printf(0, 0 + 14 + 40, C_ORANGE, "ADC AMP:%.1fmV", gs_base_amp_avg[gs_lcd_cal_ch]*1000);
            
            if(gs_lcd_cal_ch == 0 || gs_lcd_cal_ch == 1) {
                ug_printf(0, 0 + 14 + 60, C_RED, "AMP:%duV", (int)(value_base_amp[gs_lcd_cal_ch]*1000));
                ug_printf(0, 0 + 14 + 80, C_ORANGE, "RI: %d", (int)rin);
                ug_printf(0, 0 + 14 + 100, C_ORANGE, "GAIN: %d", (int)outgain);
            } else {
                ug_printf(0, 0 + 14 + 60, C_RED, "AMP:%dmV", (int)value_base_amp[gs_lcd_cal_ch]);
                ug_printf(0, 0 + 14 + 80, C_ORANGE, "GAIN: %d", (int)outgain);
            }
            break;
        case LCD_MODE_CAL_OK:
            ug_printf(0, 0 + 14, C_ORANGE, "Cal mode, CH%d", gs_lcd_cal_ch);
            ug_printf(0, 0 + 14 + 20, C_GREEN, "OK SAVE?");
            ug_printf(0, 0 + 14 + 40, C_GREEN, "ADC AMP:%.1fmV", gs_base_amp_avg[gs_lcd_cal_ch]*1000);

            if(gs_lcd_cal_ch == 0 || gs_lcd_cal_ch == 1) {
                ug_printf(0, 0 + 14 + 60, C_BLUE, "AMP:%duV", (int)(value_base_amp[gs_lcd_cal_ch]*1000));
                ug_printf(0, 0 + 14 + 80, C_GREEN, "RI: %d", (int)rin);
                ug_printf(0, 0 + 14 + 100, C_GREEN, "GAIN: %d", (int)outgain);
            } else {
                ug_printf(0, 0 + 14 + 60, C_BLUE, "AMP:%dmV", (int)value_base_amp[gs_lcd_cal_ch]);
                ug_printf(0, 0 + 14 + 80, C_ORANGE, "GAIN: %d", (int)outgain);
            }
            break;
        /*cal mode end*/
        default:
            break;
        }
        UG_FontSelect ( &FONT_8X14 );
        UG_SetBackcolor(C_BLACK);
    }
}

char *mode_fault_string[LCD_MODE_FAULT_NUMBER] = {
    [LCD_MODE_FAULT_SHOW] = "分析模式",
    [LCD_MODE_FAULT_RI]   = "输入电阻模式",
    [LCD_MODE_FAULT_DCI]  = "输入电压模式",
    [LCD_MODE_FAULT_DCO]  = "输出电压模式",
    [LCD_MODE_FAULT_UP]  = "电容3增益模式",
    [LCD_MODE_FAULT_LP]  = "电容2增益模式",
};

char *mode_string[LCD_MODE_NUMBER] = {
    [0] = "测量模式",
    [1] = "故障分析模式",
    [2] = "参数校准模式",
};


void key_inout_receive_proc(int8_t id)
{
    APP_DEBUG("KEY = %d \r\n", id);
    lcd_printf("KEY = %d \r\n", id);
    wave_on = 0;
    switch(id) {
    case KEY_MODE_SWITCH_PAGE:
        if(gs_lcd_mode < LCD_MODE_NUMBER) {
            if(gs_lcd_cal_ch == 1) {
                //恢复频率
                setWave(SINE, 1000);
            }
            gs_lcd_mode++;
            if(gs_lcd_mode >= LCD_MODE_NUMBER) {
                gs_lcd_mode = 0;
            }
            tts_printf("%s", mode_string[gs_lcd_mode]);
        } else {
            
        }
        break;
    case KEY_MODE_ADD:
        switch(gs_lcd_mode) {
        case LCD_MODE_FAULT:
            gs_lcd_mode_fault++;
            if(gs_lcd_mode_fault >= LCD_MODE_FAULT_NUMBER) {
                gs_lcd_mode_fault = 0;
            }
            tts_printf("%s", mode_fault_string[gs_lcd_mode_fault]);
        case LCD_MODE_CHANNEL:
            //选择下一个通道
            gs_lcd_cal_ch++;
            if(gs_lcd_cal_ch >= PARA_CHANNEL_NUMBER) {
                gs_lcd_cal_ch = 0;
            }
            tts_printf("通道%d", gs_lcd_cal_ch);
            break;
        case LCD_MODE_CAL:
            //快速逼近
            gs_para_y_div[gs_lcd_cal_index] = value_base_amp[gs_lcd_cal_ch];
            break;
        default:
            break;
        }
        break;
    case KEY_MODE_OK:
        switch(gs_lcd_mode) {
        case LCD_MODE_CHANNEL:
            //当按下OK键，且LCD模式为选择通道界面
            gs_lcd_mode = LCD_MODE_CAL; //进入设定值模式
            gs_lcd_cal_index = 0;
            gs_para_y_div[gs_lcd_cal_index] = value_base_amp[gs_lcd_cal_ch];
            tts_printf("请校准参数%d", gs_lcd_cal_index);
            break;
        case LCD_MODE_CAL:
            //设定值模式
            if(gs_lcd_cal_ch == CHANNEL_OUT_LOAD)
                gs_para_x_div[gs_lcd_cal_index] = gs_base_amp_avg[CHANNEL_OUT]; //VOUT_LOAD
            else
                gs_para_x_div[gs_lcd_cal_index] = gs_base_amp_avg[gs_lcd_cal_ch];
            gs_lcd_cal_index++;
            if(gs_lcd_cal_index >= PARA_NUM) {
                //计算回归曲线
                
                for(int i=0; i<PARA_NUM; i++) {
                    printf("%.3f, ", gs_para_x_div[i]);
                }
                printf("\r\n");
                
                param_value_reset(gs_para_x_div, gs_para_y_div, gs_lcd_cal_ch, gs_lcd_cal_index);
                
                //进入参数确认界面
                gs_lcd_cal_index = 0;
                gs_lcd_mode = LCD_MODE_CAL_OK;
                break;
            }
            gs_para_y_div[gs_lcd_cal_index] = value_base_amp[gs_lcd_cal_ch];
            tts_printf("请校准参数%d", gs_lcd_cal_index);
            break;
        case LCD_MODE_CAL_OK:
            //参数确认界面
            //写入内部FLASH
            switch(gs_lcd_mode) {
                case LCD_MODE_CAL_OK:
                    param_value_save();
                    tts_printf("保存");
                    gs_lcd_mode = LCD_MODE_CHANNEL;
                default:
                    ;
            }
            break;
        default:
            break;
        }
        break;
    case KEY_MODE_KADD:
        switch(gs_lcd_mode) {
        case LCD_MODE_CHANNEL:
            //B++;
            if(gs_lcd_cal_ch == 0 || gs_lcd_cal_ch == 1) {
                gs_para[gs_lcd_cal_ch].b += 0.05;
            } else {
                gs_para[gs_lcd_cal_ch].b += 1;
            }
            break;
        case LCD_MODE_CAL:
            if(gs_lcd_cal_ch == 0 || gs_lcd_cal_ch == 1) {
                gs_para_y_div[gs_lcd_cal_index] += 0.05;
            } else {
                gs_para_y_div[gs_lcd_cal_index] += 1;
            }
            break;
        default:
            break;
        }
        break;
    case KEY_MODE_KSUB:
        switch(gs_lcd_mode) {
        case LCD_MODE_CHANNEL:
            //B--;
            if(gs_lcd_cal_ch == 0 || gs_lcd_cal_ch == 1) {
                gs_para[gs_lcd_cal_ch].b -= 0.05;
            } else {
                gs_para[gs_lcd_cal_ch].b -= 1;
            }
            break;
        case LCD_MODE_CAL:
            if(gs_lcd_cal_ch == 0 || gs_lcd_cal_ch == 1) {
                gs_para_y_div[gs_lcd_cal_index] -= 0.05;
            } else {
                gs_para_y_div[gs_lcd_cal_index] -= 1;
            }
            break;
        default:
            break;
        }
        break;
    case KEY_MODE_SAVE:
        switch(gs_lcd_mode) {
        case LCD_MODE_CHANNEL:
            //B--;
            param_value_save();
            tts_printf("保存");
            break;
        case LCD_MODE_CAL:
            //计算回归曲线
            
            for(int i=0; i<gs_lcd_cal_index; i++) {
                printf("%.3f, ", gs_para_x_div[i]);
            }
            printf("\r\n");
            param_value_reset(gs_para_x_div, gs_para_y_div, gs_lcd_cal_ch, gs_lcd_cal_index);
            
            //进入参数确认界面
            gs_lcd_cal_index = 0;
            gs_lcd_mode = LCD_MODE_CAL_OK;
            break;
        default:
            break;
        }
        break;
    case KEY_MODE_FREQ_SCAN:
        if(freq_scan_status == FS_STOP) {
            freq_scan_status = FS_START;
            wave_on = 1;
            tts_printf("伏频特性扫描模式");
            soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 10);
        } else {
            gain_1k = 100;
        }
        break;
    case KEY_MODE_RESET_1K:
        wave_on = 0;
        tts_printf("当前输出频率1千赫兹");
        setWave(SINE, 1000);
        break;
    case KEY_MODE_OR:
        wave_on = 0;
        //output register
        {
            static float or_uf = 0;
            
            if(or_status == 0) {
                setWave(SINE, 1000);
                APP_DEBUG(RED_FONT, "please add load resistor\r\n");
                tts_printf("请接通负载电阻");
                or_status = 1;
                rout = 0;
            } else if(or_status == 1){
                APP_DEBUG(RED_FONT, "please del load resistor\r\n");
                
                or_uf = value_base_amp[CHANNEL_OUT_LOAD];
                APP_WARN("Uf voltage = %.4fV\r\n", or_uf);
                tts_printf("请断开负载电阻");
                or_status = 2;
                rout = 0;
            } else {
                
                float or_uo = value_base_amp[CHANNEL_OUT];
                APP_WARN("Uo voltage = %.4fV\r\n", or_uo);
                rout = 2000.0*(or_uo - or_uf)/or_uf;
                APP_WARN("RO = %.0f\r\n", rout);
                if(rout > 1000.0f) {
                    tts_printf("输出电阻%.2f千欧", (rout/1000));
                } else {
                    tts_printf("输出电阻%d欧", (int)rout);
                }
                or_status = 0;
            }
        }
        break;
    case KEY_MODE_IR:
        wave_on = 0;
        if(rin > 1000.0f) {
            tts_printf("输入电阻%.2f千欧", (rin/1000));
        } else {
            tts_printf("输入电阻%d欧", (int)rin);
        }
        break;
    case KEY_MODE_GAIN:
        tts_printf("当前增益%.2f", outgain);
        break;
    case KEY_MODE_CONSOLE:
        //切换终端
        if(lcd_console_enable) {
            lcd_console_enable = 0;
            //关闭终端
        } else {
            lcd_console_enable = 1;
            //开启终端
        }
        break;
    case KEY_MODE_QUARE:
        setWave(SQUARE, dds_output_freq);
        break;
    }
}



void user_setup(void)
{
    PRINTF("\r\n\r\n[H7] Build , %s %s \r\n", __DATE__, __TIME__);
    
    /* hardware lowlevel init */
    data_interface_hal_init();
    
    /* adc parametra */
    param_default_value_init();
    
    /* soft timer */
    soft_timer_init();
    
    soft_timer_create(SOFT_TIMER_LED_DEBUG_ID, 1, 1, led_debug_proc, 200);
    soft_timer_create(SOFT_TIMER_UGUI_ID, 1, 1, UG_Update, 20);
    
    /* ugui */
    gui_init();
    gui_window_init();
    
    AD9833_AmpSet(dds_output_amp);
    setWave(SINE, dds_output_freq);
    
    fft_init();
    fft_hann_init();
    
    
    gui_wave_init(&gwav1, 0, 0, 240, 120, buffer_wave_1, C_BLACK);
    gui_wave_init(&gwav2, 0, 0, 240, 120, buffer_wave_2, C_BLACK);
    gui_wave_init(&gwav3, 0, 0, 240, 120, buffer_wave_3, C_BLACK);
    
    tts_init();
    printf("init success!\r\n");
}


void user_loop(void)
{
    
    TIMER_TASK(timer2, 100, wave_on) {
        gui_wave_draw(gwavs, 3);

        //ug_printf(0, 0, C_MAGENTA, "FREQ %.1fkHz", real_freq/1000.0);
        ug_printf(0, 0, C_BLUE, "FREQ %.1fkHz", gs_base_freq_hz[CHANNEL_IN2]/1000.0);
        ug_printf(0, 14, C_ORANGE, "GAIN %.1f", outgain_fast);
//        ug_printf(0, 128 - 20, C_LIGHT_CYAN, "T'D 300ms");
        
        UG_SetBackcolor(C_GRAY);
        UG_FillFrame(0, 120, 240-1, 240-1, C_GRAY);
        
        UG_FontSelect ( &FONT_6X10 );
        uint8_t local = 0;
        uint8_t show_height = 0;
        uint8_t show_width = 0;
        for(int i=0; i<240; i++) {
            if(i%30 == 0 || i == 239) {
                if(i==0) {
                    show_width = i;
                } else if(i==239) {
                    show_width = 240 - 30;
                } else {
                    show_width = i - 10;
                }
                float showfreq = log10(F_START_FREQ) + i*(log10(F_END_FREQ) - log10(F_START_FREQ))/F_COUNTER;
                showfreq = pow(10, showfreq);
                show_height = local?130:(120);
                if(showfreq >= 1000) {
                    ug_printf(show_width, show_height, C_LIGHT_CYAN, "%.0fk", showfreq/1000);
                } else {
                    ug_printf(show_width, show_height, C_LIGHT_CYAN, "%.0f", showfreq);
                }
                local = !local;
            }
        }
        
        UG_FontSelect ( &FONT_12X20 );
        ug_printf(0, 120 + 30, C_GREEN, "Fup %d KHz", (int)(up_freq/1000.0));
        ug_printf(0, 120 + 50, C_GREEN, "Flp %d Hz", (int)(lp_freq));
        ug_printf(0, 120 + 70, C_GREEN, "Aup %d", (int)gain_1k);
        
        UG_FontSelect ( &FONT_8X14 );
        UG_SetBackcolor(C_BLACK);
    }
    
    /* lcd flush */
    TIMER_TASK(time1, 33, 1) {
        if(lcd240x240_flush() >= 0) {
            fps_inc++;
        }
        
        tts_proc(); //TTS
    }
    
    /* real time task */
    /* adc proc task */
    adc_rx_proc(adc3_receive_proc);
    
     /* key scan task */
    TIMER_TASK(time0, 5, 1) {
        key_inout_proc(key_inout_receive_proc);
    }
    /* soft timer */
    soft_timer_proc();
}


/*****************************END OF FILE***************************/
