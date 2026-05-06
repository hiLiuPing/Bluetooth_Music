#include "i2c_bus.h"

HAL_StatusTypeDef I2C_Bus_Init(I2C_Bus_t *bus, I2C_HandleTypeDef *hi2c)
{
    if (bus == NULL || hi2c == NULL)
        return HAL_ERROR;

    bus->hi2c = hi2c;
    bus->mutex = xSemaphoreCreateMutex();

    if (bus->mutex == NULL)
        return HAL_ERROR;

    return HAL_OK;
}


HAL_StatusTypeDef I2C_Write(I2C_Bus_t *bus,
                            uint16_t dev_addr,
                            uint8_t *data,
                            uint16_t len)
{
    if (!bus) return HAL_ERROR;

    xSemaphoreTake(bus->mutex, portMAX_DELAY);

    HAL_StatusTypeDef ret =
        HAL_I2C_Master_Transmit(bus->hi2c,
                                dev_addr,
                                data,
                                len,
                                100);

    xSemaphoreGive(bus->mutex);
    return ret;
}


HAL_StatusTypeDef I2C_Read(I2C_Bus_t *bus,
                           uint16_t dev_addr,
                           uint8_t *data,
                           uint16_t len)
{
    if (!bus) return HAL_ERROR;

    xSemaphoreTake(bus->mutex, portMAX_DELAY);

    HAL_StatusTypeDef ret =
        HAL_I2C_Master_Receive(bus->hi2c,
                               dev_addr,
                               data,
                               len,
                               100);

    xSemaphoreGive(bus->mutex);
    return ret;
}


HAL_StatusTypeDef I2C_Mem_Write(I2C_Bus_t *bus,
                                uint16_t dev_addr,
                                uint16_t reg,
                                uint16_t reg_size,
                                uint8_t *data,
                                uint16_t len)
{
    if (!bus) return HAL_ERROR;

    xSemaphoreTake(bus->mutex, portMAX_DELAY);

    HAL_StatusTypeDef ret =
        HAL_I2C_Mem_Write(bus->hi2c,
                          dev_addr,
                          reg,
                          reg_size,
                          data,
                          len,
                          100);

    xSemaphoreGive(bus->mutex);
    return ret;
}


HAL_StatusTypeDef I2C_Mem_Read(I2C_Bus_t *bus,
                               uint16_t dev_addr,
                               uint16_t reg,
                               uint16_t reg_size,
                               uint8_t *data,
                               uint16_t len)
{
    if (!bus) return HAL_ERROR;

    xSemaphoreTake(bus->mutex, portMAX_DELAY);

    HAL_StatusTypeDef ret =
        HAL_I2C_Mem_Read(bus->hi2c,
                         dev_addr,
                         reg,
                         reg_size,
                         data,
                         len,
                         100);

    xSemaphoreGive(bus->mutex);
    return ret;
}