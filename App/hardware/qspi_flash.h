/* 2019 07 14 */
/* By hxdyxd */

#ifndef _QSPI_FLASH_H
#define _QSPI_FLASH_H


#include <quadspi.h>
#include <app_debug.h>
#include "data_interface_hal.h"

#define QSPI_WAIT_TIMEOUT      5000

#define  QSPI_GET_TICK()     hal_read_TickCounter()



void qspi_flash_reset(void);
uint32_t qspi_flash_jedec_id(void);
uint32_t qspi_flash_device_id(void);
uint32_t qspi_flash_device_id_by_qspi(void);

int qspi_flash_memory_maped(void);
uint32_t qspi_flash_read(uint32_t address, void *pdata, uint32_t length);
uint32_t qspi_flash_write(uint32_t address, void *pdata, uint32_t length);
uint32_t qspi_flash_4k_sector_erase(uint32_t address);


int qspi_flash_memory_maped_by_dspi(void);


int qspi_flash_memory_maped_by_qspi(void);
uint32_t qspi_flash_read_by_qspi(uint32_t address, void *pdata, uint32_t length);
void qspi_flash_enable_qe(void);




#endif
/******************* (C) COPYRIGHT 2018 hxdyxd *****END OF FILE****/
