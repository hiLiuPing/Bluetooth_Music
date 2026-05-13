#ifndef FLASH_MAP_H
#define FLASH_MAP_H

#define FLASH_TOTAL_SIZE     (32 * 1024 * 1024)

/* ================= Boot 区 ================= */
#define BOOT_SIZE            (1 * 1024 * 1024)

#define BOOT_START_ADDR      0x00000000
#define BOOT_END_ADDR        (BOOT_SIZE - 1)

/* ================= FS 区 ================= */
#define FS_START_ADDR        (BOOT_SIZE)
#define FS_SIZE              (FLASH_TOTAL_SIZE - BOOT_SIZE)

#endif