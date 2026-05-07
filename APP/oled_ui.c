#include "oled_ui.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>  // 必须加，解决 sinf 未定义报错
#include "music_app.h"
        #include "icons.h"



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

// 在屏幕 (x, y) 位置显示 32x32 图标
u8g2_DrawXBM(u8g2, x, y, 32, 32, U8_IMG_100);
    char buf[8]; sprintf(buf, "%dC", temp);
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(u8g2, x + 34, y + 10, buf);
}
// 充电进度动画计数器
static uint32_t s_battery_charge_anim_tick = 0;
/* ================= 优化后的充电速度 ================= */
static void UI_DrawStatusBarBattery(u8g2_t *u8g2, int x, int y, uint8_t pct, uint8_t charging) {
    u8g2_DrawFrame(u8g2, x, y, 16, 8);
    u8g2_DrawBox(u8g2, x + 16, y + 2, 2, 4);

    if (pct > 100) pct = 100;
    int max_fill = (12 * pct) / 100;

    if (charging) {
        s_battery_charge_anim_tick++;

        // 将除数从 3 增加到 8，每 8 帧上涨 1 像素，视觉效果更平缓
        int anim_progress = (s_battery_charge_anim_tick / 8) % (max_fill + 1);

        u8g2_SetDrawColor(u8g2, 0);
        u8g2_DrawBox(u8g2, x + 2, y + 2, 12, 4);

        u8g2_SetDrawColor(u8g2, 1);
        u8g2_DrawBox(u8g2, x + 2, y + 2, anim_progress, 4);
    } else {
        u8g2_SetDrawColor(u8g2, 0);
        u8g2_DrawBox(u8g2, x + 2, y + 2, 12, 4);

        u8g2_SetDrawColor(u8g2, 1);
        u8g2_DrawBox(u8g2, x + 2, y + 2, max_fill, 4);
    }
}
void OLED_UI_SetBattery(uint8_t percent, uint8_t charging) {
    g_ui.battery.percent = percent;
    g_ui.battery.is_charging = charging; // 新增：标记充电状态
    if (charging) UI_Popup_Show(Draw_BatteryFullPopup, "", 3000); 
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
    UI_Comp_SetLayout(&comp_title, 65, -20, 65, 20, 0.08f, EaseOutBack);
}
// 退场动画，跟入场坐标相反
void OLED_UI_SetPage(UI_Page_t page) {
    if (g_ui.cur_page == page || g_ui.stage == UI_STAGE_EXITING) return;
    g_ui.next_page = page;
    g_ui.stage = UI_STAGE_EXITING;
    
    // 退出动画：根据当前页面决定控件散开方式
    switch (g_ui.cur_page) {
        case UI_PAGE_HOME:
            UI_Comp_SetLayout(&comp_title, 65, 20, 65, -20, 0.12f, EaseInBack);
            break;
        case UI_PAGE_PLAY:
            UI_Comp_SetLayout(&comp_icon, 50, 28, 50, 80, 0.08f, Linear);
            break;
        case UI_PAGE_STOP:
            UI_Comp_SetLayout(&comp_title, 65, 20, 65, -20, 0.12f, EaseInBack);
            break;
        default: break;
    }
    
    // 容器基准状态同步切换为退出
    UI_Comp_SetLayout(&comp_page_root, 0, 0, 0, 0, 0.12f, Linear);
    comp_page_root.state = UI_STATE_EXIT; 
}
// 入场动画，跟退场坐标相反
void OLED_UI_Update(void) {
    UI_Popup_Update();

    // 退出动画完成后，切换到新页面并执行入场动画
    if (g_ui.stage == UI_STAGE_EXITING && comp_page_root.state == UI_STATE_DEAD) {
        g_ui.cur_page = g_ui.next_page;
        g_ui.stage = UI_STAGE_NORMAL;

        // 新页面控件入场动画
        switch (g_ui.cur_page) {
            case UI_PAGE_HOME:
                UI_Comp_SetLayout(&comp_title, 65, -20, 65, 20, 0.08f, EaseOutCubic);
                break;
            case UI_PAGE_PLAY:
                UI_Comp_SetLayout(&comp_icon, 50, 80, 50, 28, 0.07f, EaseOutBounce);
                break;
            case UI_PAGE_STOP:
                UI_Comp_SetLayout(&comp_title, 65, -20, 65, 20, 0.12f, EaseInBack);
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
// 正弦波频谱动画 全局计数器
static uint16_t s_spectrum_anim_tick = 0;

// 独立封装：绘制音乐频谱动画（正弦波平滑动画）
/* ================= 优化后的频谱动画 ================= */
static void UI_DrawSpectrum(u8g2_t *u8g2, int x, int y) {
    // 1. 定义每根柱子的最大随机振幅权重，避免高度过于统一
    const float amplitudes[6] = {10.0f, 6.0f, 9.0f, 5.0f, 8.0f, 4.0f};
    
    for (int i = 0; i < 6; i++) {
        // 2. 提高角频率系数（从 0.3f 增加到 0.5f），使波动变快
        // 3. 增大相位差（从 0.9f 增加到 1.2f），让波形错落感更强
        float rad = (s_spectrum_anim_tick * 0.5f) + (i * 1.2f);
        
        // 使用 (0.5 + 0.5 * sin) 将正弦值映射到 [0, 1] 区间，再乘以权重
        // 这样可以保证高度始终为正，且波动更平滑
        int h = 2 + (int)(amplitudes[i] * (0.5f + 0.5f * sinf(rad)));

        // 绘制单根频谱柱 (宽度5, 间距3)
        u8g2_DrawBox(u8g2, x + i * 8, y - h, 5, h);
    }

    s_spectrum_anim_tick++;
}
void OLED_UI_Render(void) {
    u8g2_t *u8g2 = OLED_GetHandle();
    u8g2_ClearBuffer(u8g2);

    // 1. 渲染固定状态栏
    UI_DrawWeather_Internal(u8g2, (int)comp_weather.x, (int)comp_weather.y, g_ui.temp);
    // UI_DrawStatusBarBattery(u8g2, (int)comp_battery.x, (int)comp_battery.y, g_ui.battery.percent);
UI_DrawStatusBarBattery(u8g2, 
                        (int)comp_battery.x, 
                        (int)comp_battery.y, 
                        g_ui.battery.percent, 
                        g_ui.battery.is_charging); // 新增充电状态参数
    // 2. 渲染页面内容
    u8g2_SetFont(u8g2, u8g2_font_6x12_tf);
    switch (g_ui.cur_page) {
        case UI_PAGE_HOME:
            u8g2_DrawStr(u8g2, (int)comp_title.x, (int)comp_title.y, "HOME");
            break;
        case UI_PAGE_PLAY:
            // u8g2_DrawStr(u8g2, (int)comp_title.x, (int)comp_title.y, g_ui.is_playing ? "PLAYING" : "PAUSED");
            // 频谱动画图标
            UI_DrawSpectrum(u8g2, (int)comp_icon.x, (int)comp_icon.y);
            break;
        case UI_PAGE_STOP:
            u8g2_DrawStr(u8g2, (int)comp_title.x, (int)comp_title.y, "STOP");
            // u8g2_DrawStr(u8g2, (int)comp_desc.x, (int)comp_desc.y, "Press Any Key");
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
            UI_Popup_Show(Draw_SystemMsgPopup, "PLAY", 800);
            break;
        case UI_EVT_PAUSE:
            UI_Popup_Show(Draw_SystemMsgPopup, "PAUSE", 800);
            break;
        case UI_EVT_STOP:
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
