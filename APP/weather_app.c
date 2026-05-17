#include "weather_app.h"

#include "log.h"
#include <stdlib.h>   // 提供 atoi 函数
#include <string.h>   // 提供 strtok 函数

AirQuality_t g_air_detail; // 全局空气质量详情
WeatherData_t g_now_weather;
WeatherDay_t g_future_weather[7];
uart_dma_t uart1_admin; // 唯一实体的定义

static int extract_int(char* str) {
    if (str == NULL) return 0;
    char *p = str;
    // 跳过所有不是数字且不是负号的字符
    while (*p && (*p < '0' || *p > '9') && *p != '-') p++; 
    if (*p == '\0') return 0;
    return atoi(p);
}
// 业务分发函数
void process_protocol_data(uint8_t cmd, char *data)
{
    char *ptr;
    switch (cmd)
    {
    case CMD_GET_TIME:
        log_printf("[Time] Received: %s\r\n", data);
        // 这里执行同步系统时间的逻辑
        break;
case CMD_GET_NOW_DETAIL:
        {
            // --- 字段 1: 天气文本 ---
            ptr = strtok(data, ",");
            if (ptr) strncpy(g_now_weather.text, ptr, 31);

            // --- 字段 2: 图标ID ---
            ptr = strtok(NULL, ",");
            if (ptr) g_now_weather.icon = atoi(ptr);

            // --- 字段 3: 温度 ---
            ptr = strtok(NULL, ",");
            if (ptr) g_now_weather.temp = atoi(ptr);

            // --- 字段 4: 体感温度 (处理 "体感温度28℃") ---
            ptr = strtok(NULL, ",");
            if (ptr) g_now_weather.feelsLike = extract_int(ptr);

            // --- 字段 5: 风向 ---
            ptr = strtok(NULL, ",");
            if (ptr) strncpy(g_now_weather.windDir, ptr, 31);

            // --- 字段 6: 能见度 (处理 "能见度3 KM") ---
            ptr = strtok(NULL, ",");
            if (ptr) g_now_weather.vis = extract_int(ptr);

            // --- 字段 7: 湿度 ---
            ptr = strtok(NULL, ",");
            if (ptr) g_now_weather.humidity = atoi(ptr);

            // --- 字段 8: AQI ---
            ptr = strtok(NULL, ",");
            if (ptr) g_now_weather.aqi = atoi(ptr);

            // --- 字段 9~14: 剩下的 PM2.5, CO 等纯数字 ---
            ptr = strtok(NULL, ","); if(ptr) g_now_weather.pm10 = atoi(ptr);
            ptr = strtok(NULL, ","); if(ptr) g_now_weather.pm25 = atoi(ptr);
            ptr = strtok(NULL, ","); if(ptr) g_now_weather.no2 = atoi(ptr);
            ptr = strtok(NULL, ","); if(ptr) g_now_weather.so2 = atoi(ptr);
            ptr = strtok(NULL, ","); if(ptr) g_now_weather.co = atof(ptr);
            ptr = strtok(NULL, ","); if(ptr) g_now_weather.o3 = atoi(ptr);

            log_printf("[Weather] Now: %s, Temp: %d, Feels: %d, Vis: %d\r\n", 
                        g_now_weather.text, g_now_weather.temp, 
                        g_now_weather.feelsLike, g_now_weather.vis);
            break;
        }

case CMD_GET_FUTURE_7DAY:
{
    if (strcmp(data, "LIST_START") == 0) {
        log_printf("[Future] Sync Start...\r\n");
    } 
    else if (strcmp(data, "LIST_END") == 0) {
        log_printf("[Future] Sync Complete!\r\n");
    } 
    else {
        // 使用 strtok 手动切分，不受编码干扰
        char *ptr;
        int idx = -1;

        // 1. 提取 Index
        ptr = strtok(data, "|");
        if (ptr) idx = atoi(ptr);

        if (idx >= 0 && idx < 7) {
            // 2. 提取 Date
            ptr = strtok(NULL, "|");
            if (ptr) strncpy(g_future_weather[idx].date, ptr, 15);

            // 3. 提取 Weather (中文 UTF-8 就在这里)
            ptr = strtok(NULL, "|");
            if (ptr) strncpy(g_future_weather[idx].weather, ptr, 31);

            // 4. 提取 High Temp
            ptr = strtok(NULL, "|");
            if (ptr) g_future_weather[idx].temp_high = atoi(ptr);

            // 5. 提取 Low Temp
            ptr = strtok(NULL, "|");
            if (ptr) g_future_weather[idx].temp_low = atoi(ptr);

            // 6. 提取 Icon ID
            ptr = strtok(NULL, "|");
            if (ptr) g_future_weather[idx].icon_id = atoi(ptr);

            log_printf("[Future] Day %d: %s, %s, %d/%d, Icon:%d\r\n", 
                        idx, g_future_weather[idx].date, g_future_weather[idx].weather,
                        g_future_weather[idx].temp_high, g_future_weather[idx].temp_low, 
                        g_future_weather[idx].icon_id);
        }
    }
    break;
}
    case CMD_GET_AIR_DETAIL: {
        // 按照 ESP32 拼接顺序：AQI,PM10,PM2.5,NO2,SO2,CO,O3
        int count = sscanf(data, "%d,%d,%d,%d,%d,%f,%d",
                           &g_air_detail.aqi,
                           &g_air_detail.pm10,
                           &g_air_detail.pm25,
                           &g_air_detail.no2,
                           &g_air_detail.so2,
                           &g_air_detail.co,
                           &g_air_detail.o3);

        if (count == 7)
        {
            log_printf("[Air] Parse Success! AQI: %d, PM2.5: %d\r\n",
                       g_air_detail.aqi, g_air_detail.pm25);
            // 可以在这里通过事件组或消息队列通知 UI 刷新“空气详情页”
        }
        else
        {
            log_printf("[Air] Parse Failed! Got %d fields\r\n", count);
        }
        break;
    }
        // ... 其他命令处理
    }
}
