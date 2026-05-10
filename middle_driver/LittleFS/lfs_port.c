#include "lfs_port.h"
#include "spi_flash.h"
#include "log.h"
#include <string.h>
#include <stdio.h>



/* ================= 全局 ================= */

lfs_t g_lfs;
struct lfs_config g_cfg;
#define LFS_NO_MALLOC
/* 缓冲区（必须static） */
static uint8_t read_buf[LFS_CACHE_SIZE];
static uint8_t prog_buf[LFS_CACHE_SIZE];

static uint8_t lookahead_buf[LFS_LOOKAHEAD_SIZE] __attribute__((aligned(4)));

/* ================= 回调 ================= */

int lfs_read_cb(const struct lfs_config *c,
                       lfs_block_t block,
                       lfs_off_t off,
                       void *buffer,
                       lfs_size_t size)
{
    spi_flash_t *flash = (spi_flash_t *)c->context;

    // 计算 Flash 实际地址
    uint32_t addr = block * c->block_size + off;

    // // ⭐ 打印调试信息
    // log_printf("[LFS READ] block=%lu, off=%lu, size=%lu, addr=0x%06lX\r\n",
    //            (unsigned long)block,
    //            (unsigned long)off,
    //            (unsigned long)size,
    //            (unsigned long)addr);

    // 调用 SPI Flash 读取
    return spi_flash_read(flash, addr, buffer, size);
}

int lfs_prog_cb(const struct lfs_config *c,
                       lfs_block_t block,
                       lfs_off_t off,
                       const void *buffer,
                       lfs_size_t size)
{
    spi_flash_t *flash = (spi_flash_t *)c->context;

    uint32_t addr = block * c->block_size + off;
    // ⭐ 打印调试信息
    // log_printf("[LFS prog] block=%lu, off=%lu, size=%lu, addr=0x%06lX\r\n",
    //            (unsigned long)block,
    //            (unsigned long)off,
    //            (unsigned long)size,
    //            (unsigned long)addr);
    return spi_flash_write(flash, addr, buffer, size);
}

int lfs_erase_cb(const struct lfs_config *c,
                        lfs_block_t block)
{
    spi_flash_t *flash = (spi_flash_t *)c->context;

    uint32_t addr = block * c->block_size;
    // log_printf("[LFS READ] block=%lu, addr=0x%06lX\r\n",
    //            (unsigned long)block,
    //            (unsigned long)addr);
    return spi_flash_erase(flash, addr, c->block_size);
}

int lfs_sync_cb(const struct lfs_config *c)
{
    spi_flash_t *flash = (spi_flash_t *)c->context;

    return spi_flash_sync(flash);
}

/* ================= 初始化 ================= */

int lfs_port_init(spi_flash_t *flash)
{
    // log_printf("\r\n[LFS] init start\r\n");
   HAL_Delay(100);
    memset(&g_cfg, 0, sizeof(g_cfg));

    /* 绑定flash */
    g_cfg.context = flash;

    /* 回调 */
    g_cfg.read  = lfs_read_cb;
    g_cfg.prog  = lfs_prog_cb;
    g_cfg.erase = lfs_erase_cb;
    g_cfg.sync  = lfs_sync_cb;

//     /* 参数（必须匹配你驱动） */

    g_cfg.read_size = LFS_READ_SIZE;
    g_cfg.prog_size = LFS_PROG_SIZE;
    g_cfg.block_size = LFS_BLOCK_SIZE;
    g_cfg.block_count = LFS_BLOCK_COUNT;
    g_cfg.lookahead_size = LFS_LOOKAHEAD_SIZE;
    g_cfg.block_cycles = 500;
        g_cfg.cache_size = 512;
    /* ========== 挂载 ========== */
//    ret = lfs_format(&g_lfs, &g_cfg);
g_cfg.read_buffer = read_buf;
g_cfg.prog_buffer = prog_buf;
g_cfg.lookahead_buffer = lookahead_buf;


// lfs_format(&g_lfs, &g_cfg);
    int ret = lfs_mount(&g_lfs, &g_cfg);

    if (ret != 0)
    {
        // log_printf("[LFS] mount fail -> format\r\n");

        /* 格式化 */
        ret = lfs_format(&g_lfs, &g_cfg);
        if (ret != 0)
        {
            // log_printf("[LFS] format fail\r\n");
            return -1;
        }

        /* 再挂载 */
        ret = lfs_mount(&g_lfs, &g_cfg);
        if (ret != 0)
        {
            // log_printf("[LFS] mount fail again\r\n");
            return -2;
        }
    }

    // log_printf("[LFS] mount OK\r\n");

    return 0;
}

/* ================= 获取句柄 ================= */

lfs_t* lfs_port_get(void)
{
    return &g_lfs;
}