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

uint16_t dma_buf_div1[FFT_LENGTH];
uint16_t dma_buf_div2[FFT_LENGTH];
uint16_t dma_buf_tmp[FFT_LENGTH];

float gs_base_freq_hz = 0;
float gs_base_amp = 0;

void adc3_receive_proc(int id, void *pbuf, int len)
{
    uint16_t *adc_data = (uint16_t *)pbuf;
    
    
    
    if(id == 1) {
        static int flag = 0;
        
        flag++;
        if(flag >= 244) {
            flag = 0;
            APP_DEBUG("adc 1s \r\n");
        }
        
        for(int i=0; i<FFT_LENGTH; i++) {
            dma_buf_div1[i] = adc_data[i*ADC1_CHANNEL_NUMBER];
            dma_buf_div2[i] = adc_data[i*ADC1_CHANNEL_NUMBER + 1];
        }
        
#if 0
        TIMER_TASK(timer1, 1000, 1) {

            //用于MATLAB绘制频域图像
            for(int i= 0;i< FFT_LENGTH ;i++) {
                
                printf("%d ",  dma_buf_div1[i] );
            }
            printf("\r\n");

        }
#endif
        //FFT及基波幅度分析
        fft_fast_real_u16_to_float(dma_buf_div1, abs_outputbuf);    //实数FFT运算
        float zero_val = abs_outputbuf[0]/FFT_LENGTH;               //直流分量
        
        
        fft_hann_get(dma_buf_tmp, dma_buf_div1, zero_val);         //hann window
        
        fft_fast_real_u16_to_float(dma_buf_tmp, abs_outputbuf);    //实数FFT运算,with hann window
        
        uint32_t base_freq = find_fft_max_freq_index(abs_outputbuf, FFT_LENGTH/2);   //找出幅度最大值，前面一半数据有效
        float base_freq_hz  = FFT_INDEX_TO_FREQ(base_freq, ADC1_FREQ_SAMP);   //转化为真实频率
        float base_amp = FFT_ASSI(abs_outputbuf[base_freq]);
        
        gs_base_freq_hz = base_freq_hz;
        TIMER_TASK(timer2, 10, 1) {
            gs_base_amp = ADC_12BIT_VOLTAGE_GET(base_amp);
            gui_wave_set(&gwav3, EASY_LR(gs_base_amp, 0, 0, 0.5, 120.0), C_ORANGE);
        }
        
        TIMER_TASK(timer3, 1000, 1) {
            APP_DEBUG( "zero_val = %.3f \r\n", zero_val );
            APP_DEBUG("base_freq_hz = %.1f, vpp = %.3fV\r\n", base_freq_hz,  ADC_12BIT_VOLTAGE_GET(base_amp));
            
        }
        
    }
    
    if(id == 3) {
        
    }
}


#define F_START_FREQ (100)
#define F_END_FREQ   (400000)
#define F_COUNTER    (240)


#define FS_STOP         (0)
#define FS_START        (1)
#define FS_NOT_DETECT   (2)
#define FS_DETECTED     (3)

uint8_t freq_scan_status = FS_STOP;
static float cur_freq_set = 0;
static int freq_point = 0;
static uint32_t real_freq = 0;


void freq_scan_proc(void)
{
    
    switch(freq_scan_status) {
    case FS_START:
        freq_point = 0;
        cur_freq_set = log10(F_START_FREQ);
        real_freq = pow(10, cur_freq_set);
        setWave(SINE, real_freq);
        soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 20);
        APP_DEBUG("start freq scan, cur_freq_set = %dHz\r\n", real_freq);
        lcd_printf("[%d] freq = %dHz\n", freq_point, real_freq);
        freq_scan_status = FS_NOT_DETECT;
        break;
    case FS_NOT_DETECT:
        freq_point++;
        if(freq_point >= F_COUNTER || cur_freq_set >= log10(F_END_FREQ) || 0) {
            APP_DEBUG("[%d] success freq scan, %d\r\n", freq_point, real_freq);
            lcd_printf("[%d] scan success\n", freq_point);
            freq_scan_status = FS_STOP;
            return;
        }
        cur_freq_set += (log10(F_END_FREQ) - log10(F_START_FREQ))/F_COUNTER;
        real_freq = pow(10, cur_freq_set);
        setWave(SINE, real_freq);
        soft_timer_create(SOFT_TIMER_FREQSCAN_ID, 1, 0, freq_scan_proc, 20);
        APP_DEBUG("[%d] run freq scan, cur_freq_set = %dHz\r\n", freq_point, real_freq);
        lcd_printf("[%d] freq = %dHz\n", freq_point, real_freq);
        break;
    default:
        APP_DEBUG("default ?\r\n");
        break;
    }
    gui_wave_set(&gwav1, EASY_LR(real_freq, 0, 0, F_END_FREQ, 120.0), C_MAGENTA);
    gui_wave_set(&gwav2, EASY_LR(gs_base_freq_hz, F_END_FREQ, 0, 0, 120.0), C_BLUE);
}





uint32_t fps_inc = 0;

void led_debug_proc(void)
{
    
    LED_REV(LED0_BASE);
    
    static uint16_t i = 0;
    i = (i+1) % 4;
    
    
    if(i&1)
        UG_TouchUpdate(15, 60, TOUCH_STATE_PRESSED );
    else
        UG_TouchUpdate(-1, -1, TOUCH_STATE_RELEASED );
    
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
    
    
    TIMER_TASK(timer0, 500, 1) {
        lcd_printf("DDS F %.1f, A %d\n", dds_output_freq, dds_output_amp);
        TIMER_TASK(timer0, 1000, 1) {
            static uint32_t last_timer = 0;
            APP_DEBUG("fps: %d\r\n", fps_inc*1000/(hal_read_TickCounter() - last_timer) );
            fps_inc = 0;
            last_timer = hal_read_TickCounter();
        }
    }
    
//    static uint16_t color_buf[4] = {RED, GREEN, BLUE, YELLOW};
//    UG_FillScreen( color_buf[i] );
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
    
    printf("init success!\r\n");
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
    }
    
    TIMER_TASK(timer2, 100, 1) {
        gui_wave_draw(gwavs, 3);

        ug_printf(0, 0, C_MAGENTA, "FREQ %.1fkHz", real_freq/1000.0);
        ug_printf(0, 14, C_BLUE, "FREQ %.1fkHz", gs_base_freq_hz/1000.0);
        ug_printf(0, 28, C_ORANGE, "Amp %.1fmV", gs_base_amp*1000);
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
