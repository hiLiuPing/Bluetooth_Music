#ifndef __EEPROM_APP_H__
#define __EEPROM_APP_H__

#include "main.h"
#include "eeprom.h"

extern EE24_HandleTypeDef ee_chip;

// 分区定义 (BL24C256A: 32KB)
#define EE_ZONE_SIZE        (16384)       // 16KB
#define EE_ADDR_ZONE_A      (0x0000)
#define EE_ADDR_ZONE_B      (EE_ADDR_ZONE_A + EE_ZONE_SIZE)

// 块大小定义 (每个结构体预留空间)
#define EE_BLOCK_SIZE       (256)

// 结构体偏移 (相对于 Zone 起始地址)
#define OFF_NET_CONFIG      (0)
#define OFF_CALIB_DATA      (EE_BLOCK_SIZE * 1)

// ---------------------------------------------------------
// 结构体定义 (统一格式：头 4 字节 Magic，尾 2 字节 CRC)
// ---------------------------------------------------------
typedef struct __attribute__((packed)) {
    uint32_t magic;          // 必须放在开头
    char     ssid[32];
    char     pwd[64];
    char     weather_url[128];
    // char reserved[300];
    uint16_t crc;            // 必须放在末尾
} NetConfig_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    float    sensor_offset;
    int      calibration_val;
    uint16_t crc;
} CalibData_t;

// 静态断言：Keil 5 编译时检查大小,每增加一个结构体，增加一个断言，不然会报错
#define STATIC_ASSERT(COND, MSG) typedef char static_assertion_##MSG[(COND)?1:-1]
STATIC_ASSERT(sizeof(NetConfig_t) <= EE_BLOCK_SIZE, NetConfig_Too_Large);
STATIC_ASSERT(sizeof(CalibData_t) <= EE_BLOCK_SIZE, CalibData_Too_Large);

// ---------------------------------------------------------
// 通用函数声明
// ---------------------------------------------------------
uint16_t AppConfig_CRC16(uint8_t *ptr, uint16_t len);
bool AppConfig_Save(uint8_t use_zone_b, uint32_t offset, void *data, uint16_t size);
bool AppConfig_Load(uint8_t use_zone_b, uint32_t offset, void *data, uint16_t size);
int8_t AppConfig_System_Check(void);
#endif /* __EEPROM_APP_H__ */