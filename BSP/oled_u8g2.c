#include "oled_u8g2.h"
#include <string.h>
// #include "cmsis_os.h" // 包含 FreeRTOS 接口
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <string.h>
#include "u8g2.h"   // 确保包含这个
/* ================= 私有变量 ================= */

static u8g2_t u8g2;
static I2C_HandleTypeDef *oled_i2c;

// 缓冲区大小计算：(128 * 32 / 8) = 512 字节显存 + 指令开销
// 设为 600 字节以确保安全
static uint8_t i2c_buffer[600];
static uint16_t buffer_idx = 0;

/* ================= 底层回调 (U8g2 接口) ================= */

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
        case U8X8_MSG_BYTE_INIT:
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            buffer_idx = 0;
            break;

        case U8X8_MSG_BYTE_SEND:
        {
            uint8_t *data = (uint8_t *)arg_ptr;
            if (buffer_idx + arg_int <= sizeof(i2c_buffer))
            {
                memcpy(&i2c_buffer[buffer_idx], data, arg_int);
                buffer_idx += arg_int;
            }
            break;
        }

        case U8X8_MSG_BYTE_END_TRANSFER:
            // 使用阻塞模式发送，超时时间根据波特率设定
            if (HAL_I2C_Master_Transmit(oled_i2c, OLED_I2C_ADDR, i2c_buffer, buffer_idx, 100) != HAL_OK)
            {
                return 0;
            }
            break;

        default:
            return 0;
    }
    return 1;
}

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
        case U8X8_MSG_DELAY_MILLI:
            // 兼容调度器未启动的情况
            if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
                vTaskDelay(pdMS_TO_TICKS(arg_int));
            else
                HAL_Delay(arg_int);
            break;

        case U8X8_MSG_DELAY_10MICRO:
            // 粗略微秒延时
            for (volatile uint32_t i = 0; i < (SystemCoreClock / 1000000); i++);
            break;

        default:
            return 1;
    }
    return 1;
}

/* ================= 初始化与基础功能 ================= */

void OLED_Init(I2C_HandleTypeDef *hi2c)
{
    oled_i2c = hi2c;

    // 初始化 SSD1312 128x32 I2C 驱动
    u8g2_Setup_ssd1312_i2c_128x32_f(&u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);
    
    u8g2_SetI2CAddress(&u8g2, OLED_I2C_ADDR);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);
}

void OLED_Clear(void)
{
    u8g2_ClearBuffer(&u8g2);
}

void OLED_Update(void)
{
    u8g2_SendBuffer(&u8g2);
}

void OLED_DrawStr(uint8_t x, uint8_t y, const char *str)
{
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, x, y, str);
}

u8g2_t* OLED_GetHandle(void)
{
    return &u8g2;
}

// /* ================= UI 物理动画层 ================= */

// static float toast_y = 35.0f; // 初始位置在屏幕外
// static float velocity = 0.0f;
// static uint8_t toast_active = 0;
// static char toast_text[32] = "";

// void OLED_ShowToast(const char *str)
// {
//     // 安全拷贝字符串
//     strncpy(toast_text, str, sizeof(toast_text) - 1);
//     toast_text[sizeof(toast_text) - 1] = '\0';

//     toast_y = 35.0f;   // 从下方弹出
//     velocity = -5.0f;  // 给一个向上的初速度
//     toast_active = 1;
// }

// void OLED_UI_Render(void)
// {
//     const float target_y = 12.0f; // 弹出停留在中间位置
//     const float stiffness = 0.15f; // 弹簧劲度
//     const float damping = 0.75f;   // 阻尼（越小越不抖）

//     if (toast_active)
//     {
//         // 弹簧物理计算：F = k * Δx
//         float force = (target_y - toast_y) * stiffness;
//         velocity += force;
//         velocity *= damping;
//         toast_y += velocity;
//     }

//     u8g2_ClearBuffer(&u8g2);

//     if (toast_active)
//     {
//         // 绘制圆角矩形背景
//         u8g2_SetDrawColor(&u8g2, 1);
//         u8g2_DrawRBox(&u8g2, 5, (int16_t)toast_y, 118, 16, 3);

//         // 反色显示文字
//         u8g2_SetDrawColor(&u8g2, 0);
//         u8g2_SetFont(&u8g2, u8g2_font_6x12_tf);
//         // 居中计算（大概值）
//         uint16_t str_w = u8g2_GetStrWidth(&u8g2, toast_text);
//         u8g2_DrawStr(&u8g2, (128 - str_w) / 2, (int16_t)toast_y + 12, toast_text);
        
//         u8g2_SetDrawColor(&u8g2, 1); // 恢复画笔颜色
//     }

//     u8g2_SendBuffer(&u8g2);
// }