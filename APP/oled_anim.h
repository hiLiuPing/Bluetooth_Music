#ifndef __OLED_ANIM_H__
#define __OLED_ANIM_H__

/* 定义缓动函数指针类型 */
typedef float (*EasingFunc_t)(float);

/* 常用缓动曲线声明 */
float Linear(float t);
float EaseOutCubic(float t);
float EaseInBack(float t);
float EaseOutBack(float t);
float EaseOutBounce(float t);

typedef enum {
    UI_STATE_IDLE,
    UI_STATE_MOVE,
    UI_STATE_EXIT,
    UI_STATE_DEAD
} UI_State_t;

typedef struct {
    float x, y;             // 当前坐标
    float target_x, target_y; // 目标坐标
    float start_x, start_y;   // 起始坐标
    float t;                // 进度 (0.0 to 1.0)
    float speed;            // 步进速度
    EasingFunc_t easing_func; // 缓动曲线函数指针
    UI_State_t state;
} UI_Comp_t;

/* 函数接口 */
void UI_Comp_SetLayout(UI_Comp_t *comp, float sx, float sy, float tx, float ty, float speed, EasingFunc_t func);
void UI_Comp_Update(UI_Comp_t *comp);

#endif