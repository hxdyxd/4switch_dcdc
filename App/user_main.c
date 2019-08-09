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

#define  FREQ_SCAN           (0)
#define  FREQ_SCAN_START     (1)


static uint8_t buffer_wave_1[240];
static uint8_t buffer_wave_2[240];
static uint8_t buffer_wave_3[240];
static struct lcd_wave_t gwav1, gwav2, gwav3;
static struct lcd_wave_t *gwavs[3] = {
    &gwav1, &gwav2, &gwav3, 
};



uint8_t mode_run = 0;


uint8_t dds_output_amp = 100;
float dds_output_freq = 1e3;



float abs_outputbuf[FFT_LENGTH];

uint16_t dma_buf_div[ADC1_CHANNEL_NUMBER][FFT_LENGTH];
uint16_t dma_buf_tmp[FFT_LENGTH];

float gs_base_freq_hz[ADC1_CHANNEL_NUMBER] = {0};
float gs_base_amp[ADC1_CHANNEL_NUMBER] = {0};
float gs_zero_val[ADC1_CHANNEL_NUMBER]= {0};

float gs_base_amp_sum_20[ADC1_CHANNEL_NUMBER] = {0};
float gs_base_amp_avg_20[ADC1_CHANNEL_NUMBER] = {0};
uint32_t gs_base_amp_avg_20_cnt = 0;

static float value_base_amp[ADC1_CHANNEL_NUMBER] = {0};
static float value_base_amp_fast[ADC1_CHANNEL_NUMBER] = {0};

float rin = 0;
float outgain = 0;
float outgain_fast = 0;


uint32_t fft_cnt = 0;

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
            APP_DEBUG("adc 1s \r\n");
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
            value_base_amp_fast[i] = get_param_value(gs_base_amp_avg_20[i]*1000, i);
        }
        
        outgain_fast = value_base_amp_fast[CHANNEL_OUT]/value_base_amp_fast[CHANNEL_IN2];
        
        
        if(gs_base_amp_avg_20_cnt >= 20) {
            gs_base_amp_avg_20_cnt = 0;
            for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
                gs_base_amp_avg_20[i] = gs_base_amp_sum_20[i]/20.0;
                gs_base_amp_sum_20[i] = 0;
                //校准
                value_base_amp[i] = get_param_value(gs_base_amp_avg_20[i]*1000, i);
                rin = 2000*(value_base_amp[1])/(value_base_amp[0] - value_base_amp[1]);
                outgain = value_base_amp[CHANNEL_OUT]/value_base_amp[CHANNEL_IN2];
            }
        } else {
            for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
                gs_base_amp_sum_20[i] += gs_base_amp[i];
            }
            gs_base_amp_avg_20_cnt++;
        }
        
        
        TIMER_TASK(timer2, 10, 1) {
            //gui_wave_set(&gwav3, EASY_LR(gs_base_amp[0], 0, 0, 0.5, 120.0), C_ORANGE);
        }
        
        
        TIMER_TASK(timer3, 1000, 1) {
            for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
                APP_DEBUG( "[%d] zero_val = (%.1f) %.1fmV \r\n", i, gs_zero_val[i], ADC_12BIT_VOLTAGE_GET(gs_zero_val[i])*1000 );
                APP_DEBUG("[%d] base_freq_hz = %.1f, vpp = %.1fmV\r\n", i, gs_base_freq_hz[i], gs_base_amp[i]*1000);
                
                APP_DEBUG("avg20: %.1fmV, real: %.3fmV \r\n", gs_base_amp_avg_20[i]*1000, value_base_amp[i]);
            }
            
            APP_DEBUG("RI: %.1f\r\n", rin );
            APP_DEBUG("GAIN: %.1f\r\n", outgain );
            
            
            static uint32_t last_timer = 0;
            APP_DEBUG("FFT CNT: %d\r\n", fft_cnt*1000/(hal_read_TickCounter() - last_timer) );
            fft_cnt = 0;
            last_timer = hal_read_TickCounter();
        }
        
    }
    
    if(id == 3) {
        APP_DEBUG("adc3\r\n");
    }
}


#define F_START_FREQ (10000)
#define F_END_FREQ   (250000)
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


