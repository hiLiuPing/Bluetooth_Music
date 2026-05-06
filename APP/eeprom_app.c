#include "eeprom_app.h"
#include "log.h"

#include <string.h> // 必须包含，用于 memcpy
// 定义句柄
EE24_HandleTypeDef ee_chip;

// 内部工具：计算真实物理地址
static uint32_t Get_Real_Addr(uint8_t use_zone_b, uint32_t offset) {
    return (use_zone_b ? EE_ADDR_ZONE_B : EE_ADDR_ZONE_A) + offset;
}

/**
 * @brief 通用加载配置函数
 * @param use_zone_b: 0 为 A 区，1 为 B 区
 * @param offset: 结构体在区内的偏移量 (如 OFF_NET_CONFIG)
 * @param data: 指向结构体变量的指针
 * @param size: 结构体的大小 (sizeof)
 */
bool AppConfig_Load(uint8_t use_zone_b, uint32_t offset, void *data, uint16_t size) {
    uint32_t addr = Get_Real_Addr(use_zone_b, offset);
    uint8_t *pBuf = (uint8_t*)data;

    // 1. 读取数据
    if(!EE24_Read(&ee_chip, addr, pBuf, size, 1000)) return false;

    // 2. 通用验证：魔数 (假设所有结构体前4字节都是 magic)
    uint32_t read_magic;
    memcpy(&read_magic, pBuf, 4);
    if(read_magic != 0x55AA55AA) return false;

    // 3. 通用验证：CRC (假设所有结构体最后2字节都是 crc)
    uint16_t read_crc;
    memcpy(&read_crc, pBuf + size - 2, 2);
    
    uint16_t cal_crc = AppConfig_CRC16(pBuf, size - 2);
    return (cal_crc == read_crc);
}

/**
 * @brief 通用保存配置函数
 */
bool AppConfig_Save(uint8_t use_zone_b, uint32_t offset, void *data, uint16_t size) {
    uint32_t addr = Get_Real_Addr(use_zone_b, offset);
    uint8_t *pBuf = (uint8_t*)data;

    // 1. 强制设置魔数 (前4字节)
    uint32_t magic = 0x55AA55AA;
    memcpy(pBuf, &magic, 4);

    // 2. 计算并填充 CRC (最后2字节)
    uint16_t cal_crc = AppConfig_CRC16(pBuf, size - 2);
    memcpy(pBuf + size - 2, &cal_crc, 2);

    // 3. 写入 EEPROM
    return EE24_Write(&ee_chip, addr, pBuf, size, 1000);
}
/**
 * @brief  计算 CRC16 (MODBUS 标准)
 * @param  ptr: 数据指针
 * @param  len: 长度
 * @return uint16_t: 校验结果
 */
uint16_t AppConfig_CRC16(uint8_t *ptr, uint16_t len) {
    uint16_t crc = 0xFFFF;
    while(len--) {
        crc ^= *ptr++;
        for(int i = 0; i < 8; i++) {
            if(crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
void AppConfig_RestoreDefault(uint8_t zone) {
    NetConfig_t default_net;
    CalibData_t default_calib;

    // 1. 准备网络默认参数
    memset(&default_net, 0, sizeof(NetConfig_t));
    default_net.magic = 0x55AA55AA;  // 关键：填入魔数
    strncpy(default_net.ssid, "Redmi_1403", 31);
    strncpy(default_net.pwd, "15018762563", 63);
    strncpy(default_net.weather_url, "https://api.weather.com", 127);
    
    // 2. 准备校准默认参数
    memset(&default_calib, 0, sizeof(CalibData_t));
    default_calib.magic = 0x55AA55AA; // 关键：填入魔数
    default_calib.sensor_offset = 0.0f;
    default_calib.calibration_val = 100;

    // 3. 写入到指定分区
    // 注意：这里直接调用底层 AppConfig_Save，它会自动重新算一次 CRC 并写入
    AppConfig_Save(zone, OFF_NET_CONFIG, &default_net, sizeof(NetConfig_t));
    AppConfig_Save(zone, OFF_CALIB_DATA, &default_calib, sizeof(CalibData_t));
    
    log_printf("EEPROM: Data Saved Successfully", zone);
}
/**
 * @brief  系统配置初始化自检
 * @return 0: A区正常, 1: A区损坏但B区修复成功, -1: 全区损坏已初始化
 */
int8_t AppConfig_System_Check(void) {
    NetConfig_t temp_net;
    bool zoneA_ok = false;
    bool zoneB_ok = false;

    log_printf("EEPROM: Running System Self-Check...");

    // 1. 检查 A 区
    zoneA_ok = AppConfig_Load(0, OFF_NET_CONFIG, &temp_net, sizeof(NetConfig_t));
    
    // 2. 检查 B 区
    // 注意：即使 A 区好了，有时也要检查 B 确保备份实时可用
    zoneB_ok = AppConfig_Load(1, OFF_NET_CONFIG, &temp_net, sizeof(NetConfig_t));

    if (zoneA_ok) {
        log_printf("EEPROM: Zone A Data Verified (PASS)");
        if (!zoneB_ok) {
            log_printf("EEPROM: Zone B Corrupted, Syncing from A...");
            AppConfig_Load(0, OFF_NET_CONFIG, &temp_net, sizeof(NetConfig_t));
            AppConfig_Save(1, OFF_NET_CONFIG, &temp_net, sizeof(NetConfig_t));
        }
        return 0;
    } 
    else if (zoneB_ok) {
        log_printf("EEPROM: Zone A Corrupted, Recovering from B...");
        AppConfig_Load(1, OFF_NET_CONFIG, &temp_net, sizeof(NetConfig_t));
        AppConfig_Save(0, OFF_NET_CONFIG, &temp_net, sizeof(NetConfig_t));
        return 1;
    } 
    else {
        log_printf("EEPROM: All Zones Failed, Restoring Factory Defaults...");
        AppConfig_RestoreDefault(0); // 初始化 A
        AppConfig_RestoreDefault(1); // 初始化 B
        return -1;
    }
}
