#include "AHT20.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* AHT20 命令定义 */
#define CMD_INIT          0xE1    // 初始化命令
#define CMD_TRIGGER       0xAC    // 触发测量
#define CMD_SOFTRESET     0xBA    // 软复位

/**
  * @brief  I2C 发送命令
  * @param  hi2c: I2C句柄
  * @param  cmd: 命令字
  * @param  data: 数据
  * @param  len: 数据长度
  * @retval HAL_StatusTypeDef
  */
static HAL_StatusTypeDef AHT20_WriteCmd(I2C_HandleTypeDef *hi2c, uint8_t cmd, uint8_t *data, uint8_t len)
{
    uint8_t buf[3];
    buf[0] = cmd;

    if(len > 0)
    {
        memcpy(&buf[1], data, len);
    }

    return HAL_I2C_Master_Transmit(hi2c, AHT20_ADDR, buf, len + 1, 100);
}

/**
  * @brief  AHT20初始化
  * @param  hi2c: 使用的I2C端口（如 &hi2c1）
  * @retval AHT20_OK / AHT20_ERROR
  */
uint8_t AHT20_Init(I2C_HandleTypeDef *hi2c)
{
    // 软复位
    if(AHT20_WriteCmd(hi2c, CMD_SOFTRESET, NULL, 0) != HAL_OK)
        return AHT20_ERROR;

    HAL_Delay(20);  // 复位等待时间

    // 发送初始化命令 0xE1 0x08 0x00
    uint8_t init_data[2] = {0x08, 0x00};
    if(AHT20_WriteCmd(hi2c, CMD_INIT, init_data, 2) != HAL_OK)
        return AHT20_ERROR;

    HAL_Delay(40);  // 初始化稳定时间

    return AHT20_OK;
}

/**
  * @brief  读取温湿度值
  * @param  hi2c: I2C句柄
  * @param  temperature: 温度指针
  * @param  humidity: 湿度指针
  * @retval AHT20_OK / AHT20_ERROR
  */
uint8_t AHT20_Read(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity)
{
    uint8_t cmd_data[2] = {0x33, 0x00};
    uint8_t buf[6] = {0};

    // 触发测量
    if(AHT20_WriteCmd(hi2c, CMD_TRIGGER, cmd_data, 2) != HAL_OK)
        return AHT20_ERROR;

    HAL_Delay(80);  // 等待测量完成

    // 读取6字节数据
    if(HAL_I2C_Master_Receive(hi2c, AHT20_ADDR | 1, buf, 6, 100) != HAL_OK)
        return AHT20_ERROR;

    // 检测忙状态位
    if((buf[0] & 0x80) != 0)
        return AHT20_ERROR;

    // ===================== 计算湿度 =====================
    uint32_t hum_raw = ((uint32_t)buf[1] << 12) | ((uint32_t)buf[2] << 4) | (buf[3] >> 4);
    *humidity = (hum_raw * 100.0f) / 1048576.0f;

    // ===================== 计算温度 =====================
    uint32_t temp_raw = ((uint32_t)(buf[3] & 0x0F) << 16) | ((uint32_t)buf[4] << 8) | buf[5];
    *temperature = (temp_raw * 200.0f) / 1048576.0f - 50.0f;

    return AHT20_OK;
}