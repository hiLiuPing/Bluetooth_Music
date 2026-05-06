#ifndef __AHT20_H
#define __AHT20_H

#include "main.h"

// I2C 地址（HAL库必须左移1位）
#define AHT20_ADDR        (0x38 << 1)

// 返回值定义
#define AHT20_OK          0
#define AHT20_ERROR       1

/* 函数声明 */
// 初始化AHT20（传入使用的I2C端口句柄，如 &hi2c1）
uint8_t AHT20_Init(I2C_HandleTypeDef *hi2c);

// 读取温湿度
uint8_t AHT20_Read(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity);

#endif