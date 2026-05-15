#ifndef __FILE_TRANSFER_H__
#define __FILE_TRANSFER_H__

#include "main.h"
#include "ymodem.h"
#include <stdint.h>
#include <stdbool.h>
#include "lfs_port.h"
/* Version */
#define TRANSFER_VERSION "1.0.0"

// esp32 相关协议定义
#define FRAME_HEAD      0xAA
#define FRAME_TAIL      0x55

// #define CMD_GET_TIME        0x01
// #define CMD_GET_NOW_DETAIL  0x02
// #define CMD_GET_AIR_DETAIL  0x03
// #define CMD_GET_FUTURE_7DAY 0x04
// #define CMD_RESTART         0x05
// #define CMD_OTA_START       0x11
// #define CMD_FS_LIST         0x20
// #define CMD_FS_SELECT       0x22



extern SPI_HandleTypeDef hspi2; // 假设你使用的是 hspi2，根据实际修改
extern spi_flash_t flash_32mb; // Flash 设备句柄


/* 指令类型枚举 */
typedef enum {
    CMD_GET_TIME        =   0x01,
    CMD_GET_NOW_DETAIL  =   0x02,
    CMD_GET_AIR_DETAIL  =   0x03,
    CMD_GET_FUTURE_7DAY =   0x04,
    CMD_RESTART         =   0x05,
    CMD_OTA_START       =   0x11,
    CMD_FS_LIST         =   0x20,
    CMD_FS_SELECT       =   0x22
} comm_cmd_t;

/* 任务间通信的消息结构 */
typedef struct {
    comm_cmd_t type;
    char filename[64]; // 仅在下载文件时使用
} comm_msg_t;



typedef struct {
    uint32_t flash_addr;
    uint32_t total_written;
    uint32_t last_percent;
} transfer_context_t;


/* Flash config */


/* UART config */
#define TRANSFER_UART_TIMEOUT 1000



/* 延时 */
// void transfer_delay_ms(uint32_t ms);

/* transfer 核心接口 */
void transfer_init(void);
// void transfer_run(void);


/* Y-modem 接收 */
int transfer_receive_file(void);

/* 外部 UART */
extern UART_HandleTypeDef huart3;

#endif /* __transfer_H__ */