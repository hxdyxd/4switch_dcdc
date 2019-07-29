/* 
 *
 *2019 07 19 & hxdyxd
 *
 */
#include "tts.h"
#include "kfifo.h"
#include "app_debug.h"

#define ERR_LOG  APP_ERROR



#define TTS_BUFFER_SIZE  (128)


extern int gbk_tts_table(uint16_t *word, unsigned char const **pbuf);

#define PCM_BUFFER_SIZE   (32000)
static volatile uint16_t ALIGN_32BYTES(pcm[PCM_BUFFER_SIZE]);
static volatile uint8_t gflag = 1;
static volatile uint8_t tts_underflow = 0;

static struct __kfifo tts_fifo;



void tts_init(void)
{
    static uint16_t kfifo_buff[TTS_BUFFER_SIZE];
    if(__kfifo_init(&tts_fifo, kfifo_buff, sizeof(kfifo_buff), 2) < 0) {
        ERR_LOG("kfifo init failed\r\n");
    }
}

int tts_puts(char *str, int len)
{
    if(tts_underflow) {
        uint8_t word[2];
        for(int i = 0; i < len; i++) {
            if(str[i]>>7) {
                //gbk
                __kfifo_in(&tts_fifo, &str[i++], 1);
            } else {
                //ascii
                word[0] = str[i];
                word[1] = 0;
                __kfifo_in(&tts_fifo, &word, 1);
            }
        }
        tts_underflow = 0;
        return len;
    } else {
        return -1;
    }
}



void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    gflag = 1;
    //APP_DEBUG("dac cb\r\n");
}


void tts_proc(void)
{
    if(!gflag) {
        return;
    }
    const unsigned char *pbuf = NULL;
    const char *str[] = {"零", "一", "二", "三", "四", "五", "六", "七", "八", "九", };
    uint8_t word[2];
    if(__kfifo_out(&tts_fifo, &word, 1) != 1) {
        word[0] = 0;
        word[1] = 0;
        tts_underflow = 1;
        return;
    }
    
    if(word[1] == 0 && word[0] <= '9' && word[0] >= '0') {
        word[1] = str[word[0] - 48][1];
        word[0] = str[word[0] - 48][0];
    } else if(word[1] == 0 && word[0] == '.') {
        word[1] = "点"[1];
        word[0] = "点"[0];
    }
    
    int len;
    len = gbk_tts_table((void *)word, &pbuf);
    
    len /= 2;
    
    printf("tester: %d %d\r\n", len, *pbuf);
    
    int16_t *s16buf = (int16_t *)pbuf;
    for(int i=0; i<PCM_BUFFER_SIZE; i++) {
        if(i < len) {
            pcm[i] = s16buf[i] ^ 0x8000;
        } else {
            pcm[i] = 0x8000;
        }
    }
    SCB_CleanDCache_by_Addr((uint32_t *)pcm, sizeof(pcm));
    
    
    if(len == 0) {
        len = 4000;
    }
    //len = len;
    len = (len > PCM_BUFFER_SIZE?PCM_BUFFER_SIZE:len);
    
    
    if(HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)pcm, len, DAC_ALIGN_12B_L) == HAL_OK) {
        gflag = 0;
        APP_DEBUG("start dac1 dma at 0x%p \r\n", pcm);
    } else {
        APP_ERROR("start dac1 dma error\r\n");
    }
}

/*****************************END OF FILE***************************/

