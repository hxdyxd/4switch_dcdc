/* 2019 07 14 */
/* By hxdyxd */
#include <qspi_flash.h>



#define W25X_WriteEnable        0x06
#define W25X_WriteDisable       0x04

static void qspi_flash_write_enable(int enable)
{
    QSPI_CommandTypeDef s_command;
    
    /* set address */
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    /* set alternate */
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
     /* set data */
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    
    
    if(enable) {
        /* set instruction */
        s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        s_command.Instruction       = W25X_WriteEnable;
    } else {
        /* set instruction */
        s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        s_command.Instruction       = W25X_WriteDisable;
    }
    
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return;
    }
}


#define W25X_ReadStatusReg1      0x05
#define W25X_ReadStatusReg2      0x35
#define W25X_WriteStatusReg      0x01
#define WIP_Flag                 0x01  /* Write In Progress (WIP) flag */

static void qspi_flash_wait_for_write(void)
{
    QSPI_CommandTypeDef s_command;
    uint8_t status_reg = 0;
    
    /* read data */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_ReadStatusReg1;
    
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 1;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    
    uint32_t timeout = QSPI_GET_TICK();
    do {
        if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            APP_ERROR("command error\r\n");
            return;
        }
        
        if (HAL_QSPI_Receive(&hqspi, &status_reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            APP_ERROR("receive error\r\n");
            return;
        }
        
        if( QSPI_GET_TICK() - timeout >= QSPI_WAIT_TIMEOUT) {
            return;
        }
            
    } while(status_reg & WIP_Flag);
}


void qspi_flash_enable_qe(void)
{
    uint8_t status_reg[2];
    QSPI_CommandTypeDef s_command;
    
    /* read data */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_ReadStatusReg1;
    
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 1;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
 
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return;
    }
    
    if (HAL_QSPI_Receive(&hqspi, &status_reg[0], HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("receive error\r\n");
        return;
    }
    
    s_command.Instruction       = W25X_ReadStatusReg2;
    
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return;
    }
    
    if (HAL_QSPI_Receive(&hqspi, &status_reg[1], HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("receive error\r\n");
        return;
    }
    
    printf("read status reg %2x %2x \r\n", status_reg[0], status_reg[1]);
    if(status_reg[1] & 0x2) {
        APP_DEBUG("already QE = 1\r\n");
    } else {
        status_reg[1] |= 0x2;
        qspi_flash_write_enable(1);
        qspi_flash_wait_for_write();
        
        s_command.Instruction       = W25X_WriteStatusReg;
        
        s_command.DataMode          = QSPI_DATA_1_LINE;
        s_command.DummyCycles       = 0;
        s_command.NbData            = 2;
        
        if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            APP_ERROR("command error\r\n");
            return;
        }
        
        if (HAL_QSPI_Transmit(&hqspi, status_reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            APP_ERROR("Transmit error\r\n");
            return;
        }
        
        printf("write status reg %2x %2x \r\n", status_reg[0], status_reg[1]);
        qspi_flash_wait_for_write();
        qspi_flash_write_enable(0);
    }
}



#define W25X_EnableReset        0x66
#define W25X_ResetDevice        0x99

void qspi_flash_reset(void)
{
    QSPI_CommandTypeDef s_command;
    
    /* set instruction */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_EnableReset;
    /* set address */
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    /* set alternate */
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
     /* set data */
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
 
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return;
    }
    
    s_command.Instruction       = W25X_EnableReset;
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return;
    }
}



#define W25X_DeviceID                0xAB
#define W25X_JedecDeviceID           0x9F
#define W25X_ManufactDeviceID        0x90
#define W25X_QSPI_ManufactDeviceID   0x94

uint32_t qspi_flash_jedec_id(void)
{
    QSPI_CommandTypeDef s_command;
    uint8_t idData[5];
    
    /* Read JEDEC ID */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_JedecDeviceID;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 3;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
 
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return 0;
    }
    
    if (HAL_QSPI_Receive(&hqspi, idData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("receive error\r\n");
        return 0;
    }
    
    printf("Read JEDEC ID :  %2X %2X %2X\r\n", idData[0], idData[1], idData[2]);
    return 0;
}


uint32_t qspi_flash_device_id(void)
{
    QSPI_CommandTypeDef s_command;
    uint8_t idData[5];
    
    /* Read JEDEC ID */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_ManufactDeviceID;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 3*8;
    s_command.NbData            = 2;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
 
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return 0;
    }
    
    if (HAL_QSPI_Receive(&hqspi, idData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("receive error\r\n");
        return 0;
    }
    
    printf("Read device ID by SPI : %2X %2X\r\n", idData[0], idData[1]);
    return 0;
}


