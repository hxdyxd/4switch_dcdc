/* 2019 08 05 */
/* By hxdyxd */
#include "key_inout.h"
#include "gpio.h"
#include "data_interface_hal.h"

#define  KEY_TIMEOUT   (50)
#define KEY_IO_OUT_NUM   (2)
#define KEY_IO_IN_NUM    (4)

#define KEY_IO_NUM  (KEY_IO_OUT_NUM + KEY_IO_IN_NUM)

/*******************************/
#define  GPIO_READ(port, pin)              HAL_GPIO_ReadPin(port, pin)
#define  GPIO_WRITE(port, pin, value)      HAL_GPIO_WritePin(port, pin, (value==0)?(GPIO_PIN_RESET):(GPIO_PIN_SET))
#define  KEY_TICK()                        hal_read_TickCounter()

struct tGpio {
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
};

struct tGpio KEY_4_4[KEY_IO_NUM] = {
    {
        .GPIOx = KO1_GPIO_Port,
        .GPIO_Pin = KO1_Pin,
    },
    {
        .GPIOx = KO3_GPIO_Port,
        .GPIO_Pin = KO3_Pin,
    },
    
    {
        .GPIOx = KI0_GPIO_Port,
        .GPIO_Pin = KI0_Pin,
    },
    {
        .GPIOx = KI1_GPIO_Port,
        .GPIO_Pin = KI1_Pin,
    },
    {
        .GPIOx = KI2_GPIO_Port,
        .GPIO_Pin = KI2_Pin,
    },
    {
        .GPIOx = KI3_GPIO_Port,
        .GPIO_Pin = KI3_Pin,
    },
};
/********************************/

void key_inout_init(void)
{
    printf("key inout init\r\n");
}



void key_inout_proc(KEY_INOUT_CB_T key_func)
{
    static int8_t last_key_val = -1;
    static int8_t key_val = -1;
    static uint32_t timer = 0;
    uint8_t i, j;
    uint8_t key_find = 0;
    for(i=0;i<KEY_IO_OUT_NUM;i++) {
        GPIO_WRITE( KEY_4_4[i].GPIOx , KEY_4_4[i].GPIO_Pin, 0);   //low
        
        for( j=KEY_IO_OUT_NUM;j<KEY_IO_NUM;j++ ) {
            if( GPIO_READ( KEY_4_4[j].GPIOx , KEY_4_4[j].GPIO_Pin ) == GPIO_PIN_RESET ) {
                
                last_key_val = key_val;
                key_val = (j - KEY_IO_OUT_NUM) + (i * KEY_IO_IN_NUM);
                key_find = 1;
                if(last_key_val != key_val) {
                    timer = KEY_TICK();
                }
                
                GPIO_WRITE( KEY_4_4[i].GPIOx , KEY_4_4[i].GPIO_Pin, 1);   //high
                break;
            }
        }
        
        GPIO_WRITE( KEY_4_4[i].GPIOx , KEY_4_4[i].GPIO_Pin, 1);   //high
    }

    if(!key_find) {
        key_val = -1;
    }


    static int8_t last_key_val_to_func = -1;

    if( KEY_TICK() - timer > KEY_TIMEOUT ) {
        
        if(key_val != -1 && last_key_val_to_func != key_val) {
            key_func(key_val);
        }
        
        last_key_val_to_func = key_val;
    }
}

/*****************************END OF FILE***************************/
