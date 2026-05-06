#include "oled_ui.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

UI_Global_t g_ui;

/* ================= 1. 组件定义 ================= */
static UI_Comp_t comp_weather;    
static UI_Comp_t comp_battery;    
static UI_Comp_t comp_page_root;  

// 独立控件组件，用于实现“每个控件独立动画”
static UI_Comp_t comp_title;      // 通用标题
static UI_Comp_t comp_icon;       // 通用图标/动画元素
static UI_Comp_t comp_desc;       // 通用描述文字

/* ================= 2. 内部辅助函数 ================= */
static void UI_DrawWeather_Internal(u8g2_t *u8g2, int x, int y, int8_t temp) {
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawCircle(u8g2, x + 10, y + 10, 5, U8G2_DRAW_ALL);
    char buf[8]; sprintf(buf, "%dC", temp);
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, x + 4, y + 28, buf);
}

static void UI_DrawStatusBarBattery(u8g2_t *u8g2, int x, int y, uint8_t pct) {
    u8g2_DrawFrame(u8g2, x, y, 16, 8);
    u8g2_DrawBox(u8g2, x + 2, y + 2, (12 * pct)/100, 4);
    u8g2_DrawBox(u8g2, x + 16, y + 2, 2, 4);
}

/* ================= 3. 核心逻辑 ================= */
void OLED_UI_Init(I2C_HandleTypeDef *hi2c) {
    memset(&g_ui, 0, sizeof(g_ui));
    OLED_Init(hi2c);
    UI_Popup_Init();
    srand((unsigned int)HAL_GetTick()); // 基于系统滴答定时器初始化随机数
    
    // 固定元素入场动画
    UI_Comp_SetLayout(&comp_weather, -30, 2, 2, 2, 0.08f, EaseOutCubic);
    UI_Comp_SetLayout(&comp_battery, 140, 2, 108, 2, 0.08f, EaseOutCubic);
    
    // 默认进入首页
    g_ui.cur_page = UI_PAGE_HOME;
    UI_Comp_SetLayout(&comp_page_root, 0, 0, 0, 0, 1.0f, Linear); 
    UI_Comp_SetLayout(&comp_title, -100, 20, 40, 20, 0.08f, EaseOutBack);
}

void OLED_UI_SetPage(UI_Page_t page) {
    if (g_ui.cur_page == page || g_ui.stage == UI_STAGE_EXITING) return;
    g_ui.next_page = page;
    g_ui.stage = UI_STAGE_EXITING;
    
    // 退出动画：根据当前页面决定控件散开方式
    switch (g_ui.cur_page) {
        case UI_PAGE_HOME:
            UI_Comp_SetLayout(&comp_title, 40, 20, -120, 20, 0.12f, EaseInBack);
            break;
        case UI_PAGE_PLAY:
            UI_Comp_SetLayout(&comp_title, 50, 20, 50, -30, 0.1f, EaseInBack);
            UI_Comp_SetLayout(&comp_icon, 50, 28, 50, 80, 0.08f, Linear);
            break;
        case UI_PAGE_STOP:
            UI_Comp_SetLayout(&comp_title, 45, 25, 45, 80, 0.1f, EaseInBack);
            break;
        default: break;
    }
    
    // 容器基准状态同步切换为退出
    UI_Comp_SetLayout(&comp_page_root, 0, 0, 0, 0, 0.12f, Linear);
    comp_page_root.state = UI_STATE_EXIT; 
}

void OLED_UI_Update(void) {
    UI_Popup_Update();

    // 退出动画完成后，切换到新页面并执行入场动画
    if (g_ui.stage == UI_STAGE_EXITING && comp_page_root.state == UI_STATE_DEAD) {
        g_ui.cur_page = g_ui.next_page;
        g_ui.stage = UI_STAGE_NORMAL;

        // 新页面控件入场动画
        switch (g_ui.cur_page) {
            case UI_PAGE_HOME:
                UI_Comp_SetLayout(&comp_title, 128, 20, 40, 20, 0.08f, EaseOutCubic);
                break;
            case UI_PAGE_PLAY:
                UI_Comp_SetLayout(&comp_title, 50, -20, 50, 20, 0.09f, EaseOutBack);
                UI_Comp_SetLayout(&comp_icon, 50, 80, 50, 28, 0.07f, EaseOutBounce);
                break;
            case UI_PAGE_STOP:
                UI_Comp_SetLayout(&comp_title, -100, 25, 45, 25, 0.1f, EaseOutCubic);
                UI_Comp_SetLayout(&comp_desc, 128, 40, 30, 40, 0.08f, EaseOutBack);
                break;
            default: break;
        }
        UI_Comp_SetLayout(&comp_page_root, 0, 0, 0, 0, 0.1f, Linear); 
    }

    // 驱动所有组件动画更新
    UI_Comp_Update(&comp_weather);
    UI_Comp_Update(&comp_battery);
    UI_Comp_Update(&comp_page_root);
    UI_Comp_Update(&comp_title);
    UI_Comp_Update(&comp_icon);
    UI_Comp_Update(&comp_desc);
}