uint32_t qspi_flash_device_id_by_qspi(void)
{
    QSPI_CommandTypeDef s_command;
    uint8_t idData[8];
    
    /* Read devixe ID */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_QSPI_ManufactDeviceID;
    
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.Address           = 0x000000;
    
    s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    s_command.AlternateBytes     = 0xef;
    
    s_command.DataMode          = QSPI_DATA_4_LINES;
    s_command.DummyCycles       = 4;
    s_command.NbData            = 6;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return 0;
    }
    
    if (HAL_QSPI_Receive(&hqspi, idData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("receive error\r\n");
        return 0;
    }
    
    printf("Read device ID by QSPI : %2X %2X %2X %2X %2X %2X\r\n",
        idData[0], idData[1], idData[2], idData[3], idData[4], idData[5]);
    return 0;
}



#define W25X_ReadData           0x03
#define W25X_FastReadData       0x0B
#define W25X_PageProgram        0x02
#define W25X_BlockErase         0xD8
#define W25X_SectorErase        0x20
#define W25X_ChipErase          0xC7
#define W25X_PowerDown          0xB9
#define W25X_ReleasePowerDown   0xAB


//SectorErase (4KB)
uint32_t qspi_flash_4k_sector_erase(uint32_t address)
{
    QSPI_CommandTypeDef s_command;
    qspi_flash_write_enable(1);
    qspi_flash_wait_for_write();
    
    /* program data */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_SectorErase;
    
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.Address           = address;
    
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
 
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return 0;
    }
    
    qspi_flash_wait_for_write();
    qspi_flash_write_enable(0);
    return 1;
}



uint32_t qspi_flash_read(uint32_t address, void *pdata, uint32_t length)
{
    QSPI_CommandTypeDef s_command;
    
    /* read data */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_ReadData;
    
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.Address           = address;
    
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = length;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
 
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return 0;
    }
    
    if (HAL_QSPI_Receive(&hqspi, pdata, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("receive error\r\n");
        return 0;
    }
    return length;
}


uint32_t qspi_flash_write(uint32_t address, void *pdata, uint32_t length)
{
    QSPI_CommandTypeDef s_command;
    qspi_flash_write_enable(1);
    qspi_flash_wait_for_write();
    
    /* program data */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_PageProgram;
    
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.Address           = address;
    
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = length;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
 
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return 0;
    }
    
    if (HAL_QSPI_Transmit(&hqspi, pdata, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("transmit error\r\n");
        return 0;
    }
    
    qspi_flash_wait_for_write();
    qspi_flash_write_enable(0);
    return length;
}



#define  W25X_DSPI_FastReadData   (0x3b)
#define  W25X_DSPI_FastReadDataIO (0xbb)
#define  W25X_QSPI_FastReadData   (0x6b)
#define  W25X_QSPI_FastReadDataIO (0xeb)

uint32_t qspi_flash_read_by_qspi(uint32_t address, void *pdata, uint32_t length)
{
    QSPI_CommandTypeDef s_command;
    
    /* read data */
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_QSPI_FastReadData;
    
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.Address           = address;
    
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    
    s_command.DataMode          = QSPI_DATA_4_LINES;
    s_command.DummyCycles       = 8;
    s_command.NbData            = length;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
 
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("command error\r\n");
        return 0;
    }
    
    if (HAL_QSPI_Receive(&hqspi, pdata, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
        APP_ERROR("receive error\r\n");
        return 0;
    }
    return length;
}


int qspi_flash_memory_maped(void)
{
    QSPI_CommandTypeDef s_command;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_FastReadData;
    
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 8;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    
    
    QSPI_MemoryMappedTypeDef cfg;
    cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    
    return (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &cfg) == HAL_OK);
}


int qspi_flash_memory_maped_by_dspi(void)
{
    QSPI_CommandTypeDef s_command;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_DSPI_FastReadDataIO;
    
    s_command.AddressMode       = QSPI_ADDRESS_2_LINES;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    
    s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_2_LINES;
    /* Fast Read Dual I/O with ¡°Continuous Read Mode¡± */
    s_command.AlternateBytes     = 0x20;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    
    s_command.DataMode          = QSPI_DATA_2_LINES;
    s_command.DummyCycles       = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    
    
    QSPI_MemoryMappedTypeDef cfg;
    cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    
    return (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &cfg) == HAL_OK);
}


int qspi_flash_memory_maped_by_qspi(void)
{
    QSPI_CommandTypeDef s_command;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = W25X_QSPI_FastReadDataIO;
    
    s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    
    s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
    s_command.AlternateBytes     = 0x00;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    
    s_command.DataMode          = QSPI_DATA_4_LINES;
    s_command.DummyCycles       = 4;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    
    
    QSPI_MemoryMappedTypeDef cfg;
    cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    
    return (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &cfg) == HAL_OK);
}

/******************* (C) COPYRIGHT 2018 hxdyxd *****END OF FILE****/
