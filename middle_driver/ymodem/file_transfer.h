#ifndef __FILE_TRANSFER_H__
#define __FILE_TRANSFER_H__

#include "main.h"
#include "ymodem.h"
#include <stdint.h>
#include <stdbool.h>
#include "lfs_port.h"
/* Version */
#define TRANSFER_VERSION "1.0.0"


extern SPI_HandleTypeDef hspi2; // 假设你使用的是 hspi2，根据实际修改
extern spi_flash_t flash_32mb; // Flash 设备句柄


/* 指令类型枚举 */
typedef enum {
    CMD_SYNC_TIME,
    CMD_GET_FILE_LIST,
    CMD_DOWNLOAD_FILE,
    // 可以继续添加...
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