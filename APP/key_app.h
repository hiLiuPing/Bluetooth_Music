#ifndef __KEY_APP_H__
#define __KEY_APP_H__

#include "main.h"
#include "multi_button.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define MAX_KEY_NUM 3

/* ================= 按键对象 ================= */
extern  Button key_left;
extern  Button key_middle;
extern  Button key_right;   
extern Button VIRTUAL_BLE_STATE;
extern Button VIRTUAL_AUDIO_STATE;
/* 队列句柄 */




typedef struct {
    //  Button *btn; // ✅ 正确
       uint8_t id;
    ButtonEvent event;
} ButtonCommand_t;

/* ================= 按键初始化 ================= */
void Key_Init(void);

/* ================= 按键扫描（定时调用） ================= */
void Key_Scan(void);

#endif /* __KEY_H__ */
