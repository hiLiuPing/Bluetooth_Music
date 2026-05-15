#ifndef __UART_DMA_H__
#define __UART_DMA_H__

#include "main.h"
#include "lwrb.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_DMA_RX_SIZE      1024
#define UART_LWRB_SIZE        4096

extern UART_HandleTypeDef huart1;

extern  lwrb_t uart_rb;

 extern uint8_t dma_rx_buf[UART_DMA_RX_SIZE];

 extern uint8_t lwrb_buf[UART_LWRB_SIZE];

void uart_dma_init(void);

void uart_dma_rx_check(void);

int uart_dma_read(uint8_t *data,
                  uint32_t len,
                  uint32_t timeout);

int uart_dma_write(const uint8_t *data,
                   uint32_t len,
                   uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif