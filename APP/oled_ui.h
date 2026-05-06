#ifndef __OLED_UI_H__
#define __OLED_UI_H__

#include "oled_anim.h"
#include "oled_popup.h"
#include "FreeRTOS.h"
#include "queue.h"

/* 页面枚举定义 */
typedef enum {
    UI_PAGE_HOME = 0,
    UI_PAGE_PLAY,
    UI_PAGE_STOP,    // 新增：停止页面[cite: 6]
    UI_MAX_PAGE
} UI_Page_t;

/* UI 运行阶段 */
typedef enum {
    UI_STAGE_NORMAL = 0,
    UI_STAGE_EXITING  // 正在执行退出动画
} UI_Stage_t;

/* 系统事件 */
typedef enum {
    UI_EVT_NONE = 0,
    UI_EVT_PREV,
    UI_EVT_NEXT,
    UI_EVT_PLAY,
    UI_EVT_PAUSE,
    UI_EVT_STOP,
    UI_EVT_VOL_UP,
    UI_EVT_VOL_DOWN,
    UI_EVT_BATTERY_LOW, // 电量低事件
    UI_EVT_BATTERY_FULL, // 电量充满事件
    UI_EVT_BATTERY_CHARGING, // 电量充电事件
} UI_Event_t;

/* 全局 UI 状态结构体 */
typedef struct {
    UI_Page_t cur_page;
    UI_Page_t next_page;
    UI_Stage_t stage;
    struct { 
        uint8_t percent; 
        uint8_t is_charging; 
    } battery;
    int8_t temp;          
    uint8_t is_playing;
} UI_Global_t;

extern UI_Global_t g_ui;

/* 核心生命周期函数 */
void OLED_UI_Init(I2C_HandleTypeDef *hi2c);
void OLED_UI_Update(void);
void OLED_UI_Render(void);

/* 外部控制接口 */
void OLED_UI_SetPage(UI_Page_t page);
void OLED_UI_OnEvent(UI_Event_t event);
void OLED_UI_SetBattery(uint8_t percent, uint8_t charging);

#endif