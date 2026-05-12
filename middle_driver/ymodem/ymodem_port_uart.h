#ifndef __YMODEM_PORT_UART_H__
#define __YMODEM_PORT_UART_H__

#include "main.h"
#include <stdint.h>

/* ================= OTA / APP 起始地址 ================= */
#define APP_FLASH_START_ADDR   (0x00000000U)

/* ================= Flash 参数（可改） ================= */
#define FLASH_PAGE_SIZE        (256U)
#define FLASH_SECTOR_SIZE      (4096U)
#define FLASH_BLOCK_SIZE       (65536U)



extern UART_HandleTypeDef huart1;

int ymodem_uart_read(uint8_t *data,
                     uint16_t len,
                     uint32_t timeout);

int ymodem_uart_write(const uint8_t *data,
                      uint16_t len,
                      uint32_t timeout);

#endif