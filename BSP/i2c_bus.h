#ifndef __I2C_BUS_H
#define __I2C_BUS_H

#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct
{
    I2C_HandleTypeDef *hi2c;
    SemaphoreHandle_t mutex;
} I2C_Bus_t;


/* 初始化 */
HAL_StatusTypeDef I2C_Bus_Init(I2C_Bus_t *bus, I2C_HandleTypeDef *hi2c);

/* 普通收发 */
HAL_StatusTypeDef I2C_Write(I2C_Bus_t *bus,
                            uint16_t dev_addr,
                            uint8_t *data,
                            uint16_t len);

HAL_StatusTypeDef I2C_Read(I2C_Bus_t *bus,
                           uint16_t dev_addr,
                           uint8_t *data,
                           uint16_t len);

/* 寄存器访问 */
HAL_StatusTypeDef I2C_Mem_Write(I2C_Bus_t *bus,
                                uint16_t dev_addr,
                                uint16_t reg,
                                uint16_t reg_size,
                                uint8_t *data,
                                uint16_t len);

HAL_StatusTypeDef I2C_Mem_Read(I2C_Bus_t *bus,
                               uint16_t dev_addr,
                               uint16_t reg,
                               uint16_t reg_size,
                               uint8_t *data,
                               uint16_t len);

#endif