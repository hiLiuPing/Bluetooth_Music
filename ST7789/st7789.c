#include "st7789.h"

/* ?????? */
static uint8_t color_order = ST7789_COLOR_ORDER_BGR;

/* ??? */
static void ST7789_WriteCommand(uint8_t cmd)
{
    LCD_CS_LOW();
    ST7789_DC_Clr();
    HAL_SPI_Transmit(&ST7789_SPI_PORT, &cmd, 1, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

/* ??? */
static void ST7789_WriteData(uint8_t *data, uint32_t size)
{
    LCD_CS_LOW();
    ST7789_DC_Set();
    HAL_SPI_Transmit(&ST7789_SPI_PORT, data, size, HAL_MAX_DELAY);
    LCD_CS_HIGH();
}

/* ?????? */
static void ST7789_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t data[4];

    uint16_t xs = x0 + X_SHIFT;
    uint16_t xe = x1 + X_SHIFT;
    uint16_t ys = y0 + Y_SHIFT;
    uint16_t ye = y1 + Y_SHIFT;

    ST7789_WriteCommand(ST7789_CASET);
    data[0] = xs >> 8; data[1] = xs & 0xFF;
    data[2] = xe >> 8; data[3] = xe & 0xFF;
    ST7789_WriteData(data, 4);

    ST7789_WriteCommand(ST7789_RASET);
    data[0] = ys >> 8; data[1] = ys & 0xFF;
    data[2] = ye >> 8; data[3] = ye & 0xFF;
    ST7789_WriteData(data, 4);

    ST7789_WriteCommand(ST7789_RAMWR);
}

/* ?????? */
void ST7789_SetRotation(uint8_t r)
{
    uint8_t madctl = 0;

    switch(r % 4)
    {
        case 0: madctl = 0x00; break; // 0?
        case 1: madctl = 0x60; break; // 90?
        case 2: madctl = 0xC0; break; // 180?
        case 3: madctl = 0xA0; break; // 270?
    }

    if(color_order == ST7789_COLOR_ORDER_BGR)
        madctl |= 0x08; // BGR?

    ST7789_WriteCommand(ST7789_MADCTL);
    ST7789_WriteData(&madctl, 1);
}

/* ?????? RGB/BGR */
void ST7789_SetColorOrder(uint8_t order)
{
    color_order = order ? ST7789_COLOR_ORDER_BGR : ST7789_COLOR_ORDER_RGB;
    ST7789_SetRotation(ST7789_ROTATION);
}

/* ????? */
//void ST7789_Init(void)
//{
//    HAL_Delay(10);

//    ST7789_RST_Clr();
//    HAL_Delay(100);
//    ST7789_RST_Set();
//    HAL_Delay(200);

//    ST7789_WriteCommand(ST7789_SWRESET);
//    HAL_Delay(150);

//    ST7789_WriteCommand(ST7789_SLPOUT);
//    HAL_Delay(120);

//    ST7789_WriteCommand(ST7789_COLMOD);
//    uint8_t color = 0x55; // 16bit
//    ST7789_WriteData(&color, 1);

//    ST7789_SetRotation(ST7789_ROTATION);

//    ST7789_WriteCommand(ST7789_INVON);
//    ST7789_WriteCommand(ST7789_DISPON);
//    HAL_Delay(20);

//    ST7789_Fill_Color(BLACK);
//}
void ST7789_Init(void)
{
    // 1. ????
    ST7789_RST_Set();
    HAL_Delay(10);
    ST7789_RST_Clr();
    HAL_Delay(50);      // ????
    ST7789_RST_Set();
    HAL_Delay(120);     // ?????????

    // 2. ????
    ST7789_WriteCommand(ST7789_SWRESET);
    HAL_Delay(150);

    // 3. ????
    ST7789_WriteCommand(ST7789_SLPOUT);
    HAL_Delay(120);     // ????????? 120ms ???????

    // 4. ?????? (COLMOD)
    ST7789_WriteCommand(ST7789_COLMOD);
    uint8_t color_format = 0x55; // 16-bit/pixel (RGB565)
    ST7789_WriteData(&color_format, 1);

    // 5. ???????? (MADCTL)
    // ??????,?????? RGB ?? (bit3 = 0)
    // ??????????,??? ST7789_COLOR_ORDER_BGR
    ST7789_SetColorOrder(ST7789_COLOR_ORDER_RGB);
    ST7789_SetRotation(ST7789_ROTATION);

    // 6. ?????? (Inversion Control)
    // ??? INVON (0x21),???? INVOFF (0x20) ??????
    ST7789_WriteCommand(0x20); 

    // 7. ?????? (????)
    ST7789_SetAddressWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    // 8. ????
    ST7789_WriteCommand(ST7789_DISPON);
    HAL_Delay(20);

    // 9. ?? (??????)
    ST7789_Fill_Color(BLACK);
}
/* ???? */
void ST7789_Fill_Color(uint16_t color)
{
    uint8_t buffer[512];
    for(int i = 0; i < 256; i++)
    {
        buffer[i*2] = color >> 8;
        buffer[i*2+1] = color & 0xFF;
    }

    ST7789_SetAddressWindow(0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1);

    uint32_t pixels = ST7789_WIDTH * ST7789_HEIGHT;
    while(pixels)
    {
        uint16_t chunk = pixels > 256 ? 256 : pixels;
        ST7789_WriteData(buffer, chunk*2);
        pixels -= chunk;
    }
}

/* ??? */
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if(x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;

    uint8_t data[2];
    data[0] = color >> 8;
    data[1] = color & 0xFF;

    ST7789_SetAddressWindow(x, y, x, y);
    ST7789_WriteData(data, 2);
}

/* ??? */
void ST7789_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data)
{
    if((x+w) > ST7789_WIDTH || (y+h) > ST7789_HEIGHT) return;

    ST7789_SetAddressWindow(x, y, x+w-1, y+h-1);
    ST7789_WriteData((uint8_t*)data, w*h*2);
}

