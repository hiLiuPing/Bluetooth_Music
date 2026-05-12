#ifndef __TRANSMIT_APP_H__
#define __TRANSMIT_APP_H__

#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lwrb.h"


extern UART_HandleTypeDef huart3; // 用你自己的UART句柄
#define RB_SIZE 4096

#define DMA_BUF_SIZE 2048
/* 1. 命令类型枚举 */
/* 建议使用这种带明确数值的枚举，与 ESP32 端的协议文档严格一致 */
typedef enum {
    CMD_SYNC_TIME      = 0x01,  // 对时请求 (对应 AA 01 55)
    CMD_UPDATE_WEATHER = 0x02,  // 更新天气 (对应 AA 02 55)
    CMD_GET_FILE_LIST  = 0x20,  // 获取文件列表 (对应 AA 20 55)
    CMD_DOWNLOAD_FILE  = 0x22,  // 下载指定文件 (对应 AA 22 [name] 55)
    CMD_UPGRADE_APP    = 0x11   // 触发 YModem 升级 (对应 AA 11 55)
} cmd_type_t;

/* 2. 命令结构体 */
typedef struct {
    cmd_type_t type;
    char filename[32];    // 用于 CMD_DOWNLOAD_FILE 传递文件名
} comm_msg_t;

/* 3. 声明原生 FreeRTOS 队列句柄 */
extern QueueHandle_t CommQueueHandle;



#endif /* __TRANSMIT_APP_H__ */
