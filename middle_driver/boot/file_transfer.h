#ifndef __FILE_TRANSFER_H__
#define __FILE_TRANSFER_H__

#include "main.h"
#include "ymodem.h"
#include <stdint.h>
#include <stdbool.h>

/* Version */
#define TRANSFER_VERSION "1.0.0"

/* Flash config */


/* UART config */
#define TRANSFER_UART_TIMEOUT 1000

// /* LED 控制 */
// void transfer_led_toggle(void);
// void transfer_led_set(bool state);

/* 延时 */
// void transfer_delay_ms(uint32_t ms);

/* transfer 核心接口 */
void transfer_init(void);
// void transfer_run(void);


/* Y-modem 接收 */
int transfer_receive_file(void);

/* CRC32 */
uint32_t transfer_crc32(const uint8_t *data, uint32_t size);


/* 外部 UART */
extern UART_HandleTypeDef huart1;

#endif /* __transfer_H__ */