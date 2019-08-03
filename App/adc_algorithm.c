/* 2019 06 25 */
/* By hxdyxd */
#include "adc_algorithm.h"
#include "data_interface_hal.h"

/*
 * Hardware gain
 */
struct adc_adjustment_t value_adc_adjustment_key[ADC1_CHANNEL_NUMBER] = {
    {
        .key = 10.0/(100 + 10),
        .info = "VOUT",
    },
    {
        .key = 10.0/(180 + 10),
        .info = "VIN",
    },
    {
        .key = 10.0/(180 + 10),
        .info = "VLED",
    },
};

/**
  * @brief  ADC PHYSICAL VALUE GET
  */
float value_adc_physical_get(float adc_voltage, int id)
{
    if(id >= ADC1_CHANNEL_NUMBER) {
        return -1.0;
    }
    return (float)adc_voltage/value_adc_adjustment_key[id].key;
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
#define PARA_NUM   (2)

float param_x_val[PARA_NUM] = {
    0.391, 0.828, 
};

float param_y_val[PARA_NUM] = { 
    0.439, 0.874,
};


struct param_t gs_para[ADC1_CHANNEL_NUMBER];

void param_default_value_init(void)
{
    for(int i=0; i<ADC1_CHANNEL_NUMBER; i++) {
        polyfit1(param_x_val, param_y_val, PARA_NUM, &gs_para[i].k, &gs_para[i].b);
        printf("ch %d k: %.3f, b: %.3f\r\n", i, gs_para[i].k, gs_para[i].b);
    }
}

inline float get_param_value(float input, int i)
{
    return (input * gs_para[i].k) + gs_para[i].b;
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