void OLED_UI_Render(void) {
    u8g2_t *u8g2 = OLED_GetHandle();
    u8g2_ClearBuffer(u8g2);

    // 1. 渲染固定状态栏
    UI_DrawWeather_Internal(u8g2, (int)comp_weather.x, (int)comp_weather.y, g_ui.temp);
    UI_DrawStatusBarBattery(u8g2, (int)comp_battery.x, (int)comp_battery.y, g_ui.battery.percent);

    // 2. 渲染页面内容
    u8g2_SetFont(u8g2, u8g2_font_6x12_tf);
    switch (g_ui.cur_page) {
        case UI_PAGE_HOME:
            u8g2_DrawStr(u8g2, (int)comp_title.x, (int)comp_title.y, "HOME PAGE");
            break;
        case UI_PAGE_PLAY:
            u8g2_DrawStr(u8g2, (int)comp_title.x, (int)comp_title.y, g_ui.is_playing ? "PLAYING" : "PAUSED");
            // 频谱动画图标
            for(int i=0; i<6; i++) {
                int h = g_ui.is_playing ? (2 + rand() % 16) : 2;
                u8g2_DrawBox(u8g2, (int)comp_icon.x + i*8, (int)comp_icon.y - h, 5, h);
            }
            break;
        case UI_PAGE_STOP:
            u8g2_DrawStr(u8g2, (int)comp_title.x, (int)comp_title.y, "SYSTEM STOP");
            u8g2_DrawStr(u8g2, (int)comp_desc.x, (int)comp_desc.y, "Press Any Key");
            break;
        default: break;
    }

    // 3. 渲染顶层弹窗（带滑入/滑出动画）
    UI_Popup_Render(u8g2);
    u8g2_SendBuffer(u8g2);
}

/* --- 事件处理接口 --- */
void OLED_UI_OnEvent(UI_Event_t event) {
    switch (event) {
        case UI_EVT_PLAY:
            g_ui.is_playing = 1;
            UI_Popup_Show(Draw_SystemMsgPopup, "PLAY", 800);
            break;
        case UI_EVT_PAUSE:
            g_ui.is_playing = 0;
            UI_Popup_Show(Draw_SystemMsgPopup, "PAUSE", 800);
            break;
        case UI_EVT_STOP:
            g_ui.is_playing = 0;
            OLED_UI_SetPage(UI_PAGE_STOP);
            UI_Popup_Show(Draw_SystemMsgPopup, "STOPPED", 800);
            break;
        case UI_EVT_NEXT:
            UI_Popup_Show(Draw_SystemMsgPopup, "NEXT >>", 600);
            if (g_ui.cur_page == UI_PAGE_HOME) {
                OLED_UI_SetPage(UI_PAGE_PLAY);
            }
            break;
        case UI_EVT_PREV:
            UI_Popup_Show(Draw_SystemMsgPopup, "<< PREV", 600);
            if (g_ui.cur_page == UI_PAGE_HOME) {
                OLED_UI_SetPage(UI_PAGE_PLAY);
            }
            break;
        case UI_EVT_VOL_UP:
            UI_Popup_Show(Draw_SystemMsgPopup, "VOL +", 500);
            break;
        case UI_EVT_VOL_DOWN:
            UI_Popup_Show(Draw_SystemMsgPopup, "VOL -", 500);
            break;
        case UI_EVT_BATTERY_CHARGING:
            UI_Popup_Show(Draw_BatteryFullPopup, "", 3000);
            break;
        case UI_EVT_BATTERY_FULL:
            UI_Popup_Show(Draw_SystemMsgPopup, "BATTERY FULL", 1000);
            break;
        case UI_EVT_BATTERY_LOW:
            UI_Popup_Show(Draw_SystemMsgPopup, "LOW BATTERY", 1000);
            break;
         case UI_EVT_NONE:
            break;    
        default:
            break;
    }
}

void OLED_UI_SetBattery(uint8_t percent, uint8_t charging) {
    g_ui.battery.percent = percent;
    if (charging) UI_Popup_Show(Draw_BatteryFullPopup, NULL, 1000); 
}