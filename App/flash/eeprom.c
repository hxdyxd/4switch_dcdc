/* 
 *
 *2019 08 04 & hxdyxd
 *
 */

#include "eeprom.h"


#define eeprom_min(xx,yy)  ((xx)<(yy))?(xx):(yy)

int eeprom_write(uint16_t addr, uint8_t *buf, int len)
{
    for(int i=0; i<len; i+=8) {
        uint8_t wsize = eeprom_min(len - i, 8);
        if(HAL_I2C_Mem_Write(&hi2c4, 0xa0, addr, I2C_MEMADD_SIZE_8BIT, buf + i, wsize, 20) != HAL_OK) {
            printf("eeprom write error\r\n");
            return 0;
        }
        HAL_Delay(5);
        addr += wsize;
    }
    
    return 1;
}

int eeprom_read(uint16_t addr, uint8_t *buf, int len)
{
    return (HAL_I2C_Mem_Read(&hi2c4, 0xa1, addr, I2C_MEMADD_SIZE_8BIT, buf, len, 20) == HAL_OK);
}


/*********************************ADC**************************************************/
