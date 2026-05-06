#include "key_app.h"
#include "main.h"
#include "log.h"
/* ================= 按键对象 ================= */

Button key_left;
Button key_middle;
Button key_right;
Button VIRTUAL_BLE_STATE;
Button VIRTUAL_AUDIO_STATE;


/* 队列句柄 */
 QueueHandle_t buttonQueue;

/* ================= GPIO 读取函数 ================= */
/* 注意：button_id 0 -> KEY1, 1 -> KEY2 */

// #define SW3_Pin GPIO_PIN_6
// #define SW3_GPIO_Port GPIOE
// #define SW2_Pin GPIO_PIN_13
// #define SW2_GPIO_Port GPIOC
// #define SW_Pin GPIO_PIN_2
// #define SW_GPIO_Port GPIOA
// #define CONNECT_Pin GPIO_PIN_1
// #define CONNECT_GPIO_Port GPIOC
// #define MUSIC_ON_Pin GPIO_PIN_1
// #define MUSIC_ON_GPIO_Port GPIOE
static uint8_t Key_Read(uint8_t button_id)
{
    switch(button_id)
    {
        case 0: return HAL_GPIO_ReadPin(SW_GPIO_Port, SW_Pin);
        case 1: return HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin);
        case 2: return HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin);
        case 3: return HAL_GPIO_ReadPin(CONNECT_GPIO_Port, CONNECT_Pin);
        case 4: return HAL_GPIO_ReadPin(MUSIC_ON_GPIO_Port, MUSIC_ON_Pin);
        default: return 1;  // 默认未按下
    }
}

/* ================= 按键事件回调 ================= */



void Button_Callback(Button *btn)
{
    ButtonCommand_t cmd;
    // cmd.btn = btn;
     cmd.id = btn->button_id;    
    cmd.event = button_get_event(btn);

    xQueueSend(buttonQueue, &cmd, 0); // 任务上下文调用，直接发送
        // log_printf("Button %d, event %d", cmd.btn, cmd.event);
}

/* ================= 按键初始化 ================= */
void Key_Init(void)
{
    /* 初始化按键对象（注意 4 个参数：handle, GPIO读取函数, 有效电平, 按键ID） */
    button_init(&key_left, Key_Read, 0, 0);
    button_attach(&key_left, BTN_SINGLE_CLICK, Button_Callback);
    button_attach(&key_left, BTN_DOUBLE_CLICK, Button_Callback);
    button_attach(&key_left, BTN_LONG_PRESS_START, Button_Callback);
    button_start(&key_left);

    button_init(&key_middle, Key_Read, 0, 1);
    button_attach(&key_middle, BTN_SINGLE_CLICK, Button_Callback);
    button_attach(&key_middle, BTN_DOUBLE_CLICK, Button_Callback);
    button_attach(&key_middle, BTN_LONG_PRESS_START, Button_Callback);
    button_start(&key_middle);

    button_init(&key_right, Key_Read, 0, 2);
    button_attach(&key_right, BTN_SINGLE_CLICK, Button_Callback);       
    button_attach(&key_right, BTN_DOUBLE_CLICK, Button_Callback);
    button_attach(&key_right, BTN_LONG_PRESS_START, Button_Callback);
    button_start(&key_right);

    button_init(&VIRTUAL_BLE_STATE, Key_Read, 0, 3);
    button_attach(&VIRTUAL_BLE_STATE, BTN_PRESS_UP, Button_Callback);
    button_attach(&VIRTUAL_BLE_STATE, BTN_PRESS_DOWN, Button_Callback);
    button_start(&VIRTUAL_BLE_STATE);

    button_init(&VIRTUAL_AUDIO_STATE, Key_Read, 0, 4);
    button_attach(&VIRTUAL_AUDIO_STATE, BTN_PRESS_UP, Button_Callback);
    button_attach(&VIRTUAL_AUDIO_STATE, BTN_PRESS_DOWN, Button_Callback);
    button_start(&VIRTUAL_AUDIO_STATE);

}

/* ================= 按键扫描 ================= */
void Key_Scan(void)
{
    button_ticks();
}
