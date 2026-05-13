#include "ymodem_port.h"
#include "uart_dma.h" // 确保此头文件包含你的 UART DMA 驱动

int ymodem_port_read(uint8_t *data, uint32_t len, uint32_t timeout)
{
    // 调用你之前实现的 UART DMA 读取接口
    return uart_dma_read(data, len, timeout);
}

int ymodem_port_write(const uint8_t *data, uint32_t len, uint32_t timeout)
{
    // 调用你之前实现的 UART DMA 写入接口
    return uart_dma_write(data, len, timeout);
}