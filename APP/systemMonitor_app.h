/*============================================================
 * File: SystemMonitor_app.h
 * 播放器项目业务层监控
 *===========================================================*/
#ifndef __SYSTEMMONITOR_APP_H
#define __SYSTEMMONITOR_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "SystemMonitor.h"
#include "key_app.h"

/*============================================================
 * 业务监控ID
 *===========================================================*/
typedef enum
{
    MON_BLE_IDLE = 0,
    MON_AUDIO_IDLE,
    MON_KEY_IDLE,

    MON_APP_MAX
} AppMonitorID_t;


/*============================================================
 * 初始化
 *===========================================================*/
void UserMonitor_Init(void);


/*============================================================
 * 蓝牙事件
 *===========================================================*/
void Bluetooth_Connected(void);
void Bluetooth_Disconnected(void);


/*============================================================
 * 音乐事件
 *===========================================================*/
void Music_Playing(void);
void Music_Pause(void);


/*============================================================
 * 按键事件
 *===========================================================*/
void Key_Event(void);
// void AppState_InputHandle(ButtonCommand_t *keycmd);



#ifdef __cplusplus
}
#endif

#endif