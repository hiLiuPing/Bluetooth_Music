#include "music_app.h"
#include <stdio.h>
#include "log.h"
// 全局状态变量
// MusicStatus_t g_music_status;

// 命令队列

extern QueueHandle_t music_cmd_queue;
// ===================== 发送命令 =====================
void music_send_cmd(MusicCtrlCmd cmd)
{
    if(music_cmd_queue != NULL)
    {
        xQueueSend(music_cmd_queue, &cmd, 0);
    }
}
// ==================== 多按键 GPIO 配置（自己改引脚） ====================

// #define BULU_PWR_EN_Pin GPIO_PIN_0
// #define BULU_PWR_EN_GPIO_Port GPIOC
// #define CONNECT_Pin GPIO_PIN_1
// #define CONNECT_GPIO_Port GPIOC
// #define DOWN_Pin GPIO_PIN_2
// #define DOWN_GPIO_Port GPIOC
// #define UP_Pin GPIO_PIN_3
// #define UP_GPIO_Port GPIOC
// #define PLAY_Pin GPIO_PIN_1
// #define PLAY_GPIO_Port GPIOA
// #define MUSIC_ON_Pin GPIO_PIN_1
// #define MUSIC_ON_GPIO_Port GPIOE
const KeyGpioTypeDef key_gpio[KEY_NUM] = {
    {BULU_PWR_EN_GPIO_Port, BULU_PWR_EN_Pin},  // KEY1
    {CONNECT_GPIO_Port, CONNECT_Pin},  // KEY2
    {DOWN_GPIO_Port, DOWN_Pin},  // KEY3
    {UP_GPIO_Port, UP_Pin},  // KEY4
    {PLAY_GPIO_Port, PLAY_Pin},  // KEY5
    {MUSIC_ON_GPIO_Port, MUSIC_ON_Pin}  // KEY6
};

// ==================== 初始化 ====================
// void Music_Ctrl_Init(void)
// {
//     // 状态初始化
//     // g_music_status.conn  = CONN_DISCONNECTED;
//     // g_music_status.play  = PLAY_PAUSE;
//     // g_music_status.audio = AUDIO_NONE;

//     // 所有按键默认低电平
//     for(int i=0; i<KEY_NUM; i++)
//     {
//         HAL_GPIO_WritePin(key_gpio[i].port, key_gpio[i].pin, GPIO_PIN_RESET);
//     }
// }


// ==================== 模拟单击 ====================
void SimKey_Click(KeyIndexTypeDef key)
{
    if(key >= KEY_NUM) return;

    HAL_GPIO_WritePin(key_gpio[key].port, key_gpio[key].pin, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(100));

    HAL_GPIO_WritePin(key_gpio[key].port, key_gpio[key].pin, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(50));
}

// ==================== 模拟双击 ====================
void SimKey_DoubleClick(KeyIndexTypeDef key)
{
    SimKey_Click(key);
    vTaskDelay(pdMS_TO_TICKS(100));
    SimKey_Click(key);
}

// ==================== 模拟长按（可设秒数） ====================
void SimKey_LongPress(KeyIndexTypeDef key, float sec)
{
    if(key >= KEY_NUM) return;

    uint32_t ms = sec * 1000;

    HAL_GPIO_WritePin(key_gpio[key].port, key_gpio[key].pin, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(ms));

    HAL_GPIO_WritePin(key_gpio[key].port, key_gpio[key].pin, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(100));
}

//=============================
// 功能函数：每个独立实现
//=============================
void Music_Up(void)
{
    log_printf("Music_Up\r\n");
    SimKey_Click(KEY4); // 模拟上键单击
    // 在这里写你的硬件上一曲代码
}
void Music_Play_Stop(void)
{
    log_printf("Music_Play_Stop\r\n");
    SimKey_Click(KEY5); // 模拟播放/暂停键单击
    // 在这里写你的硬件播放/暂停代码
}
void Music_Next(void)
{
    log_printf("Music_Next\r\n");
    SimKey_Click(KEY3); // 模拟下键单击
    // 在这里写你的硬件下一曲代码5
}
// const KeyGpioTypeDef key_gpio[KEY_NUM] = {
//     {GPIOC, BULU_PWR_EN_Pin},  // KEY1
//     {GPIOC, CONNECT_Pin},  // KEY2
//     {GPIOC, DOWN_Pin},  // KEY3
//     {GPIOC, UP_Pin},  // KEY4
//     {GPIOA, PLAY_Pin},  // KEY5
//     {GPIOE, MUSIC_ON_Pin}  // KEY6
// };
void Music_Pair(void)
{
    // log_printf("执行：配对\r\n");
    log_printf("Music_Pair\r\n");
    SimKey_LongPress(KEY5, 3); // 模拟配对键长按3S
    // g_music_status.conn = CONN_CONNECTED;
}

void Music_ClearPair(void)
{
    // log_printf("执行：清除配对记录\r\n");
    log_printf("Music_ClearPair\r\n");
    SimKey_LongPress(KEY5, 5); // 模拟配对键长按5S
    // g_music_status.conn = CONN_DISCONNECTED;
}

void Music_PowerOn(void)
{

    log_printf("Music_PowerOn\r\n");

// 先打开AD电源，再打开蓝牙电源，确保稳定开机
    HAL_GPIO_WritePin(AD_PWER_EN_GPIO_Port, AD_PWER_EN_Pin, GPIO_PIN_SET);
    vTaskDelay(pdMS_TO_TICKS(50));
    HAL_GPIO_WritePin(BULU_PWR_EN_GPIO_Port, BULU_PWR_EN_Pin, GPIO_PIN_SET);

}

void Music_PowerOff(void)
{
    // log_printf("执行：关机\r\n");
    log_printf("Music_PowerOff\r\n");
    // 先关闭蓝牙电源，再关闭AD电源，确保稳定关机
     HAL_GPIO_WritePin(BULU_PWR_EN_GPIO_Port, BULU_PWR_EN_Pin, GPIO_PIN_RESET);
    vTaskDelay(pdMS_TO_TICKS(50));
    HAL_GPIO_WritePin(AD_PWER_EN_GPIO_Port, AD_PWER_EN_Pin, GPIO_PIN_RESET);  
}

void Music_VolumeUp(void)
{
    // log_printf("执行：音量+\r\n");
    log_printf("Music_VolumeUp\r\n");
    SimKey_LongPress(KEY3, 2); // 模拟下键长按2S
}

void Music_VolumeDown(void)
{
    // log_printf("执行：音量-\r\n");
    log_printf("Music_VolumeDown\r\n");
    SimKey_LongPress(KEY4, 2); // 模拟上键长按2S
}
// 系统关机
void System_PowerOff(void)
{
    // log_printf("执行：音量-\r\n");
    log_printf("System_PowerOff\r\n");
    // SimKey_LongPress(KEY4, 2); // 模拟上键长按2S
    // #define ARM_RST_Pin GPIO_PIN_2
// #define ARM_RST_GPIO_Port GPIOB
    HAL_GPIO_WritePin(ARM_RST_GPIO_Port, ARM_RST_Pin, GPIO_PIN_RESET); 
}

