/* 2019 08 01 */
/* By hxdyxd */
#include "lcd240x240.h"
#include "data_interface_hal.h"


#if (USE_LCD_DMA)
ALIGN_32BYTES(static uint8_t disp_mem[LCD_SIZE*2]);
#endif


#if 1
static inline void LCD_Writ_Bus(uint8_t data)
{
    if(HAL_SPI_Transmit(&hspi2, &data, 1, 200) != HAL_OK) {
        printf("spi error\r\n");
    }
}
#else
static inline void LCD_Writ_Bus(uint8_t data)
{
    for(uint8_t i=0; i<8; i++)
    {
        LED_LOW(LCD_CLK);
        if(data & 0x80)
           LED_HIGH(LCD_MOSI);
        else 
           LED_LOW(LCD_MOSI);
        LED_HIGH(LCD_CLK);
        data <<= 1;
    }
}
#endif

static inline void LCD_WR_DATA8(uint8_t da)
{

    LED_HIGH(LCD_DC);
    LCD_Writ_Bus(da);
}

static inline void LCD_WR_DATA(uint16_t da)
{

    LED_HIGH(LCD_DC);
    LCD_Writ_Bus(da >> 8);
    LCD_Writ_Bus(da);
}

static inline void LCD_WR_REG(uint8_t da)
{

    LED_LOW(LCD_DC);
    LCD_Writ_Bus(da);
}


void lcd240x240_init(void)
{
    LCD_WR_REG(0);
    
    LED_HIGH(LCD_RES);
    mdelay(20);
    LED_LOW(LCD_RES);
    mdelay(20);
    LED_HIGH(LCD_RES);
    mdelay(20);
    
    //************* Start Initial Sequence **********// 
    LCD_WR_REG(0x36);
    LCD_WR_DATA8(0x00);

    LCD_WR_REG(0x3A); // pixel format; 5=RGB565
    LCD_WR_DATA8(0x05);

    LCD_WR_REG(0xB2); // Porch control
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33);

    LCD_WR_REG(0xB7);  // gate control
    LCD_WR_DATA8(0x35);

    LCD_WR_REG(0xBB);  // VCOM
    LCD_WR_DATA8(0x19);

    LCD_WR_REG(0xC0);  // LCM
    LCD_WR_DATA8(0x2C);

    LCD_WR_REG(0xC2);  // VDV & VRH command enable
    LCD_WR_DATA8(0x01);

    LCD_WR_REG(0xC3);  // VRH set
    LCD_WR_DATA8(0x12);

    LCD_WR_REG(0xC4);  // VDV set
    LCD_WR_DATA8(0x20);

    LCD_WR_REG(0xC6);  // FR control 2
    LCD_WR_DATA8(0x0F);

    LCD_WR_REG(0xD0);  // Power control 1
    LCD_WR_DATA8(0xA4);
    LCD_WR_DATA8(0xA1);

    LCD_WR_REG(0xE0);  // gamma 1
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2B);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x54);
    LCD_WR_DATA8(0x4C);
    LCD_WR_DATA8(0x18);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x23);

    LCD_WR_REG(0xE1);  // gamma 2
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2C);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x44);
    LCD_WR_DATA8(0x51);
    LCD_WR_DATA8(0x2F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x20);
    LCD_WR_DATA8(0x23);

    LCD_WR_REG(0x21);  //display inversion on

    LCD_WR_REG(0x11);  // sleep out

    LCD_WR_REG(0x29);  // display on
    
}


void lcd240x240_display(int on)
{
    if(on)
        LCD_WR_REG(0x29);  // display on
    else
        LCD_WR_REG(0x28);  // display off
}


static inline void lcd240x240_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_WR_REG(0x2a);
    LCD_WR_DATA(x1);
    LCD_WR_DATA(x2);

    LCD_WR_REG(0x2b);
    LCD_WR_DATA(y1);
    LCD_WR_DATA(y2);

    LCD_WR_REG(0x2C);
}

#if !(USE_LCD_DMA)
////清屏函数
////Color:要清屏的填充色
void lcd240x240_clear(uint16_t color)
{
    lcd240x240_address_set(0, 0, LCD_W-1, LCD_H-1);
    for(int i=0;i<LCD_SIZE;i++) {
        LCD_WR_DATA(color);
    }
}


//在指定区域内填充指定颜色
//区域大小:
//  (xend-xsta)*(yend-ysta)
int lcd240x240_fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
    lcd240x240_address_set(xsta, ysta, xend, yend); //设置光标位置
    for(uint16_t i=ysta; i<=yend; i++) {
        for(uint16_t j=xsta; j<=xend; j++) {
            LCD_WR_DATA(color);//设置光标位置
        }
    }
    return 0;
}

//画点
//POINT_COLOR:此点的颜色
void lcd240x240_drawpoint(uint16_t x, uint16_t y, uint16_t color)
{
    lcd240x240_address_set(x, y, x, y);//设置光标位置 
    LCD_WR_DATA(color);
}

#else
//DMA MODE

void lcd240x240_clear(uint16_t color)
{
    for(int i=0; i<LCD_SIZE; i++) {
        disp_mem[i*2] = color >> 8;
        disp_mem[i*2+1] = color;
    }
}


int lcd240x240_fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
    for(uint16_t i=ysta; i<=yend; i++) {
        for(uint16_t j=xsta; j<=xend; j++) {
            int local = i*LCD_W + j;
            
            disp_mem[local*2] = color >> 8;
            disp_mem[local*2+1] = (color);
        }
    }
    return 0;
}


void lcd240x240_drawpoint(uint16_t x, uint16_t y, uint16_t color)
{
    int local = y*LCD_W + x;
    disp_mem[local*2] = color >> 8;
    disp_mem[local*2+1] = (color);
}




static volatile int current_buffer = -1;

int lcd240x240_flush(void)
{
    if(current_buffer >= 0) {
        return -1;
    }
    current_buffer = 0;
    lcd240x240_address_set(0, 0, LCD_W-1, LCD_H-1);
    LED_HIGH(LCD_DC);
    SCB_CleanDCache_by_Addr( (uint32_t *)disp_mem, sizeof(disp_mem));
    SCB_InvalidateDCache_by_Addr( (uint32_t *)disp_mem, sizeof(disp_mem));
    
    HAL_SPI_Transmit_DMA(&hspi2, (uint8_t *)disp_mem, LCD_SIZE);
    return 0;
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(current_buffer == 0) {
        
        current_buffer++;
        SCB_CleanDCache_by_Addr( (uint32_t *)disp_mem, sizeof(disp_mem));
        SCB_InvalidateDCache_by_Addr( (uint32_t *)disp_mem, sizeof(disp_mem));

        HAL_SPI_Transmit_DMA(&hspi2, (uint8_t *)disp_mem + LCD_SIZE, LCD_SIZE);
    } else {
        current_buffer = -1;
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    printf("spi error \r\n");
}
#endif

/*****************************END OF FILE***************************/
