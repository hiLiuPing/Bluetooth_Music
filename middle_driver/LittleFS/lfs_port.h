#ifndef __LFS_PORT_H__
#define __LFS_PORT_H__

#include "lfs.h"
#include "spi_flash.h"


#define LFS_NO_MALLOC

/* ================= LittleFS 配置 ================= */
#define LFS_READ_SIZE        16
#define LFS_PROG_SIZE        256
#define LFS_BLOCK_SIZE       4096
#define LFS_CACHE_SIZE       256
#define LFS_LOOKAHEAD_SIZE   16
#define LFS_BLOCK_COUNT      1024

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