#ifndef __OLED_U8G2_H
#define __OLED_U8G2_H

#include "main.h"
#include "u8g2.h"

/* 配置：根据你的屏幕修改 */
#define OLED_I2C_ADDR      (0x3C << 1)
#define OLED_WIDTH         128
#define OLED_HEIGHT        32

/* 基础接口 */
void OLED_Init(I2C_HandleTypeDef *hi2c);
void OLED_Clear(void);
void OLED_Update(void);
void OLED_DrawStr(uint8_t x, uint8_t y, const char *str);

// /* UI 接口 */
// void OLED_ShowToast(const char *str);
// void OLED_UI_Render(void);

/* 获取底层句柄（用于调用原生 u8g2 函数） */
u8g2_t* OLED_GetHandle(void);

#endif