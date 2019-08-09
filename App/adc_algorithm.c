/* 2019 06 25 */
/* By hxdyxd */
#include "adc_algorithm.h"
#include <arm_math.h>

/*
 * Hardware gain
 */
struct adc_adjustment_t value_adc_adjustment_key[ADC1_CHANNEL_NUMBER] = {
    [BVOUT] = {
        .key = 10.0/(100 + 10),
        .info = "BVOUT",
    },
    [PVIN] = {
        .key = 10.0/(150 + 10),
        .info = "PVIN",
    },
};


/**
  * @brief  ADC PHYSICAL VALUE GET
  */
float value_adc_physical_set(float adc_voltage, int id)
{
    if(id >= ADC1_CHANNEL_NUMBER) {
        return -1.0;
    }
    value_adc_adjustment_key[id].physical_val = (float)adc_voltage/value_adc_adjustment_key[id].key;
    return value_adc_adjustment_key[id].physical_val;
}

/**
  * @brief  ADC INFO STRING
  */
char *value_adc_info(int id)
{
    if(id >= ADC1_CHANNEL_NUMBER) {
        return "0";
    }
    return value_adc_adjustment_key[id].info;
}


/*****************************************************************************/

void polyfit1(float *x, float *y, uint16_t num, double *output_k, double *output_b)
{
    uint16_t i;
    double rx = num;
    double ry = num;
    double xiyi = 0, xi2 = 0;
    double hx = 0, hy = 0;
    for(i=0;i<rx;i++) {
        xiyi = xiyi + x[i] * y[i];
        xi2 = xi2 + x[i] * x[i];
        hx = hx + x[i];
        hy = hy + y[i];
    }
    double pjx = hx/rx;
    double pjy = hy/ry;
    *output_k = (xiyi - rx*pjx*pjy)/(xi2 - rx*pjx*pjx);
    *output_b = pjy - (*output_k)*pjx;
}


/*
 * 采样校准参数
 */
#define PARA_NUM   (4)

float param_x_val[ADC1_CHANNEL_NUMBER][PARA_NUM] = {
    {185.8, 130.6, 94.4, 38.6},
    {220.6, 187.0, 119, 65.2},
    {193.1, 160.1, 115.7, 82.7},
};

float param_y_val[ADC1_CHANNEL_NUMBER][PARA_NUM] = { 
    {16.81, 11.81, 8.5, 3.52},
    {17.06, 14.43, 9.27, 5.09},
    {2137, 1780, 1289, 909},
};


struct param_t gs_para[ADC1_CHANNEL_NUMBER];

void param_default_value_init(void)
{
    for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
        polyfit1(param_x_val[i], param_y_val[i], PARA_NUM, &gs_para[i].k, &gs_para[i].b);
        printf("ch %d k: %.3f, b: %.3f\r\n", i, gs_para[i].k, gs_para[i].b);
    }
}

inline float get_param_value(float input, int channel)
{
    return (input * gs_para[channel].k) + gs_para[channel].b;
}

/*******************************************************/
float fft_inputbuf[FFT_LENGTH];
float fft_outputbuf[FFT_LENGTH*2];


static arm_rfft_fast_instance_f32 srfft;


void fft_init(void)
{
    arm_rfft_fast_init_f32(&srfft, FFT_LENGTH);
}

void fft_fast_real_u16_to_float(uint16_t *buf, float *abs_outputbuf)
{
    uint16_t i;
    for(i=0; i<FFT_LENGTH; i++) {
        //printf("%d ", buf[i]);
        
         fft_inputbuf[ i ] = buf[i]; //生成输入信号实部
         //fft_inputbuf[ (i<<1) +1] = 0;  //虚部全部为0
    }


    arm_rfft_fast_f32(&srfft, fft_inputbuf, fft_outputbuf, 0);  //FFT计算(基4)
    arm_cmplx_mag_f32(fft_outputbuf, abs_outputbuf, FFT_LENGTH); //把运算结果复数求模得幅值
}

uint32_t find_fft_max_freq_index(float *buf, uint32_t size)
{
    uint32_t max = 1;
    int i;

    for(i=1;i<size;i++) {
        if( buf[i] > buf[max] ) {
            max = i;
        }
    }

    return max;
}

static float flat_val[FFT_LENGTH];

void fft_hann_init(void)
{
    //hann
    for(int i=0;i<FFT_LENGTH;i++) {
        float pi = 3.1415926;
        flat_val[i] = (1 - 1.93*cos(2*pi*i/FFT_LENGTH) + 1.29*cos(4*pi*i/FFT_LENGTH) - 0.388*cos(6*pi*i/FFT_LENGTH) + 0.0322*cos(8*pi*i/FFT_LENGTH))/ 4.634;
        //printf("%d ", buf[i]);
    }
}

void fft_hann_get(uint16_t *hann_out, uint16_t *input, float zero_val)
{
    //hann
    for(int i=0;i<FFT_LENGTH;i++) {
        hann_out[i] = (uint16_t)((input[i] - zero_val) * flat_val[i] + zero_val) ;
    }
}


/******************[PID CONTROLLER]****************/
/******************[2019 06 25 HXDYXD]*************/

void pid_init(pidc_t *pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

void pid_set_output_limit(pidc_t *pid, float max_output, float min_output)
{
    pid->max_output = max_output;
    pid->min_output = min_output;
}

void pid_set_value(pidc_t *pid, float setval)
{
    pid->setval = setval;
}

float pid_ctrl(pidc_t *pid, float curval)
{
    float e = pid->setval - curval;  //p
    float de = e - pid->le;  //d
    pid->le = e;
    pid->se += e;  //i
    
    if(pid->se > pid->i_max) {
        pid->se = pid->i_max;
    } else if(pid->se < -pid->i_max) {
        pid->se = -pid->i_max;
    }
    
    pid->output += (e * pid->kp) + (pid->se * pid->ki) + (de * pid->kd);
    
    if(pid->output > pid->max_output) {
        pid->output = pid->max_output;
    } else if(pid->output < pid->min_output) {
        pid->output = pid->min_output;
    }
    
    return pid->output;
}

/******************[PID CONTROLLER]****************/



/*
 * ADC去极值平均滤波器
*/
uint16_t No_Max_Min_Filter(uint16_t *in_dat, uint16_t num, uint8_t channel_num, uint8_t n)
{
    int i;
    uint32_t sum = 0;
    float sum_f;
    uint16_t max_value, min_value;
    max_value = in_dat[n];
    min_value = in_dat[n];

    for(i=n; i<num*channel_num; i+=channel_num){
        if(in_dat[i] > max_value){
            max_value = in_dat[i];
        }
        if(in_dat[i] < min_value){
            min_value = in_dat[i];
        }
        sum += in_dat[i];
    }
    sum -= max_value;
    sum -= min_value;
    sum_f = (float)sum / (num - 2.0);
    return (uint16_t)sum_f;
}


/*****************************END OF FILE***************************/
