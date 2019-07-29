#ifndef _APP_DEBUG_H
#define _APP_DEBUG_H

#include <stdio.h>
#include "stm32h7xx_hal.h"


#define BLUE_FONT "\033[40;34m%s\033[0m "
#define RED_FONT "\033[40;31m%s\033[0m "
#define GREEN_FONT "\033[40;32m%s\033[0m "
#define YELLOW_FONT "\033[40;33m%s\033[0m "
#define PURPLE_FONT "\033[40;35m%s\033[0m "
#define DGREEN_FONT "\033[40;36m%s\033[0m "
#define WHITE_FONT "\033[40;37m%s\033[0m "


#define TIME_COUNT()  (HAL_GetTick())


#define PRINTF(...) printf(__VA_ARGS__)

#define APP_ERROR(...) do {\
                    printf("\033[40;32m[%d]\033[0m \033[2;40;33m%s(%d)\033[0m: ",\
                     TIME_COUNT(), __FUNCTION__, __LINE__);\
                    printf("\033[1;40;31mERROR\033[0m ");\
                    printf(__VA_ARGS__);\
                    }while(0)

#define APP_WARN(...) do {\
                    printf("\033[40;32m[%d]\033[0m \033[2;40;33m%s(%d)\033[0m: ",\
                    TIME_COUNT(), __FUNCTION__, __LINE__);\
                    printf("\033[1;40;33mWARN\033[0m ");\
                    printf(__VA_ARGS__);\
                    }while(0)

#define APP_DEBUG(...) do {\
                    printf("\033[40;32m[%d]\033[0m \033[2;40;33m%s(%d)\033[0m: ",\
                     TIME_COUNT(), __FUNCTION__, __LINE__);\
                    printf(__VA_ARGS__);\
                    }while(0)


#endif
