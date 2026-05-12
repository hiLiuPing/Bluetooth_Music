#include "ymodem.h"

#include <stdlib.h>
#include <string.h>

static ymodem_io_t *s_io = NULL;

static ymodem_packet_t s_packet;

static ymodem_result_t ymodem_receive_byte(uint8_t *byte,
                                           uint32_t timeout_ms);

static ymodem_result_t ymodem_send_byte(uint8_t byte);

static void ymodem_flush_input_buffer(void);

void ymodem_set_io(ymodem_io_t *io)
{
    s_io = io;
}

void ymodem_reset_state(ymodem_file_info_t *file_info)
{
    memset(file_info, 0, sizeof(ymodem_file_info_t));

    file_info->state = YMODEM_STATE_IDLE;
}

static ymodem_result_t ymodem_receive_byte(uint8_t *byte,
                                           uint32_t timeout_ms)
{
    if (s_io == NULL || s_io->read == NULL)
    {
        return YMODEM_ERROR;
    }

    if (s_io->read(byte, 1, timeout_ms) == 1)
    {
        return YMODEM_OK;
    }

    return YMODEM_TIMEOUT;
}

static ymodem_result_t ymodem_send_byte(uint8_t byte)
{
    if (s_io == NULL || s_io->write == NULL)
    {
        return YMODEM_ERROR;
    }

    if (s_io->write(&byte, 1, YMODEM_TIMEOUT_MS) == 1)
    {
        return YMODEM_OK;
    }

    return YMODEM_ERROR;
}

static void ymodem_flush_input_buffer(void)
{
    uint8_t dummy;

    while (ymodem_receive_byte(&dummy, 10) == YMODEM_OK)
    {
    }
}

ymodem_result_t ymodem_receive_init(void)
{
    ymodem_flush_input_buffer();

    return ymodem_send_response(YMODEM_C);
}

ymodem_result_t ymodem_send_response(uint8_t response)
{
    return ymodem_send_byte(response);
}

bool ymodem_verify_crc(const ymodem_packet_t *packet,
                       uint16_t data_size)
{
    uint16_t crc = crc16_update(0x0000,
                                packet->data,
                                data_size);

    return (crc == packet->crc);
}

bool ymodem_is_packet_valid(const ymodem_packet_t *packet,
                            uint8_t expected_packet_num)
{
    if (packet->packet_num != expected_packet_num)
    {
        return false;
    }

    if ((packet->packet_num + packet->packet_num_inv) != 0xFF)
    {
        return false;
    }

    return true;
}

ymodem_result_t ymodem_receive_packet(ymodem_packet_t *packet)
{
    ymodem_result_t result;

    uint16_t data_size;

    uint8_t crc_bytes[2];

    result = ymodem_receive_byte(&packet->header,
                                 YMODEM_TIMEOUT_MS);

    if (result != YMODEM_OK)
    {
        return result;
    }

    if (packet->header == YMODEM_EOT)
    {
        return YMODEM_OK;
    }

    if (packet->header == YMODEM_CAN)
    {
        return YMODEM_CANCELLED;
    }

    if (packet->header == YMODEM_SOH)
    {
        data_size = YMODEM_PACKET_SIZE_128;
    }
    else if (packet->header == YMODEM_STX)
    {
        data_size = YMODEM_PACKET_SIZE_1024;
    }
    else
    {
        return YMODEM_PACKET_ERROR;
    }

    result = ymodem_receive_byte(&packet->packet_num,
                                 YMODEM_TIMEOUT_MS);

    if (result != YMODEM_OK)
    {
        return result;
    }

    result = ymodem_receive_byte(&packet->packet_num_inv,
                                 YMODEM_TIMEOUT_MS);

    if (result != YMODEM_OK)
    {
        return result;
    }

    if (s_io->read(packet->data,
                   data_size,
                   YMODEM_TIMEOUT_MS) != data_size)
    {
        return YMODEM_TIMEOUT;
    }

    if (s_io->read(crc_bytes,
                   2,
                   YMODEM_TIMEOUT_MS) != 2)
    {
        return YMODEM_TIMEOUT;
    }

    packet->crc = (crc_bytes[0] << 8) | crc_bytes[1];

    if (!ymodem_verify_crc(packet, data_size))
    {
        return YMODEM_CRC_ERROR;
    }

    return YMODEM_OK;
}

