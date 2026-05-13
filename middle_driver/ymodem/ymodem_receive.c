#include "ymodem.h"
#include "ymodem_port.h"
#include "ymodem_crc.h"
#include <string.h>
#include <stdlib.h>

/* 解析 Packet 0 (文件名和大小) */
static void parse_header_packet(uint8_t *buf, ymodem_file_info_t *info) {
    strncpy(info->filename, (char *)buf, 128);
    char *p = (char *)buf + strlen(info->filename) + 1;
    info->file_size = (uint32_t)atoi(p);
}

/* 等待文件头 (解决 Warning #223-D) */
// 等待文件头 (Packet 0)
/* 等待文件头 */
int ymodem_wait_receive_header(ymodem_file_info_t *info, uint32_t timeout) {
    uint8_t c, buf[128], seq[2], crc_bytes[2];
    ymodem_send_response(YMODEM_C);

    if (ymodem_port_read(&c, 1, timeout) <= 0 || c != YMODEM_SOH) return 0;
    
    ymodem_port_read(seq, 2, timeout); 
    ymodem_port_read(buf, 128, timeout);
    ymodem_port_read(crc_bytes, 2, timeout);

    // 校验序号 (Packet 0 序号为 0)
    if (seq[0] != 0x00 || (seq[0] + seq[1] != 0xFF)) return 0;

    parse_header_packet(buf, info);
    ymodem_send_response(YMODEM_ACK);
    ymodem_send_response(YMODEM_C); // 准备接收第一个数据包
    return 1;
}

/* 循环接收数据包 (解决 Warning #223-D) */
int ymodem_receive_file_with_callback(ymodem_file_info_t *info, ymodem_packet_cb cb, void *user_data) {
    uint8_t c, buf[1024], seq_bytes[2], crc_bytes[2];
    uint8_t expected_seq = 1;

    while (1) {
        if (ymodem_port_read(&c, 1, 1000) <= 0) return YMODEM_TIMEOUT;

        if (c == YMODEM_EOT) {
            ymodem_send_response(YMODEM_ACK);
            return YMODEM_OK;
        }

        if (c == YMODEM_SOH || c == YMODEM_STX) {
            uint16_t size = (c == YMODEM_STX) ? 1024 : 128;
            ymodem_port_read(seq_bytes, 2, 1000);
            ymodem_port_read(buf, size, 1000);
            ymodem_port_read(crc_bytes, 2, 1000);

            // 1. 校验序号连续性与反码
            if ((seq_bytes[0] + seq_bytes[1] != 0xFF) || (seq_bytes[0] != expected_seq)) {
                ymodem_send_response(YMODEM_NAK);
                continue;
            }

            // 2. CRC16 校验 (使用上传的 ymodem_crc.c 接口)
            uint16_t remote_crc = (crc_bytes[0] << 8) | crc_bytes[1];
            if (crc16_update(0, buf, size) != remote_crc) {
                ymodem_send_response(YMODEM_NAK);
                continue;
            }

            // 3. 执行回调写入 Flash
            if (cb && !cb(buf, size, expected_seq, user_data)) {
                ymodem_send_response(YMODEM_CAN);
                return YMODEM_ERROR;
            }

            expected_seq++;
            ymodem_send_response(YMODEM_ACK);
        }
    }
}