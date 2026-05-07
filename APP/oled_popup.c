#include <string.h>
#include "oled_popup.h"
#include "FreeRTOS.h"
#include "queue.h"

// 静态全局变量，管理当前弹窗状态和任务队列
static UI_Popup_t g_popup;
static QueueHandle_t xPopupQueue = NULL;

/**
 * @brief 初始化弹窗管理器，创建消息队列
 */
void UI_Popup_Init(void) {
    memset(&g_popup, 0, sizeof(g_popup));
    // 创建弹窗队列，深度为10，可根据内存需求调整
    xPopupQueue = xQueueCreate(10, sizeof(PopupTask_t));
}

/**
 * @brief 将新的弹窗任务推入队列
 */
void UI_Popup_Show(PopupDrawFunc draw_cb, const char *msg, uint32_t ms) {
    if (xPopupQueue == NULL || draw_cb == NULL || msg == NULL) return;
    
    PopupTask_t task;
    task.draw_cb = draw_cb;
    strncpy(task.msg_buf, msg, POPUP_MSG_BUF_LEN - 1);
    task.msg_buf[POPUP_MSG_BUF_LEN - 1] = '\0'; // 保证字符串终止
    task.ms = ms;
    
    // 非阻塞发送，如果队列满了则忽略该弹窗（防止内存溢出或逻辑阻塞）

    xQueueSend(xPopupQueue, &task, 0);
}

/**
 * @brief 弹窗逻辑状态机更新
 */
void UI_Popup_Update(void) {
    // 1. 如果当前没有活跃弹窗，尝试从队列领取新任务
    if (!g_popup.is_active) {
        PopupTask_t next_task;
        if (xQueueReceive(xPopupQueue, &next_task, 0) == pdPASS) {
            g_popup.draw_cb = next_task.draw_cb;
            // 安全字符串拷贝（修复越界风险）
            strncpy(g_popup.msg_buf, next_task.msg_buf, POPUP_MSG_BUF_LEN - 1);
            g_popup.msg_buf[POPUP_MSG_BUF_LEN - 1] = '\0';
            
            // 假设任务周期为 16ms (60FPS)，计算计次数
            g_popup.timer = next_task.ms / 16; 
            if (g_popup.timer == 0) g_popup.timer = 1;
            
            g_popup.is_active = 1;
            // 触发进入动画：从上方 -32 处滑落至 0
            UI_Comp_SetLayout(&g_popup.base, 0, -32, 0, 0, 0.08f, EaseOutBack);
        }
        return;
    }

    // 2. 更新组件坐标计算
    UI_Comp_Update(&g_popup.base);

    // 3. 停留阶段计时：只有进入动画完成后才开始倒计时
    if (g_popup.base.state == UI_STATE_IDLE) {
        if (g_popup.timer > 0) {
            g_popup.timer--;
        } else {
            // 触发滑出动画：从0滑回-32，状态设为EXIT
            UI_Comp_SetLayout(&g_popup.base, 0, 0, 0, -32, 0.1f, EaseInBack);
            g_popup.base.state = UI_STATE_EXIT;
        }
    }

    // 4. 退出动画完成后释放活跃标志
    if (g_popup.base.state == UI_STATE_DEAD) {
        g_popup.is_active = 0;
    }
}

/**
 * @brief 弹窗层级渲染
 */
void UI_Popup_Render(u8g2_t *u8g2) {
    // 只有在活跃状态且回调存在时才执行渲染
    if (g_popup.is_active && g_popup.draw_cb) {
        g_popup.draw_cb(u8g2, &g_popup.base, g_popup.msg_buf);
    }
}

/**
 * @brief 通用系统消息弹窗渲染回调
 */
void Draw_SystemMsgPopup(u8g2_t *u8g2, UI_Comp_t *base, void *data) {
    char *msg = (char *)data;
    if (msg == NULL) return;

    // 获取动画实时y坐标
    int y_pos = (int)base->y; 

    // 文字字体（必须和绘制文字用同一个）
    u8g2_SetFont(u8g2, u8g2_font_6x12_tf);
    
    // ===================== 核心改动 =====================
    // 1. 先获取文字宽度
    int str_w = u8g2_GetStrWidth(u8g2, msg);
    
    // 2. 背景宽度 = 文字宽度 + 左右内边距（左右各留8像素，可自己改）
    int box_w = str_w + 16;  
    int box_h = 18;          // 背景高度固定
    int box_r = 4;           // 圆角半径
    
    // 3. 背景框居中（屏幕128宽）
    int box_x = (128 - box_w) / 2;  // 自动居中X
    int box_y = y_pos + 6;          // Y不变
    // ===================================================

    // 绘制圆角背景（白色）
    u8g2_SetDrawColor(u8g2, 1);
    u8g2_DrawRBox(u8g2, box_x, box_y, box_w, box_h, box_r); 

    // 绘制反色文字（黑色）
    u8g2_SetDrawColor(u8g2, 0);
    int start_x = (128 - str_w) / 2;  // 文字居中
    int start_y = y_pos + 19;
    u8g2_DrawStr(u8g2, start_x, start_y, msg);

    // 还原画笔颜色
    u8g2_SetDrawColor(u8g2, 1);
}

/**
 * @brief 电池满电/充电弹窗渲染回调
 */
void Draw_BatteryFullPopup(u8g2_t *u8g2, UI_Comp_t *base, void *data) {
    int y = (int)base->y;

    // ===================== 优化部分 =====================
    // 1. 全屏黑色背景（覆盖128x32）
    u8g2_SetDrawColor(u8g2, 1);    // 白色
    u8g2_DrawBox(u8g2, 0, y, 128, 32);  // 全屏涂黑

    // 2. 大号电池（尺寸放大，居中，接近全屏但略小）
    u8g2_SetDrawColor(u8g2, 0);    // 黑色绘制电池
    u8g2_DrawFrame(u8g2, 10, y + 2, 108, 28);   // 电池外框 放大版
    u8g2_DrawBox(u8g2, 118, y + 8, 6, 16);     // 电池极头

    // 3. 充电流水动画（适配新尺寸）
    int flow = (xTaskGetTickCount() / 150) % 21;
    if(flow > 0) {
        u8g2_DrawBox(u8g2, 14, y + 6, flow * 5, 20);
    }
    // ====================================================
}