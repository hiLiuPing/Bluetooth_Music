#include "file_transfer.h"
#include <string.h>
#include "log.h"
#include "spi_flash.h"
/* 全局文件信息 */
ymodem_file_info_t g_file_info;

/* 接收上下文 */
typedef struct
{
    uint32_t flash_addr;
    uint32_t total_written;
    uint32_t last_percent;
} transfer_context_t;

static transfer_context_t ctx;

/* ================= 初始化 ================= */
void transfer_init(void)
{
    log_printf("\r\n================================");
    log_printf("     STM32 Y-MODEM Receiver     ");
    log_printf("         Version %s", TRANSFER_VERSION);
    log_printf("================================\r\n");
}


/* ================= 数据包回调 ================= */
static bool packet_callback(const uint8_t *data,
                            uint16_t size,
                            uint32_t packet_num,
                            void *user_data)
{
    transfer_context_t *c = (transfer_context_t *)user_data;

    if(size == 0) return true; // 空包跳过

    /* 写入 Flash */
    // W25Q_WriteBuffer(c->flash_addr, (uint8_t *)data, size);
    c->flash_addr += size;
    c->total_written += size;

    /* 计算进度 */
    uint32_t percent = (c->total_written * 100) / g_file_info.file_size;
    if(percent != c->last_percent)
    {
        c->last_percent = percent;
        log_printf("Progress: %lu%%  (%lu/%lu)",
                   percent,
                   c->total_written,
                   g_file_info.file_size);
    }

    return true; // 返回 true 表示成功
}

/* ================= 文件接收 ================= */
int transfer_receive_file(void)
{
    memset(&g_file_info, 0, sizeof(g_file_info));
    memset(&ctx, 0, sizeof(ctx));

    log_printf("\r\nWaiting for file via Y-Modem...\r\n");

    /* 等待文件头 */
    if(!ymodem_wait_receive_header(&g_file_info, 1000))
    {
        log_printf("Timeout waiting for file header");
        return -1;
    }

    log_printf("File name : %s", g_file_info.filename);
    log_printf("File size : %lu bytes", g_file_info.file_size);

    /* 擦除 Flash 扇区（按文件大小计算需要擦除的扇区数） */
    uint32_t sector_count = (g_file_info.file_size + 4095) / 4096;
    for(uint32_t i=0; i<sector_count; i++)
    {
        log_printf("Erasing sector %lu...", i);
        // W25Q_EraseSector(i*4096);
    }

    /* 初始化写入上下文 */
    ctx.flash_addr    = 0;  // 从 Flash 0 地址写入
    ctx.total_written = 0;
    ctx.last_percent  = 0;

    /* 发送 C 请求 Y-Modem 开始传输 */
    ymodem_send_response(YMODEM_C);

    /* 接收文件 */
    if(ymodem_receive_file_with_callback(&g_file_info,
                                         packet_callback,
                                         &ctx) != YMODEM_OK)
    {
        log_printf("File reception failed!");
        return -2;
    }

    log_printf("\r\nFile received successfully!");
    log_printf("Name : %s", g_file_info.filename);
    log_printf("Size : %lu bytes", g_file_info.file_size);

    /* 可选：读取前16字节验证 */
    uint8_t buf[16];
    // W25Q_Read(0, buf, sizeof(buf));
    log_printf("Flash first 16 bytes:");
    for(int i=0; i<16; i++)
        log_printf("%02X ", buf[i]);

    return 0;
}


