#include "file_transfer.h"
#include <string.h>
#include "log.h"
#include "spi_flash.h"
#include "spi.h" // 确保能用到 hspi1 等句柄
/* 定义 Flash 设备句柄 */
static spi_flash_t flash_dev; 
/* ================= 初始化 ================= */
void transfer_init(void)
{
    log_printf("\r\n================================\r\n");
    log_printf("     STM32 Y-MODEM Receiver     \r\n");
    log_printf("         Version %s", TRANSFER_VERSION);
    log_printf("\r\n================================\r\n");

    /* 3. 初始化 Flash 硬件接口 */
    // 请根据你的实际硬件连接修改 GPIOA 和 GPIO_PIN_4
    int status = spi_flash_init(&flash_dev, &hspi1, GPIOA, GPIO_PIN_4);
    
    if (status == 0) {
        log_printf("Flash Init Success! Size: %lu MB", flash_dev.flash_size / 1024 / 1024);
    } else {
        log_printf("Flash Init Failed! Error: %d", status);
    }
}

ymodem_file_info_t g_file_info; // 现在 ymodem.h 已定义此类型


static transfer_context_t ctx;

/* 数据包回调：在此处编写真正的 Flash 写入逻辑 */
static bool packet_callback(const uint8_t *data, uint16_t size, uint32_t packet_num, void *user_data) {
    transfer_context_t *c = (transfer_context_t *)user_data;
    if (size == 0) return true;

    /* 写入 Flash (取消注释以生效) */
    // W25Q_WriteBuffer(c->flash_addr, (uint8_t *)data, size);
    
    c->flash_addr += size;
    c->total_written += size;

    uint32_t percent = (c->total_written * 100) / g_file_info.file_size;
    if (percent != c->last_percent) {
        c->last_percent = percent;
        log_printf("Progress: %lu%% (%lu/%lu)", percent, c->total_written, g_file_info.file_size);
    }
    return true;
}

int transfer_receive_file(void) {
    memset(&g_file_info, 0, sizeof(g_file_info));
    memset(&ctx, 0, sizeof(ctx));

    log_printf("\r\nWaiting for file via Y-Modem...\r\n");

    if (!ymodem_wait_receive_header(&g_file_info, 5000)) {
        log_printf("Timeout waiting for file header");
        return -1;
    }

    log_printf("File name : %s", g_file_info.filename);
    log_printf("File size : %lu bytes", g_file_info.file_size);

log_printf("File size: %lu bytes. Erasing Flash...", g_file_info.file_size);
    
    /* * 直接调用你的驱动！
     * 你的驱动内部会自动计算：
     * 1. 头部不满 64KB 的部分用 Sector Erase (4KB)
     * 2. 中间对齐的部分自动用 Block Erase (64KB) 提速
     * 3. 尾部不满 64KB 的部分用 Sector Erase
     */
    if (spi_flash_erase(&flash_dev, 0, g_file_info.file_size) != 0) {
        log_printf("Flash Erase Failed!");
        return -3;
    }

    log_printf("Erase Done. Starting Y-Modem transfer...");

    /* 必须在擦除完后再次同步同步位，确保上位机没有超时断开 */
    ymodem_send_response(YMODEM_C);

    if(ymodem_receive_file_with_callback(&g_file_info, packet_callback, &ctx) != YMODEM_OK) {
        log_printf("Transfer Failed!");
        return -2;
    }

    log_printf("Transfer Success!");
    return 0;
}