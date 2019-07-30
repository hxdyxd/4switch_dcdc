/* 2019 07 27 */
/* By hxdyxd */
#include <stdio.h>
#include <arm_math.h>
#include <app_debug.h>

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