/* ????? */
// void ST7789_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)
// {
//     uint8_t line[font.width*2];

//     ST7789_SetAddressWindow(x, y, x+font.width-1, y+font.height-1);

//     for(uint16_t i = 0; i < font.height; i++)
//     {
//         uint16_t bits = font.data[(ch-32)*font.height + i];
//         for(uint16_t j = 0; j < font.width; j++)
//         {
//             uint16_t c = (bits << j) & 0x8000 ? color : bgcolor;
//             line[j*2] = c >> 8;
//             line[j*2+1] = c & 0xFF;
//         }
//         ST7789_WriteData(line, font.width*2);
//     }
// }

/* ???? */
// void ST7789_WriteString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
// {
//     while(*str)
//     {
//         ST7789_WriteChar(x, y, *str, font, color, bgcolor);
//         x += font.width;
//         if(x + font.width > ST7789_WIDTH)
//         {
//             x = 0;
//             y += font.height;
//         }
//         str++;
//     }
// }
void lvgl_st7789_flush(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color_buf)
{

    uint32_t size = (xend - xsta + 1) * (yend - ysta + 1);

    // 1. 设置绘制窗口
    ST7789_SetAddressWindow(xsta, ysta, xend, yend);

    // 2. 开始发送像素数据
    /* 拉低CS */
    LCD_CS_LOW();
    ST7789_DC_Set();

    // 注意：LVGL默认是16位RGB565，STM32是小端模式。
    // 如果屏幕显示颜色反转（比如红蓝对调），需要在这里进行字节序交换
    // 或者在 lv_conf.h 中设置 #define LV_COLOR_16_SWAP 1
   HAL_SPI_Transmit(&ST7789_SPI_PORT, (uint8_t *)color_buf, size * 2, HAL_MAX_DELAY);

    /* 释放CS */
    LCD_CS_HIGH();

}