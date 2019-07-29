#include <stdio.h>

const char flash_str[] = "https://github.com/alibaba/AliOS-Things \r\n";


void printf_str(void)
{
    printf("FLASH RUN TEST %s %d\r\n", flash_str, 100*200);
}
