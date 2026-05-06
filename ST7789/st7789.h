#ifndef __ST7789_H
#define __ST7789_H


#include "fonts.h"
#include "main.h"

/* SPI */
#define ST7789_SPI_PORT hspi1
extern SPI_HandleTypeDef ST7789_SPI_PORT;

/* LCD resolution */
#define ST7789_WIDTH  320
#define ST7789_HEIGHT 240

/* rotation */
#define ST7789_ROTATION 1

/* offset */
#define X_SHIFT 0
#define Y_SHIFT 0

/* pins */
#define ST7789_RST_PORT GPIOC
#define ST7789_RST_PIN  GPIO_PIN_5
#define ST7789_DC_PORT  GPIOC
#define ST7789_DC_PIN   GPIO_PIN_4




// #define SPI1_CS_Pin GPIO_PIN_4
// #define SPI1_CS_GPIO_Port GPIOA
// #define SPI1_PWM_Pin GPIO_PIN_6
// #define SPI1_PWM_GPIO_Port GPIOA
// #define SPI1_DC_Pin GPIO_PIN_4
// #define SPI1_DC_GPIO_Port GPIOC
// #define SPI1_RST_Pin GPIO_PIN_5
// #define SPI1_RST_GPIO_Port GPIOC



/* pin macros */
#define ST7789_RST_Clr() HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_RESET)
#define ST7789_RST_Set() HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_SET)
#define ST7789_DC_Clr() HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET)
#define ST7789_DC_Set() HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET)
#define LCD_CS_LOW()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define LCD_CS_HIGH() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)

/* colors */
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define RED         0xF800
#define GREEN       0x07E0
// #define CYAN        0x07FF
#define YELLOW      0xFFE0
#define MAGENTA     0xF81F

/* commands */
#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT  0x11
#define ST7789_COLMOD  0x3A
#define ST7789_MADCTL  0x36
#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_DISPON  0x29
#define ST7789_INVON   0x21

/* color order */
#define ST7789_COLOR_ORDER_RGB 0
#define ST7789_COLOR_ORDER_BGR 1

/* functions */
void ST7789_Init(void);
void ST7789_SetRotation(uint8_t r);
void ST7789_SetColorOrder(uint8_t order);

void ST7789_Fill_Color(uint16_t color);
void ST7789_DrawPixel(uint16_t x,uint16_t y,uint16_t color);
void ST7789_DrawImage(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t *data);
// void ST7789_WriteChar(uint16_t x,uint16_t y,char ch,FontDef font,uint16_t color,uint16_t bgcolor);
// void ST7789_WriteString(uint16_t x,uint16_t y,const char *str,FontDef font,uint16_t color,uint16_t bgcolor);
void lvgl_st7789_flush(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color_buf);
#endif