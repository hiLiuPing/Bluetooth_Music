/*============================================================
 * File: SystemMonitor_app.c
 *===========================================================*/
#include "SystemMonitor_app.h"
#include "music_app.h"
#include "log.h"

/* 超时回调 */
static void BleDisconnectTimeout(TimerHandle_t xTimer)
{
    // System_PowerOff();
        log_printf("BLE disconnect timeout.");
}

static void AudioIdleTimeout(TimerHandle_t xTimer)
{
    // Music_PowerOff();
    log_printf("Audio idle timeout.");
}

static void KeyIdleTimeout(TimerHandle_t xTimer)
{
    // Music_PowerOff();
    log_printf("Key idle timeout.");

}



/* 初始化业务监控 */
void UserMonitor_Init(void)
{
    SystemMonitor_Init();

    Monitor_CreateEx(MON_BLE_IDLE,
                     "BLE_OFF",
                     300000,
                     0,
                     BleDisconnectTimeout);

    Monitor_CreateEx(MON_AUDIO_IDLE,
                     "AUDIO_NO",
                     300000,
                     0,
                     AudioIdleTimeout);

    Monitor_CreateEx(MON_KEY_IDLE,
                     "KEY_IDLE",
                     7200000,
                     1,
                     KeyIdleTimeout);


}


/* 蓝牙连接 */
void Bluetooth_Connected(void)
{
    Monitor_Stop(MON_BLE_IDLE);
}

/* 蓝牙断开 */
void Bluetooth_Disconnected(void)
{
    Monitor_Start(MON_BLE_IDLE);

}

/* 音乐播放 */
void Music_Playing(void)
{
    Monitor_Stop(MON_AUDIO_IDLE);
}

/* 音乐暂停 */
void Music_Pause(void)
{
    Monitor_Start(MON_AUDIO_IDLE);
}

/* 任意按键 */
void Key_Event(void)
{
    Monitor_Restart(MON_KEY_IDLE);
}


// void AppState_InputHandle(ButtonCommand_t *keycmd)
// {
//                     switch(keycmd->id)
//                     {
//                         case 3: // 蓝牙状态
//                             if(keycmd->event == BTN_PRESS_UP) // 接通
//                             {

//                       g_ble_state.ble_connected = 1;
//                       g_ble_state.ble_ever_connected = 1;
//                        Bluetooth_Connected();

//                                   log_printf("BTN_PRESS_UP Button %d, event %d", keycmd->id, keycmd->event); 
//                             }
//                             else if(keycmd->event == BTN_PRESS_DOWN) // 断开
//                             {
//                                g_ble_state.ble_connected = 0;
//                                  if(g_ble_state.ble_ever_connected)
//                                 {
//                                       Bluetooth_Disconnected();
//                                 }

//                                 log_printf("BTN_PRESS_DOWN Button %d, event %d", keycmd->id, keycmd->event); 
//                             }
//                             break;

//                         case 4: // 播放状态
//                             if(keycmd->event == BTN_PRESS_UP) // 短按
//                             {
//                             g_music_state.music_played = 1;
//                            g_music_state.music_ever_played = 1;
//                             Music_Playing();
//                                   log_printf("BTN_PRESS_UP Button %d, event %d", keycmd->id, keycmd->event); 
//                             }
//                             else if(keycmd->event == BTN_PRESS_DOWN) // 双击
//                             {

//                                  g_music_state.music_played = 0;
//                                  if(g_music_state.music_ever_played)
//                                 {

//                                       Music_Pause();
//                                 }
//                                 log_printf("BTN_PRESS_DOWN Button %d, event %d", keycmd->id, keycmd->event); 
//                             }
//                             break;

//                         default:
//                             break;
//                     }

// // 每次按键进来都要重置定时休眠定时器
//                      Key_Event();
// }