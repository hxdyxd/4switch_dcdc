#ifndef _EEPROM_H
#define _EEPROM_H

#include "data_interface_hal.h"

int eeprom_write(uint16_t addr, uint8_t *buf, int len);
int eeprom_read(uint16_t addr, uint8_t *buf, int len);


#endif
/*********************************ADC**************************************************/
