#ifndef __WEATHER_APP_H__
#define __WEATHER_APP_H__

#include "main.h"
#include "uart_dma.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// esp32 相关协议定义
#define FRAME_HEAD      0xAA
#define FRAME_TAIL      0x55
/* UART config */



/* 指令类型枚举 */
typedef enum {
    CMD_GET_TIME        =   0x01,
    CMD_GET_NOW_DETAIL  =   0x02,
    CMD_GET_AIR_DETAIL  =   0x03,
    CMD_GET_FUTURE_7DAY =   0x04,
    CMD_RESTART         =   0x05,
    CMD_OTA_START       =   0x11,
    CMD_FS_LIST         =   0x20,
    CMD_FS_SELECT       =   0x22
} comm_cmd_t;

/* 任务间通信的消息结构 */
typedef struct {
    comm_cmd_t type;
    char filename[64]; // 仅在下载文件时使用
} comm_msg_t;



typedef struct
{
    char date[16];    // 日期 "2026-05-16"
    char weather[16]; // 天气 "小雨"
    int temp_high;    // 高温
    int temp_low;     // 低温
    int icon_id;      // 图标ID
} WeatherDay_t;

// 全局 7 天天气表


/* 定义状态机状态 */
typedef enum
{
    STATE_IDLE,
    STATE_CMD,
    STATE_LEN_H,
    STATE_LEN_L,
    STATE_DATA,
    STATE_CRC,
    STATE_TAIL
} ProtocolState_t;


typedef struct {
    char text[32];      // 天气现象 (阴/雷阵雨)
    int  icon;          // 图标ID
    int  temp;          // 温度
    int  feelsLike;     // 体感温度 (只存数字)
    char windDir[32];   // 风向
    int  windScale;     // 风力等级
    int  vis;           // 能见度 (只存数字)
    int  humidity;      // 湿度
    int  aqi;
    int  pm10;
    int  pm25;
    int  no2;
    int  so2;
    float co;
    int  o3;
} WeatherData_t;
// 真正分配内存空间



typedef struct
{
    int aqi;  // 空气质量指数
    int pm10; // PM10 浓度
    int pm25; // PM2.5 浓度
    int no2;  // 二氧化氮
    int so2;  // 二氧化硫
    float co; // 一氧化碳 (通常带小数)
    int o3;   // 臭氧
} AirQuality_t;


// 辅助工具：跳过字符串开头的非数字字符，提取数字

extern WeatherData_t g_now_weather;
 extern AirQuality_t g_air_detail; // 全局空气质量详情
extern WeatherDay_t g_future_weather[7];
extern uart_dma_t uart1_admin; // 唯一实体的定义


void process_protocol_data(uint8_t cmd, char *data);

#endif /* __WEATHER_APP_H__ */