void freq_scan_proc(void)
{
    static float gain_1k = 0;
    switch(freq_scan_status) {
    case FS_START:
        //记录初始增益
        setWave(SINE, 1000);
        soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 400);
        freq_scan_status = FS_START_SCAN;
    case FS_START_SCAN:
        //使用精确增益
        gain_1k = outgain / 1.41421;
        APP_DEBUG("start freq scan, set 1k, gain = %.1f\r\n", gain_1k);
        
        //开始扫频
        freq_point = 0;
        cur_freq_set = log10(F_START_FREQ);
        real_freq = pow(10, cur_freq_set);
        setWave(SINE, real_freq);
        soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 80);
        APP_DEBUG("start freq scan, cur_freq_set = %dHz\r\n", real_freq);
        lcd_printf("[%d] freq = %dHz\n", freq_point, real_freq);
        freq_scan_status = FS_NOT_DETECT;
        break;
    case FS_NOT_DETECT:
        //扫频中
        freq_point++;
        if(/*freq_point >= F_COUNTER ||*/ cur_freq_set >= log10(F_END_FREQ) || (outgain_fast < gain_1k && real_freq > 1000)) {
            //扫频结束
            APP_DEBUG("[%d] success freq scan, %d\r\n", freq_point, real_freq);
            lcd_printf("[%d] scan success, %d\n", freq_point, real_freq);
            freq_scan_status = FS_STOP;
            return;
        }
        cur_freq_set += (log10(F_END_FREQ) - log10(F_START_FREQ))/F_COUNTER;
        real_freq = pow(10, cur_freq_set);
        setWave(SINE, real_freq);
        soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 80);
        APP_DEBUG("[%d] run freq scan, cur_freq_set = %dHz, GAIN = %.1f\r\n", freq_point, real_freq, outgain_fast);
        lcd_printf("[%d] freq = %dHz\n", freq_point, real_freq);
        break;
    default:
        APP_DEBUG("default ?\r\n");
        break;
    }
    gui_wave_set(&gwav1, EASY_LR(real_freq, 0, 0, F_END_FREQ, 120.0), C_MAGENTA);
    gui_wave_set(&gwav2, EASY_LR(gs_base_freq_hz[CHANNEL_IN2], F_END_FREQ, 0, 0, 120.0), C_BLUE);
    gui_wave_set(&gwav3, EASY_LR(outgain_fast, 0, 0, 300, 120.0), C_ORANGE);
}



uint32_t fps_inc = 0;

static uint8_t or_status = 0;
static float rout = 0;

void led_debug_proc(void)
{
    
    LED_REV(LED0_BASE);
    
    
    /* encoder */
    short encoder_cnt = ENCODER_CNT;
    ENCODER_CNT = 0;
    
    if(encoder_cnt != 0) {
        APP_DEBUG("ENCODER_CNT = %d \r\n", encoder_cnt);
        lcd_printf("ENCODER_CNT = %d \r\n", encoder_cnt);
        float point = encoder_cnt * 0.25f;
        /* --> */
        dds_output_freq += point*400;
        if(dds_output_freq < 0.0) {
            dds_output_freq = 0;
        }
        if(dds_output_freq > 1.0e6) {
            dds_output_freq = 1.0e6;
        }
        
        setWave(SINE, dds_output_freq);
        lcd_printf("SET DDS FREQ = %.1f\n", dds_output_freq);
    }
    /***********************************************/
    
    
    TIMER_TASK(timer0, 500, lcd_console_enable) {
        lcd_printf("ZERO = %.1f\n", ADC_12BIT_VOLTAGE_GET(gs_zero_val[0]) );
        lcd_printf("DDS F %.1f, A %d\n", dds_output_freq, dds_output_amp);
        TIMER_TASK(timer0, 1000, 1) {
            static uint32_t last_timer = 0;
            APP_DEBUG("fps: %d\r\n", fps_inc*1000/(hal_read_TickCounter() - last_timer) );
            fps_inc = 0;
            last_timer = hal_read_TickCounter();
        }
    }
    
    TIMER_TASK(timer1, 500, !lcd_console_enable) {
        UG_FillFrame(0, 128 + 14, 240-1, 240-1, C_GRAY);
        UG_FontSelect ( &FONT_12X20 );
        UG_SetBackcolor(C_GRAY);
        ug_printf(0, 128 + 14, C_BLUE, "Ri: %d", (int)rin);
        if(or_status == 1) {
            ug_printf(0, 128 + 14 + 20, C_BLUE, "Ro: !Wait load");
        } else if(or_status == 2) {
            ug_printf(0, 128 + 14 + 20, C_BLUE, "Ro: !Wait No-load");
        } else {
            ug_printf(0, 128 + 14 + 20, C_BLUE, "Ro: %d !manual", (int)rout);
        }
        ug_printf(0, 128 + 14 + 40, C_BLUE, "GAIN: %d", (int)outgain);
        UG_FontSelect ( &FONT_8X14 );
        UG_SetBackcolor(C_BLACK);
    }
}


