#ifndef __YMODEM_H__
#define __YMODEM_H__

#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include "crc_common.h"
/* YMODEM Protocol Constants */
#define YMODEM_SOH                 0x01
#define YMODEM_STX                 0x02
#define YMODEM_EOT                 0x04
#define YMODEM_ACK                 0x06
#define YMODEM_NAK                 0x15
#define YMODEM_CAN                 0x18
#define YMODEM_CTRLZ               0x1A
#define YMODEM_C                   0x43

#define YMODEM_PACKET_SIZE_128     128
#define YMODEM_PACKET_SIZE_1024    1024

#define YMODEM_MAX_ERRORS          10
#define YMODEM_TIMEOUT_MS          1000
#define YMODEM_LONG_TIMEOUT_MS     10000

/* IO interface */
typedef struct
{
    int (*read)(uint8_t *data,
                uint16_t len,
                uint32_t timeout);

    int (*write)(const uint8_t *data,
                 uint16_t len,
                 uint32_t timeout);

} ymodem_io_t;

/* YMODEM packet */
typedef struct
{
    uint8_t header;
    uint8_t packet_num;
    uint8_t packet_num_inv;

    uint8_t data[YMODEM_PACKET_SIZE_1024];

    uint16_t crc;

} ymodem_packet_t;

/* Transfer states */
typedef enum
{
    YMODEM_STATE_IDLE = 0,
    YMODEM_STATE_RECEIVING_HEADER,
    YMODEM_STATE_RECEIVING_DATA,
    YMODEM_STATE_COMPLETE,
    YMODEM_STATE_ERROR,
    YMODEM_STATE_CANCELLED

} ymodem_state_t;

/* Result codes */
typedef enum
{
    YMODEM_OK = 0,
    YMODEM_ERROR,
    YMODEM_TIMEOUT,
    YMODEM_CANCELLED,
    YMODEM_CRC_ERROR,
    YMODEM_PACKET_ERROR,
    YMODEM_FILE_ERROR,
    YMODEM_FLASH_ERROR

} ymodem_result_t;

/* File info */
typedef struct
{
    char filename[128];

    uint32_t file_size;
    uint32_t received_size;
    uint32_t packet_count;

    uint8_t error_count;

    ymodem_state_t state;

} ymodem_file_info_t;

/* Callback */
typedef bool (*ymodem_packet_callback_t)(const uint8_t *data,
                                         uint16_t data_size,
                                         uint32_t packet_num,
                                         void *user_data);

/* API */
void ymodem_set_io(ymodem_io_t *io);

ymodem_result_t ymodem_receive_init(void);

ymodem_result_t ymodem_receive_packet(ymodem_packet_t *packet);

ymodem_result_t ymodem_receive_file_with_callback(
    ymodem_file_info_t *file_info,
    ymodem_packet_callback_t callback,
    void *user_data);

bool ymodem_wait_receive_header(ymodem_file_info_t *file_info,
                                int retry_times);

ymodem_result_t ymodem_send_response(uint8_t response);

void ymodem_reset_state(ymodem_file_info_t *file_info);

bool ymodem_verify_crc(const ymodem_packet_t *packet,
                       uint16_t data_size);

bool ymodem_is_packet_valid(const ymodem_packet_t *packet,
                            uint8_t expected_packet_num);

ymodem_result_t ymodem_parse_header_packet(
    const ymodem_packet_t *packet,
    ymodem_file_info_t *file_info);



#endif
