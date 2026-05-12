#include "transmit_app.h"

#include "log.h"


uint8_t dma_buf[DMA_BUF_SIZE];

/* LwRB 环形缓冲 */

uint8_t rb_data[RB_SIZE];
lwrb_t rb;


void UART_DMA_LwRB_Init(void)
{
    lwrb_init(&rb, rb_data, sizeof(rb_data));

    HAL_UART_Receive_DMA(&huart3, dma_buf, DMA_BUF_SIZE);

    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
}


void UART_IdleHandler(void)
{
    static uint16_t old_pos = 0;
    uint16_t pos;

    pos = DMA_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx);

    if(pos != old_pos)
    {
        if(pos > old_pos)
        {
            lwrb_write(&rb, &dma_buf[old_pos], pos - old_pos);
        }
        else
        {
            lwrb_write(&rb, &dma_buf[old_pos], DMA_BUF_SIZE - old_pos);
            lwrb_write(&rb, &dma_buf[0], pos);
        }

        old_pos = pos;
    }
}


void UART_Forward_Task(void)
{
    uint8_t tmp_buf[256];
    uint16_t len;

    while (lwrb_get_full(&rb))
    {
        len = lwrb_read(&rb, tmp_buf, sizeof(tmp_buf));
        if (len > 0)
        {
            HAL_UART_Transmit(&huart3, tmp_buf, len, HAL_MAX_DELAY);
        }
    }
}