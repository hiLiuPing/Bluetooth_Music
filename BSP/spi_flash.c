#include "spi_flash.h"
#include <string.h>
#include "log.h"

#define FLASH_TIMEOUT 0xFFFFFF

/* ================= CS控制 ================= */

static inline void cs_low(spi_flash_t *f)
{
    HAL_GPIO_WritePin(f->cs_port, f->cs_pin, GPIO_PIN_RESET);
}

static inline void cs_high(spi_flash_t *f)
{
    HAL_GPIO_WritePin(f->cs_port, f->cs_pin, GPIO_PIN_SET);
}

/* ================= 基础函数 ================= */

static uint8_t read_status(spi_flash_t *f)
{
    uint8_t cmd = FLASH_CMD_READ_STATUS;
    uint8_t status = 0;

    cs_low(f);
    HAL_SPI_Transmit(f->hspi, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(f->hspi, &status, 1, HAL_MAX_DELAY);
    cs_high(f);

    return status;
}

static int wait_busy(spi_flash_t *f)
{
    uint32_t timeout = FLASH_TIMEOUT;

    while (read_status(f) & FLASH_SR_BUSY)
    {
        if (--timeout == 0)
        {
            log_printf("Flash Wait Busy Timeout!\r\n");
            return -1;
        }
    }
    return 0;
}

static void write_enable(spi_flash_t *f)
{
    uint8_t cmd = FLASH_CMD_WRITE_ENABLE;

    cs_low(f);
    HAL_SPI_Transmit(f->hspi, &cmd, 1, HAL_MAX_DELAY);
    cs_high(f);
}

/* ================= ⭐ 地址发送 ================= */

static void send_address(spi_flash_t *f, uint32_t addr)
{
    uint8_t buf[4];

    if (f->addr_len == 3)
    {
        buf[0] = addr >> 16;
        buf[1] = addr >> 8;
        buf[2] = addr;

        HAL_SPI_Transmit(f->hspi, buf, 3, HAL_MAX_DELAY);
    }
    else
    {
        buf[0] = addr >> 24;
        buf[1] = addr >> 16;
        buf[2] = addr >> 8;
        buf[3] = addr;

        HAL_SPI_Transmit(f->hspi, buf, 4, HAL_MAX_DELAY);
    }
}

/* ================= 4字节模式 ================= */

static void enter_4byte_mode(spi_flash_t *f)
{
    uint8_t cmd = FLASH_CMD_ENTER_4B;

    cs_low(f);
    HAL_SPI_Transmit(f->hspi, &cmd, 1, HAL_MAX_DELAY);
    cs_high(f);
}

/* ================= 读ID ================= */

uint32_t spi_flash_read_id(spi_flash_t *f)
{
    uint8_t cmd = FLASH_CMD_READ_ID;
    uint8_t id[3] = {0};

    cs_low(f);
    HAL_SPI_Transmit(f->hspi, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(f->hspi, id, 3, HAL_MAX_DELAY);
    cs_high(f);

    return (id[0] << 16) | (id[1] << 8) | id[2];
}

/* ================= 初始化 ================= */

int spi_flash_init(spi_flash_t *f,
                   SPI_HandleTypeDef *hspi,
                   GPIO_TypeDef *cs_port,
                   uint16_t cs_pin)
{
    f->hspi = hspi;
    f->cs_port = cs_port;
    f->cs_pin = cs_pin;

    f->sector_size = 4096;
    f->page_size = 256;
    f->addr_len = 3;

    uint32_t id = spi_flash_read_id(f);

    log_printf("Flash ID: 0x%06X\r\n", id);

    if ((id >> 16) == 0xEF) // Winbond
    {
        switch (id & 0xFF)
        {
        case 0x17: f->flash_size = 8 * 1024 * 1024; break;
        case 0x18: f->flash_size = 16 * 1024 * 1024; break;

        case 0x19:  // ⭐ W25Q256
            f->flash_size = 32 * 1024 * 1024;
            f->addr_len = 4;
            break;

        default:
            return -2;
        }
    }
    else
    {
        return -1;
    }

    /* ⭐ 如果是4字节地址，必须进入模式 */
    if (f->addr_len == 4)
    {
        enter_4byte_mode(f);
    }

    return 0;
}

/* ================= 读 ================= */

int spi_flash_read(spi_flash_t *f,
                   uint32_t addr,
                   uint8_t *buf,
                   uint32_t len)
{
    cs_low(f);

    uint8_t cmd = FLASH_CMD_READ_DATA;
    HAL_SPI_Transmit(f->hspi, &cmd, 1, HAL_MAX_DELAY);

    send_address(f, addr);

    HAL_SPI_Receive(f->hspi, buf, len, HAL_MAX_DELAY);

    cs_high(f);

    return 0;
}

/* ================= 写 ================= */

int spi_flash_write(spi_flash_t *f,
                    uint32_t addr,
                    const uint8_t *buf,
                    uint32_t len)
{
    uint32_t remain = len;

    while (remain > 0)
    {
        uint32_t page_offset = addr % f->page_size;
        uint32_t write_len = f->page_size - page_offset;

        if (write_len > remain)
            write_len = remain;

        write_enable(f);

        cs_low(f);

        uint8_t cmd = FLASH_CMD_PAGE_PROGRAM;
        HAL_SPI_Transmit(f->hspi, &cmd, 1, HAL_MAX_DELAY);

        send_address(f, addr);

        HAL_SPI_Transmit(f->hspi, (uint8_t*)buf, write_len, HAL_MAX_DELAY);

        cs_high(f);

        if (wait_busy(f) != 0)
            return -1;

        addr += write_len;
        buf  += write_len;
        remain -= write_len;
    }

    return 0;
}

/* ================= 擦除 ================= */

int spi_flash_erase(spi_flash_t *f,
                    uint32_t addr,
                    uint32_t len)
{
    if (addr % f->sector_size != 0)
        return -1;

    uint32_t end = addr + len;

    while (addr < end)
    {
        write_enable(f);

        cs_low(f);

        uint8_t cmd = FLASH_CMD_SECTOR_ERASE;
        HAL_SPI_Transmit(f->hspi, &cmd, 1, HAL_MAX_DELAY);

        send_address(f, addr);

        cs_high(f);

        if (wait_busy(f) != 0)
            return -2;

        addr += f->sector_size;
    }

    return 0;
}

/* ================= 整片擦除 ================= */

int spi_flash_chip_erase(spi_flash_t *f)
{
    write_enable(f);

    uint8_t cmd = FLASH_CMD_CHIP_ERASE;

    cs_low(f);
    HAL_SPI_Transmit(f->hspi, &cmd, 1, HAL_MAX_DELAY);
    cs_high(f);

    return wait_busy(f);
}

/* ================= 同步 ================= */

int spi_flash_sync(spi_flash_t *f)
{
    return wait_busy(f);
}