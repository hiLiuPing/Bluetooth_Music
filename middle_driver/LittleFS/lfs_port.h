#ifndef __LFS_PORT_H__
#define __LFS_PORT_H__

#include "lfs.h"
#include "spi_flash.h"


#define LFS_NO_MALLOC


/* ================= LittleFS 优化配置 ================= */
#define LFS_READ_SIZE        16      // 最小读取粒度，16字节合适
#define LFS_PROG_SIZE        256     // 必须等于 Flash Page Size (256)
#define LFS_BLOCK_SIZE       4096    // 必须等于 Flash Sector Size (4KB)
#define LFS_CACHE_SIZE       256     // 缓存大小，建议与 Page Size 一致以优化写性能
#define LFS_LOOKAHEAD_SIZE   32      // W25Q256 块较多，建议增加到 32 或 64
#define LFS_BLOCK_COUNT      8192    // 满额支持 32MB

/* ================= 全局 ================= */
extern lfs_t g_lfs;
extern struct lfs_config g_cfg;

/* ================= API ================= */
int lfs_port_init(spi_flash_t *flash);
lfs_t* lfs_port_get(void);

/* ================= callbacks ================= */
int lfs_read_cb (const struct lfs_config *c,
                 lfs_block_t block,
                 lfs_off_t off,
                 void *buffer,
                 lfs_size_t size);

int lfs_prog_cb (const struct lfs_config *c,
                 lfs_block_t block,
                 lfs_off_t off,
                 const void *buffer,
                 lfs_size_t size);

int lfs_erase_cb(const struct lfs_config *c,
                 lfs_block_t block);

int lfs_sync_cb (const struct lfs_config *c);

#endif