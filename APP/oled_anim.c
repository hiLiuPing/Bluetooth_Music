#include "oled_anim.h"
#include <math.h>

// 线性
float Linear(float t) { return t; }

// 三次减速 (滑入常用)[cite: 3]
float EaseOutCubic(float t) {
    t -= 1.0f;
    return t * t * t + 1.0f;
}

// 后面带拉回效果的进入 (优化：移除 powf)
float EaseOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    float t1 = t - 1.0f;
    // 使用乘法代替 powf(t-1.0f, 3) 和 powf(t-1.0f, 2)[cite: 7]
    return 1.0f + c3 * (t1 * t1 * t1) + c1 * (t1 * t1);
}

// 预备动作后加速 (优化：移除 powf)[cite: 7]
float EaseInBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    // 使用乘法代替 t*t*t 和 t*t[cite: 7]
    return c3 * (t * t * t) - c1 * (t * t);
}

// 弹跳效果[cite: 3]
float EaseOutBounce(float t) {
    if (t < (1 / 2.75f)) return 7.5625f * t * t;
    else if (t < (2 / 2.75f)) return 7.5625f * (t -= (1.5f / 2.75f)) * t + 0.75f;
    else if (t < (2.5 / 2.75f)) return 7.5625f * (t -= (2.25f / 2.75f)) * t + 0.9375f;
    else return 7.5625f * (t -= (2.625f / 2.75f)) * t + 0.984375f;
}

/* 更新组件位置的逻辑[cite: 1, 3] */
void UI_Comp_Update(UI_Comp_t *comp) {
    if (comp->state == UI_STATE_IDLE || comp->state == UI_STATE_DEAD) return;

    comp->t += comp->speed;
    if (comp->t >= 1.0f) {
        comp->t = 1.0f;
        comp->x = comp->target_x;
        comp->y = comp->target_y;
        // 如果是退出状态，完成后转为 DEAD[cite: 3]
        comp->state = (comp->state == UI_STATE_EXIT) ? UI_STATE_DEAD : UI_STATE_IDLE;
    } else {
        float val = comp->easing_func(comp->t);
        comp->x = comp->start_x + (comp->target_x - comp->start_x) * val;
        comp->y = comp->start_y + (comp->target_y - comp->start_y) * val;
    }
}

void UI_Comp_SetLayout(UI_Comp_t *comp, float sx, float sy, float tx, float ty, float speed, EasingFunc_t func) {
    comp->start_x = sx; comp->start_y = sy;
    comp->target_x = tx; comp->target_y = ty;
    comp->x = sx; comp->y = sy;
    comp->speed = speed;
    comp->t = 0.0f;
    comp->easing_func = func;
    comp->state = UI_STATE_MOVE;
}