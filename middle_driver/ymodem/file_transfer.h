#ifndef __FILE_TRANSFER_H__
#define __FILE_TRANSFER_H__

#include "main.h"
#include "ymodem.h"
#include <stdint.h>
#include <stdbool.h>

/* Version */
#define TRANSFER_VERSION "1.0.0"


extern SPI_HandleTypeDef hspi1; // 假设你使用的是 hspi1，根据实际修改

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
extern UART_HandleTypeDef huart1;

#endif /* __transfer_H__ */