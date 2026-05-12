#include "ymodem.h"
#include "spi_flash.h"
#include "ymodem_port_uart.h"

/* ================= Flash实例 ================= */
spi_flash_t g_flash;

/* ================= YMODEM IO ================= */
static ymodem_io_t ymodem_io =
{
    .read  = ymodem_uart_read,
    .write = ymodem_uart_write,
};

/* ================= context ================= */
typedef struct
{
    spi_flash_t *flash;
    uint32_t addr;
} ymodem_ctx_t;

/* ================= Flash callback ================= */
int your_flash_callback(uint8_t *data,
                        uint32_t len,
                        void *user)
{
    ymodem_ctx_t *ctx = (ymodem_ctx_t *)user;

    if (!ctx || !ctx->flash)
        return -1;

    spi_flash_write(ctx->flash,
                    ctx->addr,
                    data,
                    len);

    ctx->addr += len;

    return 0;
}

/* ================= Boot入口 ================= */
void Boot_Ymodem_Start(void)
{
    ymodem_file_info_t file_info;

    ymodem_set_io(&ymodem_io);
    ymodem_receive_init();

    ymodem_ctx_t ctx;

    ctx.flash = &g_flash;
    ctx.addr  = APP_FLASH_START_ADDR;

    if (ymodem_wait_receive_header(&file_info, 10))
    {
        /* ⭐ 关键：先擦除 */
        spi_flash_erase(ctx.flash,
                        APP_FLASH_START_ADDR,
                        file_info.file_size);

        ymodem_receive_file_with_callback(&file_info,
                                          your_flash_callback,
                                          &ctx);
    }
}
int ymodem_uart_read(uint8_t *data,
                     uint16_t len,
                     uint32_t timeout)
{
    uint32_t tick = HAL_GetTick();
    uint16_t rec = 0;

    while (rec < len)
    {
        if (HAL_UART_Receive(&huart1,
                             &data[rec],
                             len - rec,
                             10) == HAL_OK)
        {
            return len;
        }

        if (HAL_GetTick() - tick > timeout)
            break;
    }

    return rec;
}

int ymodem_uart_write(const uint8_t *data,
                      uint16_t len,
                      uint32_t timeout)
{
    return (HAL_UART_Transmit(&huart1,
                              (uint8_t *)data,
                              len,
                              timeout) == HAL_OK)
           ? len : 0;
}