void key_inout_receive_proc(int8_t id)
{
    APP_DEBUG("KEY = %d \r\n", id);
    lcd_printf("KEY = %d \r\n", id);
    
    switch(id) {
    case 0:
        dds_output_amp+=3;
        AD9833_AmpSet(dds_output_amp);
        break;
    case 1:
        dds_output_amp-=3;
        AD9833_AmpSet(dds_output_amp);
        break;
    case 2:
        if(freq_scan_status == FS_STOP) {
            freq_scan_status = FS_START;
            soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 10);
        }
        break;
    case 3:
        setWave(SINE, 1000);
        break;
    case 4:
        //output register
        {
            static float or_uf = 0;
            
            if(or_status == 0) {
                setWave(SINE, 1000);
                APP_DEBUG(RED_FONT, "please add load resistor\r\n");
                or_status = 1;
                rout = 0;
            } else if(or_status == 1){
                APP_DEBUG(RED_FONT, "please del load resistor\r\n");
                
                or_uf = value_base_amp[CHANNEL_OUT];
                APP_WARN("Uf voltage = %.4fV\r\n", or_uf);
                or_status = 2;
                rout = 0;
            } else {
                
                float or_uo = value_base_amp[CHANNEL_OUT];
                APP_WARN("Uo voltage = %.4fV\r\n", or_uo);
                rout = 2000.0*(or_uo - or_uf)/or_uf;
                APP_WARN("RO = %.0f\r\n", rout);
                tts_printf("输出电阻%d欧", (int)rout);
                or_status = 0;
            }
        }
        break;
    case 5:
        tts_printf("输入电阻%d欧", (int)rin);
        break;
    case 12:
        //切换终端
        if(lcd_console_enable) {
            lcd_console_enable = 0;
            //关闭终端
        } else {
            lcd_console_enable = 1;
            //开启终端
        }
        break;
    }
}


uint8_t eeprom_mem[256];

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
    tts_printf("欢迎使用电路特性测试仪");
    
    char *str = "init success!\r\n";
    if(eeprom_write(0, str, strlen(str)+1)) {
        printf("eeprom_write ok\r\n");
    }
    
    if(eeprom_read(0, eeprom_mem, 256)) {
        printf("eeprom_read: %s \r\n\r\n", eeprom_mem);
    }
}


void user_loop(void)
{
    /* key scan task */
    TIMER_TASK(time0, 5, 1) {
        key_inout_proc(key_inout_receive_proc);
    }
    
    /* lcd flush */
    TIMER_TASK(time1, 33, 1) {
        if(lcd240x240_flush() >= 0) {
            fps_inc++;
        }
        
        tts_proc(); //TTS
    }
    
    TIMER_TASK(timer2, 100, 1) {
        gui_wave_draw(gwavs, 3);

        ug_printf(0, 0, C_MAGENTA, "FREQ %.1fkHz", real_freq/1000.0);
        ug_printf(0, 14, C_BLUE, "FREQ %.1fkHz", gs_base_freq_hz[CHANNEL_IN2]/1000.0);
        ug_printf(0, 28, C_ORANGE, "GAIN %.1f", outgain);
        ug_printf(0, 128 - 20, C_LIGHT_CYAN, "T'D 300ms");
        TIMER_TASK(timer2_1, 1000, 1) {
            UG_FontSelect ( &FONT_6X10 );
            for(int i=0; i<240; i++) {
                if(i%30 == 0) {
                    float showfreq = log10(F_START_FREQ) + i*(log10(F_END_FREQ) - log10(F_START_FREQ))/F_COUNTER;
                    showfreq = pow(10, showfreq);
                    if(showfreq >= 1000) {
                        ug_printf(i, 128, C_LIGHT_CYAN, "%.0fk", showfreq/1000);
                    } else {
                        ug_printf(i, 128, C_LIGHT_CYAN, "%.0f", showfreq);
                    }
                    
                }
            }
            UG_FontSelect ( &FONT_8X14 );
        }
    }
    
    /* real time task */
    /* adc proc task */
    adc_rx_proc(adc3_receive_proc);
    
    /* soft timer */
    soft_timer_proc();
}


/*****************************END OF FILE***************************/
