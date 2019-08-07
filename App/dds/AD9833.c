#include "AD9833.h"
#include "spi.h"
#include "data_interface_hal.h"

#include "app_debug.h"




void AD9833_Write(unsigned int TxData)
{
    uint8_t dat[2];
    dat[0] = (TxData >> 8) & 0xff;
    dat[1] = TxData & 0xff;
    
    LED_LOW(DDS_FSY);
    
    hspi4.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
    if (HAL_SPI_Init(&hspi4) != HAL_OK) {
        APP_ERROR("spi4 init error\r\n");
    }
    if(HAL_SPI_Transmit(&hspi4, dat, 2, 200) != HAL_OK) {
        APP_ERROR("spi error\r\n");
    }
    LED_HIGH(DDS_FSY);
}


void AD9833_AmpSet(unsigned char amp)
{
    uint8_t dat[2];
    dat[0] = 0x11;
    dat[1] = amp;
    
    LED_LOW(DDS_CS);
    
    hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
    if (HAL_SPI_Init(&hspi4) != HAL_OK) {
        APP_ERROR("spi4 init error\r\n");
    }
    if(HAL_SPI_Transmit(&hspi4, dat, 2, 200) != HAL_OK) {
        APP_ERROR("spi error\r\n");
    }
    LED_HIGH(DDS_CS);
}


/*
  Command: set the waveform and frequency (Hz)
 */
void setWave(int waveform, long frequency)
{
    long freq_data = frequency * pow(2, 28) / REF_FREQ;
    int freq_MSB = (int)(freq_data >> 14) | FREQ0;
    int freq_LSB = (int)(freq_data & 0x3FFF) | FREQ0;



    
    AD9833_Write(CR_B28_COMBINED | CR_FSELECT_0 | CR_PSELECT_0 | CR_RESET);
    AD9833_Write(freq_LSB);
    AD9833_Write(freq_MSB);
    AD9833_Write(PHASE0);
    AD9833_Write(waveform);
    
}

/*****************************END OF FILE***************************/
