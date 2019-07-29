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


#define  TTS_ON      (0)

static char tts_buf[512];


void user_system_setup(void)
{

}



void flash_test2(void)
{
    SWITCH_TASK_INIT(task1);
    
    
#if TTS_ON
    SWITCH_TASK(task1) {
        int len = snprintf(tts_buf, 512, "输出功率%d  ", hal_read_TickCounter() );
        int ret = tts_puts(tts_buf, len);
        if(ret > 0) {
            APP_WARN("TTS OUT: %s\r\n", tts_buf);
        }
    }
#endif
    
    SWITCH_TASK_END(task1);
}



void mat_put(char *str, int rows, int cols, float32_t *pData)
{
    printf("Matrix %s %d*%d:\r\n", str, rows, cols);
    for(int i=0; i<rows*cols;i++) {
        if(i%cols == 0 && i)
            printf("\r\n");
        printf("%.4f, ", pData[i]);
    }
    printf("\r\n\r\n");
}


#define  MAT_ROWS   (3)
#define  MAT_COLS   (2)
float32_t pDataA[MAT_ROWS*MAT_COLS] = {
    1, -1, 
    -1, 2, 
    -2, 1, 
};

float32_t pDatab[MAT_ROWS] = {
    6,
    9,
    -1,
};

float32_t pDataA_temp1[MAT_ROWS*MAT_COLS];
float32_t pDataA_temp2[MAT_ROWS*MAT_COLS];
float32_t pDataA_temp3[MAT_ROWS*MAT_COLS];


void mat(void)
{
    arm_matrix_instance_f32  mAt1;
    arm_matrix_instance_f32  mAt2;
    arm_matrix_instance_f32  mAt3;
    
    /*****************************************************************/
    mAt3.numRows = MAT_ROWS;
    mAt3.numCols = MAT_COLS;
    mAt3.pData = pDataA;
    
    mAt1.numRows = MAT_COLS;
    mAt1.numCols = MAT_ROWS;
    mAt1.pData = pDataA_temp1;
    
    if(arm_mat_trans_f32(&mAt3, &mAt1) != ARM_MATH_SUCCESS) {
        APP_ERROR("mat\r\n");
        return;
    }
    mat_put("A'", mAt1.numRows, mAt1.numCols, pDataA_temp1);
    /*****************************************************************/
    mAt2.numRows = MAT_COLS;
    mAt2.numCols = MAT_COLS;
    mAt2.pData = pDataA_temp2;
    
    if(arm_mat_mult_f32(&mAt1, &mAt3, &mAt2) != ARM_MATH_SUCCESS) {
        APP_ERROR("mat\r\n");
        return;
    }
    mat_put("A' * A", mAt2.numRows,  mAt2.numCols, pDataA_temp2);
    /*****************************************************************/
    mAt3.numRows = MAT_COLS;
    mAt3.numCols = MAT_COLS;
    mAt3.pData = pDataA_temp3;
    
    if(arm_mat_inverse_f32(&mAt2, &mAt3) != ARM_MATH_SUCCESS) {
        APP_ERROR("mat\r\n");
        return;
    }
    mat_put("inv(A' * A)", mAt3.numRows, mAt3.numCols, pDataA_temp3);
    /*****************************************************************/
    mAt2.numRows = MAT_COLS;
    mAt2.numCols = MAT_ROWS;
    mAt2.pData = pDataA_temp2;
    
    if(arm_mat_mult_f32(&mAt3, &mAt1, &mAt2) != ARM_MATH_SUCCESS) {
        APP_ERROR("mat\r\n");
        return;
    }
    mat_put("inv(A' * A) * A'", mAt2.numRows, mAt2.numCols, pDataA_temp2);
    /*****************************************************************/
    mAt3.numRows = MAT_ROWS;
    mAt3.numCols = 1;
    mAt3.pData = pDatab;
    
    mAt1.numRows = MAT_COLS;
    mAt1.numCols = 1;
    mAt1.pData = pDataA_temp1;
    if(arm_mat_mult_f32(&mAt2, &mAt3, &mAt1) != ARM_MATH_SUCCESS) {
        APP_ERROR("mat\r\n");
        return;
    }
    mat_put("inv(A' * A) * A' * b",  mAt1.numRows, mAt1.numCols, pDataA_temp1);
    /*****************************************************************/
    
}




void adc3_receive_proc(int id, void *pbuf, int len)
{
    uint16_t *adc_data = (uint16_t *)pbuf;
    
    if(id == 1) {
        TIMER_TASK(timer1, 1000, 1) {
            
            PRINTF("id = %d \r\n", adc_data[0]);
            printf("sin = %.3f\r\n", arm_sin_f32(PI/180 * 30) );
            
            float out = 0;
            arm_sqrt_f32(PI, &out);
            printf("sqrt = %.3f\r\n", out);
            
            mat();
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
            
    #if TTS_ON
            int len = snprintf(tts_buf, 512, "当前温度%.2f  ", Vtemp_sensor);
            int ret = tts_puts(tts_buf, len);
            if(ret > 0) {
                APP_WARN("TTS OUT: %s\r\n", tts_buf);
            }
    #endif

        }
    }
}



void user_setup(void)
{
    PRINTF("\r\n\r\n[H7] Build , %s %s \r\n", __DATE__, __TIME__);
    
    
    data_interface_hal_init();

#if TTS_ON
    tts_init();
#endif
}




void user_loop(void)
{
    TIMER_TASK(time1, 200,1) {
        LED_REV(LED0_BASE);
    }
//    TIMER_TASK(time2, 600, 1) {
//        flash_test2();
//    }
#if TTS_ON
    TIMER_TASK(time3, 50, 1) {
        tts_proc();
    }
#endif
    TIMER_TASK(time4, 200,1) {
        adc_rx_proc(adc3_receive_proc);
    }
}


/*****************************END OF FILE***************************/
