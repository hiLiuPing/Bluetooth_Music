#include "uart_dma.h"

 uint8_t dma_rx_buf[UART_DMA_RX_SIZE];

 uint8_t lwrb_buf[UART_LWRB_SIZE];

 lwrb_t uart_rb;

 volatile uint32_t old_pos = 0;

/* ================= 初始化 ================= */

void uart_dma_init(void)
{
    lwrb_init(&uart_rb,
              lwrb_buf,
              sizeof(lwrb_buf));

    HAL_UARTEx_ReceiveToIdle_DMA(&huart3,
                                 dma_rx_buf,
                                 UART_DMA_RX_SIZE);

    __HAL_DMA_DISABLE_IT(huart3.hdmarx,
                         DMA_IT_HT);
}

/* ================= DMA检查 ================= */

void uart_dma_rx_check(void)
{
    uint32_t pos;

    pos = UART_DMA_RX_SIZE -
          __HAL_DMA_GET_COUNTER(huart3.hdmarx);

    if (pos != old_pos)
    {
        if (pos > old_pos)
        {
            lwrb_write(&uart_rb,
                       &dma_rx_buf[old_pos],
                       pos - old_pos);
        }
        else
        {
            lwrb_write(&uart_rb,
                       &dma_rx_buf[old_pos],
                       UART_DMA_RX_SIZE - old_pos);

            lwrb_write(&uart_rb,
                       &dma_rx_buf[0],
                       pos);
        }

        old_pos = pos;
    }
}

/* ================= 读 ================= */

int uart_dma_read(uint8_t *data,
                  uint32_t len,
                  uint32_t timeout)
{
    uint32_t tick = HAL_GetTick();

    uint32_t rec = 0;

    while (rec < len)
    {
        rec += lwrb_read(&uart_rb,
                         &data[rec],
                         len - rec);

        if ((HAL_GetTick() - tick) > timeout)
        {
            break;
        }
    }

    return rec;
}

/* ================= 写 ================= */

int uart_dma_write(const uint8_t *data,
                   uint32_t len,
                   uint32_t timeout)
{
    if (HAL_UART_Transmit(&huart3,
                          (uint8_t *)data,
                          len,
                          timeout) == HAL_OK)
    {
        return len;
    }

    return 0;
}