ymodem_result_t ymodem_parse_header_packet(
    const ymodem_packet_t *packet,
    ymodem_file_info_t *file_info)
{
    char *ptr;

    ptr = (char *)packet->data;

    ymodem_reset_state(file_info);

    if (packet->data[0] == 0)
    {
        file_info->state = YMODEM_STATE_COMPLETE;

        return YMODEM_OK;
    }

    strncpy(file_info->filename,
            ptr,
            sizeof(file_info->filename) - 1);

    ptr += strlen(ptr) + 1;

    file_info->file_size = (uint32_t)atol(ptr);

    file_info->packet_count = 1;

    file_info->state = YMODEM_STATE_RECEIVING_DATA;

    return YMODEM_OK;
}

bool ymodem_wait_receive_header(ymodem_file_info_t *file_info,
                                int retry_times)
{
    ymodem_result_t result;

    int i;

    i = 0;

    while (i++ < retry_times)
    {
        result = ymodem_receive_packet(&s_packet);

        if (result == YMODEM_OK)
        {
            result = ymodem_parse_header_packet(&s_packet,
                                                file_info);

            if (result != YMODEM_OK)
            {
                ymodem_send_response(YMODEM_NAK);

                return false;
            }

            ymodem_send_response(YMODEM_ACK);

            return true;
        }

        ymodem_send_response(YMODEM_C);
    }

    return false;
}

ymodem_result_t ymodem_receive_file_with_callback(
    ymodem_file_info_t *file_info,
    ymodem_packet_callback_t callback,
    void *user_data)
{
    ymodem_result_t result;

    uint8_t expected_packet_num = 1;

    while (1)
    {
        result = ymodem_receive_packet(&s_packet);

        if (result == YMODEM_TIMEOUT)
        {
            file_info->error_count++;

            if (file_info->error_count >= YMODEM_MAX_ERRORS)
            {
                ymodem_send_response(YMODEM_CAN);

                file_info->state = YMODEM_STATE_ERROR;

                return YMODEM_TIMEOUT;
            }

            ymodem_send_response(YMODEM_NAK);

            continue;
        }

        if (result == YMODEM_CANCELLED)
        {
            file_info->state = YMODEM_STATE_CANCELLED;

            return YMODEM_CANCELLED;
        }

        if (result != YMODEM_OK)
        {
            ymodem_send_response(YMODEM_NAK);

            continue;
        }

        if (s_packet.header == YMODEM_EOT)
        {
            ymodem_send_response(YMODEM_ACK);

            file_info->state = YMODEM_STATE_COMPLETE;

            break;
        }

        if (!ymodem_is_packet_valid(&s_packet,
                                    expected_packet_num))
        {
            ymodem_send_response(YMODEM_NAK);

            continue;
        }

        uint16_t packet_size;

        packet_size = (s_packet.header == YMODEM_SOH)
                          ? YMODEM_PACKET_SIZE_128
                          : YMODEM_PACKET_SIZE_1024;

        uint32_t remain;

        remain = file_info->file_size - file_info->received_size;

        uint16_t actual_size;

        actual_size = (remain < packet_size)
                          ? remain
                          : packet_size;

        if (callback)
        {
            if (!callback(s_packet.data,
                          actual_size,
                          expected_packet_num,
                          user_data))
            {
                ymodem_send_response(YMODEM_CAN);

                file_info->state = YMODEM_STATE_ERROR;

                return YMODEM_FLASH_ERROR;
            }
        }

        ymodem_send_response(YMODEM_ACK);

        file_info->received_size += actual_size;

        file_info->packet_count++;

        file_info->error_count = 0;

        expected_packet_num++;
    }

    return YMODEM_OK;
}

