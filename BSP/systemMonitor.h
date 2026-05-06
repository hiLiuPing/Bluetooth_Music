/*============================================================
 * File: SystemMonitor.h
 * 通用系统监控模块（底层）
 *===========================================================*/
#ifndef __SYSTEMMONITOR_H
#define __SYSTEMMONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stdint.h"
#include "FreeRTOS.h"
#include "timers.h"


/*============================================================
 * 宏定义
 *===========================================================*/
#define MONITOR_MAX_NUM        8
#define MONITOR_INVALID_ID     0xFF


/*============================================================
 * 类型定义
 *===========================================================*/

/* 监控ID类型（底层不关心业务含义） */
typedef uint8_t MonitorID_t;

/* 超时回调函数 */
typedef void (*MonitorCallback_t)(TimerHandle_t xTimer);

/* 单个监控项 */
typedef struct
{
    uint8_t           used;
    uint8_t           auto_start;
    uint32_t          timeout_ms;
    TimerHandle_t     timer;
    MonitorCallback_t callback;
    char              name[16];
} MonitorItem_t;

/* 总监控器 */
typedef struct
{
    MonitorItem_t item[MONITOR_MAX_NUM];
} SystemMonitor_t;


typedef struct
{
    uint8_t ble_connected;   // 当前是否连接
    uint8_t ble_ever_connected; // 是否曾经连接过
} BleState_t;

extern BleState_t g_ble_state;

typedef struct
{
    uint8_t music_played;   // 当前是否播放
    uint8_t music_ever_played; // 是否曾经播放过
} MusicState_t;

extern MusicState_t g_music_state;

/*============================================================
 * 全局变量
 *===========================================================*/
extern SystemMonitor_t sys_monitor;


/*============================================================
 * 底层接口
 *===========================================================*/

/* 初始化 */
void SystemMonitor_Init(void);

/* 自动分配ID创建 */
MonitorID_t Monitor_Create(const char *name,
                           uint32_t timeout_ms,
                           uint8_t auto_start,
                           MonitorCallback_t cb);

/* 指定ID创建 */
MonitorID_t Monitor_CreateEx(MonitorID_t id,
                             const char *name,
                             uint32_t timeout_ms,
                             uint8_t auto_start,
                             MonitorCallback_t cb);

/* 启动 */
void Monitor_Start(MonitorID_t id);

/* 停止 */
void Monitor_Stop(MonitorID_t id);

/* 重启 / 喂狗 */
void Monitor_Restart(MonitorID_t id);

/* 删除 */
void Monitor_Delete(MonitorID_t id);

/* 修改超时 */
void Monitor_ChangeTimeout(MonitorID_t id, uint32_t timeout_ms);

/* 查询运行状态 */
uint8_t Monitor_IsActive(MonitorID_t id);


#ifdef __cplusplus
}
#endif

#endif