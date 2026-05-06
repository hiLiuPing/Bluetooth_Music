#ifndef __OLED_POPUP_H__
#define __OLED_POPUP_H__

#include "oled_anim.h"
#include "oled_u8g2.h"
#define POPUP_MSG_BUF_LEN 32
/**
 * @brief 弹窗渲染回调函数类型
 * @param u8g2    U8G2 句柄
 * @param base    指向动画组件基类，用于获取实时 y 坐标[cite: 8]
 * @param data    用户自定义数据（如字符串或数值指针）[cite: 8]
 */
typedef void (*PopupDrawFunc)(u8g2_t *u8g2, UI_Comp_t *base, void *data);

/**
 * @brief 弹窗任务结构体（用于 FreeRTOS 队列传输）
 */
typedef struct {
    PopupDrawFunc draw_cb;
    char msg_buf[POPUP_MSG_BUF_LEN]; // 拷贝字符串，避免野指针
    uint32_t ms;
} PopupTask_t;

/**
 * @brief 弹窗管理结构体
 */
typedef struct {
    UI_Comp_t base;        // 动画基类[cite: 8]
    PopupDrawFunc draw_cb; // 当前渲染函数[cite: 8]
    char msg_buf[POPUP_MSG_BUF_LEN]; // 安全字符串
    uint32_t timer;        // 内部倒计时计数器[cite: 7, 8]
    uint8_t is_active;     // 激活状态标志[cite: 8]
} UI_Popup_t;

/* --- 核心生命周期函数 --- */

/**
 * @brief 初始化弹窗管理器及 FreeRTOS 消息队列[cite: 7, 8]
 */
void UI_Popup_Init(void);

/**
 * @brief 将弹窗任务发送至队列，支持多个弹窗排队显示
 * @param draw_cb 绘图回调函数
 * @param data    要显示的数据
 * @param ms      停留时间
 */
void UI_Popup_Show(PopupDrawFunc draw_cb, const char *msg, uint32_t ms);

/**
 * @brief 弹窗逻辑更新，处理动画状态切换及队列提取[cite: 7, 8]
 */
void UI_Popup_Update(void);

/**
 * @brief 弹窗渲染，执行当前活跃任务的回调函数[cite: 7, 8]
 */
void UI_Popup_Render(u8g2_t *u8g2);

/* --- 预设弹窗内容渲染函数 (需在 oled_ui_5.c 中实现) --- */
void Draw_BatteryFullPopup(u8g2_t *u8g2, UI_Comp_t *base, void *data);
void Draw_SystemMsgPopup(u8g2_t *u8g2, UI_Comp_t *base, void *data);

#endif