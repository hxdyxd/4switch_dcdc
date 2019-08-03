/* 2019 04 10 */
/* By hxdyxd */

#include <stdio.h>
#include "usart.h"
#include "ugui.h"



int fputc(int ch, FILE *f)
{
    uint8_t c = ch;
    HAL_UART_Transmit(&huart1, &c, 1, 20);
    return ch;
}

int fgetc(FILE *f)
{
    uint8_t c;
    while( HAL_UART_Receive(&huart1, &c, 1, 600) != HAL_OK);
    return c;
}
