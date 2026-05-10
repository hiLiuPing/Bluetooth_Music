#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__

#include "main.h"
#include <stdint.h>

/* ================= Flash指令 ================= */

#define FLASH_CMD_READ_ID        0x9F
#define FLASH_CMD_READ_DATA      0x03
#define FLASH_CMD_FAST_READ      0x0B
#define FLASH_CMD_PAGE_PROGRAM   0x02
#define FLASH_CMD_SECTOR_ERASE   0x20
#define FLASH_CMD_BLOCK_ERASE    0xD8
#define FLASH_CMD_CHIP_ERASE     0xC7
#define FLASH_CMD_WRITE_ENABLE   0x06
#define FLASH_CMD_READ_STATUS    0x05

#define FLASH_CMD_ENTER_4B       0xB7
#define FLASH_CMD_EXIT_4B        0xE9

/* ================= 状态位 ================= */

#define FLASH_SR_BUSY            0x01

/* ================= 结构体 ================= */

typedef struct
{
    SPI_HandleTypeDef *hspi;

    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;

    uint32_t flash_size;
    uint32_t sector_size;
    uint32_t page_size;

    uint8_t addr_len;   // ⭐ 3 or 4 byte

} spi_flash_t;

/* ================= API ================= */

int spi_flash_init(spi_flash_t *flash,
                   SPI_HandleTypeDef *hspi,
                   GPIO_TypeDef *cs_port,
                   uint16_t cs_pin);

int spi_flash_read(spi_flash_t *flash,
                   uint32_t addr,
                   uint8_t *buf,
                   uint32_t len);

int spi_flash_write(spi_flash_t *flash,
                    uint32_t addr,
                    const uint8_t *buf,
                    uint32_t len);

int spi_flash_erase(spi_flash_t *flash,
                    uint32_t addr,
                    uint32_t len);

int spi_flash_chip_erase(spi_flash_t *flash);

int spi_flash_sync(spi_flash_t *flash);

/* 调试 */
uint32_t spi_flash_read_id(spi_flash_t *flash);

#endif