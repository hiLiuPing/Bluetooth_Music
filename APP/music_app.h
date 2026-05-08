#ifndef __MUSIC_APP_H__
#define __MUSIC_APP_H__

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// // 连接状态
// typedef enum
// {
//     CONN_DISCONNECTED,  // 未连接
//     CONN_CONNECTED      // 已连接
// } ConnectStatus;

// // 播放状态
// typedef enum
// {
//     PLAY_PAUSE,     // 暂停
//     PLAY_PLAY       // 播放
// } PlayStatus;

// // 音频状态
// typedef enum
// {
//     AUDIO_NONE,     // 无音频
//     AUDIO_HAVE      // 有音频
// } AudioStatus;

// 音乐控制命令
typedef enum
{
    CMD_PLAY_STOP,  // 播放/暂停   
    CMD_PREV,       // 上一曲
    CMD_NEXT,       // 下一曲
    CMD_PAIR,       // 配对
    CMD_CLEAR_PAIR, // 清除配对记录
    CMD_POWER_ON,   // 开机
    CMD_POWER_OFF,  // 关机
    CMD_VOL_UP,     // 音量+
    CMD_VOL_DOWN    // 音量-
} MusicCtrlCmd;

// // 全局状态结构体（三合一，方便管理）
// typedef struct {
//     ConnectStatus conn;   // 连接状态
//     PlayStatus    play;   // 播放状态
//     AudioStatus   audio;  // 音频存在状态
// } MusicStatus_t;
typedef struct
{
    GPIO_TypeDef* port;
    uint16_t      pin;
} KeyGpioTypeDef;

// 按键编号定义（必须先定义！）
typedef enum
{
    KEY1,   // BULU_PWR_EN
    KEY2,   // CONNECT
    KEY3,   // DOWN
    KEY4,   // UP
    KEY5,   // PLAY
    KEY6,   // MUSIC_ON
    KEY_NUM // 总数 6
} KeyIndexTypeDef;

// 全局变量
// extern MusicStatus_t g_music_status;
extern QueueHandle_t music_cmd_queue;

// 函数
void music_app_init(void);
void music_send_cmd(MusicCtrlCmd cmd);
// void Music_Ctrl_Init(void);

// 每个功能独立函数
void Music_Play_Stop(void);
void Music_Up(void);
void Music_Next(void);
void Music_Pair(void);
void Music_ClearPair(void);
void Music_PowerOn(void);
void Music_PowerOff(void);
void Music_VolumeUp(void);
void Music_VolumeDown(void);
void System_PowerOff(void);

// 多按键模拟（通用函数）
void SimKey_Click(KeyIndexTypeDef key);          // 单击
void SimKey_DoubleClick(KeyIndexTypeDef key);    // 双击
void SimKey_LongPress(KeyIndexTypeDef key, float sec); // 长按(秒)


#endif