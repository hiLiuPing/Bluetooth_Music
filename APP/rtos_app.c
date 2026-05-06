#include "rtos_app.h"
#include "log.h"
#include "key_app.h"
#include "music_app.h"
// #include "lis3dh_app.h"
#include "defin_common.h"
#include "multi_led.h"
#include "main.h"
#include "tim.h"
#include "SystemMonitor_app.h"
#include "oled_ui.h"
#include "aht20.h"
// #include "tftlcd.h"  
// #include "pic.h"
// #include "ws2812.h"
// #include "buzzer.h"
// #include "eeprom_app.h"
// #include "string.h"

// // #include "sfud.h"
// // #include "spi.h" // 如果 spi 句柄定义在这里
// #include "spi_flash.h"
// // #include "gc9d01.h"
// #include "lfs_port.h"
// 按键扫描任务
void vKey_Scan_Task(void *pvParameters);
#define Key_Scan_Task_PRIORITY   (tskIDLE_PRIORITY + 2)
#define Key_Scan_Task_STACK_SIZE 128
#define Key_Scan_Task_PERIOD_MS  10
TaskHandle_t Key_Scan_Task_Handle;

// 按键分配任务
void vKey_Manllege_Task(void *pvParameters);
#define Key_Manllegr_Task_PRIORITY   (tskIDLE_PRIORITY + 2)
#define Key_Manllegrn_Task_STACK_SIZE 128
#define Key_Manllegr_Task_PERIOD_MS  10
TaskHandle_t Key_Manllegr_Task_Handle;


// 音乐播放任务
void vMusic_Task(void *pvParameters);
#define Music_Task_PRIORITY   (tskIDLE_PRIORITY + 2)
#define Music_Task_STACK_SIZE 128
#define Music_Task_PERIOD_MS  10
TaskHandle_t Music_Task_Handle;

// LED任务
void vLED_Task(void *pvParameters);
#define LED_Task_PRIORITY   (tskIDLE_PRIORITY + 2)
#define LED_Task_STACK_SIZE 128
#define LED_Task_PERIOD_MS  10
TaskHandle_t LED_Task_Handle;
    LED_Object_t led_blue;  // PE3 -> TIM3_CH1
    LED_Object_t led_green; // PE4 -> TIM3_CH2
    LED_Object_t led_red;   // PE5 -> TIM3_CH3
void LED_Update(void);


// //OLED任务队列
extern I2C_HandleTypeDef hi2c2;
void vOLED_Task(void *pvParameters);     
#define OLED_Task_PRIORITY   (tskIDLE_PRIORITY + 2)
#define OLED_Task_STACK_SIZE 512
#define OLED_Task_PERIOD_MS  20
TaskHandle_t oled_Task_Handle;



// //I2C传感器任务队列


// void vI2C_Sensor_Task(void *pvParameters);
// #define I2C_SENSOR_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)
// #define I2C_SENSOR_TASK_STACK_SIZE 256
// #define I2C_SENSOR_TASK_PERIOD_MS  1000
// TaskHandle_t i2c_sensor_Task_Handle;

// // ADC传感器任务
// void vADC_Sensor_Task(void *pvParameters);
// #define SENSOR_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)
// #define SENSOR_TASK_STACK_SIZE 256
// #define SENSOR_TASK_PERIOD_MS  500
// TaskHandle_t sensor_Task_Handle;

// // TFT显示任务
// void vTFT_Display_Task(void *pvParameters);
// #define TFT_Display_Task_PRIORITY   (tskIDLE_PRIORITY + 1)
// #define TFT_Display_Task_STACK_SIZE 1024*2
// #define TFT_Display_Task_PERIOD_MS  50
// TaskHandle_t tft_display_Task_Handle;

// void vLED_ws2812_Task(void *pvParameters);
// #define WS2812_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)       
// #define WS2812_TASK_STACK_SIZE 256
// #define WS2812_TASK_PERIOD_MS  50
// TaskHandle_t ws2812_task_Handle;

// void vEeprom_Task(void *pvParameters);
// #define EEPROM_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)       
// #define EEPROM_TASK_STACK_SIZE 256
// #define EEPROM_TASK_PERIOD_MS  1000
// TaskHandle_t eeprom_task_Handle;
// // spi flash 任务
// void vFlash_Task(void *pvParameters);
// #define FLASH_TASK_PRIORITY   (tskIDLE_PRIORITY + 2)       
// #define FLASH_TASK_STACK_SIZE 1024*2
// #define FLASH_TASK_PERIOD_MS  1000
// TaskHandle_t flash_task_Handle;

void vKey_Scan_Task(void *pvParameters)
{
    Key_Init();
    // vTaskDelay(3000 / portTICK_PERIOD_MS); // 等待系统稳定
    TickType_t key_scan_tick_count = xTaskGetTickCount();
   buttonQueue = xQueueCreate(16, sizeof(ButtonCommand_t));
    while (1) {

        Key_Scan();

        //  log_printf("Hello, this is a test log message.");
        vTaskDelayUntil(&key_scan_tick_count, Key_Scan_Task_PERIOD_MS);
    }
}

void vKey_Manllege_Task(void *pvParameters)
{
    ButtonCommand_t keycmd;
    BaseType_t xStatus;
    music_app_init();
    // MusicCmd_t xMusicCmd;    // 音乐指令缓存
    // DisplayCmd_t xDisplayCmd;// 显示指令缓存

    while (1) {
        // 阻塞接收按键事件
        xStatus = xQueueReceive(buttonQueue, &keycmd, portMAX_DELAY);
        
        if(xStatus == pdPASS)
        {
            // 仅做指令转发，不包含任何业务逻辑
            //  这里重置各种定时器。
                    switch(keycmd.id)
                    {
case 3:
    if(keycmd.event == BTN_PRESS_UP)
    {
        g_ble_state.ble_connected = 1;
        g_ble_state.ble_ever_connected = 1;
        Bluetooth_Connected();
        // OLED_UI_OnEvent("BLE_CONNECTED");
    }
    else if(keycmd.event == BTN_PRESS_DOWN)
    {
        g_ble_state.ble_connected = 0;

        if(g_ble_state.ble_ever_connected)
        {
            Bluetooth_Disconnected();
                                    //        music_send_cmd(CMD_PLAY_STOP);
                                    // OLED_UI_OnEvent(UI_EVT_PAUSE);
                                    // // g_music_state.music_played = 0;

                                    // OLED_UI_SetPage(UI_PAGE_STOP);     
        }
    }

    LED_Update();   // ⭐统一刷新
    break;

case 4:
    if(keycmd.event == BTN_PRESS_UP)
    {
        g_music_state.music_played = 1;
        g_music_state.music_ever_played = 1;
        Music_Playing();
                                    //       music_send_cmd(CMD_PLAY_STOP);
                                    // OLED_UI_OnEvent(UI_EVT_PLAY);
                                    // // g_music_state.music_played = 1;
                                    // OLED_UI_SetPage(UI_PAGE_PLAY);  
        // OLED_UI_OnEvent("PLAY");
    }
    else if(keycmd.event == BTN_PRESS_DOWN)
    {
        g_music_state.music_played = 0;

        if(g_music_state.music_ever_played)
        {
            Music_Pause();
            // OLED_UI_OnEvent("PAUSE");
                                    // music_send_cmd(CMD_PLAY_STOP);
                                    // OLED_UI_OnEvent(UI_EVT_PAUSE);
                                    // // g_music_state.music_played = 0;

                                    // OLED_UI_SetPage(UI_PAGE_STOP);
                                    //       music_send_cmd(CMD_PLAY_STOP);
                                    // OLED_UI_OnEvent(UI_EVT_PLAY);
                                    // // g_music_state.music_played = 1;
                                    // OLED_UI_SetPage(UI_PAGE_PLAY);  

        }
    }

    LED_Update();   // ⭐统一刷新
    break;

                        default:
                            break;
                    }

// 每次按键进来都要重置定时休眠定时器
                     Key_Event();

            switch(g_app_current_state)
            {
                // ========== 音乐状态：转发到音乐队列 ==========
                case MUSIC_ON:
                    switch(keycmd.id)
                    {
                        case 0: // OK键
                            if(keycmd.event == BTN_SINGLE_CLICK) // 短按
                            {
                                // xMusicCmd = MUSIC_CMD_PLAY_PAUSE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0); // 非阻塞发送
                                   music_send_cmd(CMD_PREV);
                                    //    CMD_PLAY_STOP,  // 播放/暂停   
    // CMD_PREV,       // 上一曲
    // CMD_NEXT,       // 下一曲
    // CMD_PAIR,       // 配对
    // CMD_CLEAR_PAIR, // 清除配对记录
    // CMD_POWER_ON,   // 开机
    // CMD_POWER_OFF,  // 关机
    // CMD_VOL_UP,     // 音量+
    // CMD_VOL_DOWN    // 音量-
                          LED_Driver_SendCmd(&led_blue, LED_MODE_PWM, LED_Heartbeat_Handler, 2000, 2000, NULL);
                                  log_printf("CMD_PREV Button %d, event %d", keycmd.id, keycmd.event); 
                                  OLED_UI_OnEvent(UI_EVT_PREV);
                                 
 
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK) // 双击
                            {
                                // xMusicCmd = MUSIC_CMD_SWITCH_MODE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                  music_send_cmd(CMD_VOL_DOWN);
                                  LED_Driver_SendCmd(&led_blue, LED_MODE_PWM, LED_Heartbeat_Handler, 2000, 2000, NULL);
                                   OLED_UI_OnEvent(UI_EVT_VOL_DOWN);
                                log_printf("CMD_VOL_DOWN Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START) // 长按
                            {
                                // xMusicCmd = MUSIC_CMD_NEXT_TRACK;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                // Buzzer_Beep_Long();
                                log_printf("MUSIC_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            break;

                        case 1: // 上键短按
                            if(keycmd.event == BTN_SINGLE_CLICK) // 短按
                            {
                                // Buzzer_Beep_Short();
                                // xMusicCmd = MUSIC_CMD_PLAY_PAUSE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0); // 非阻塞发送

                                 if (g_music_state.music_played)
                                 {
                                    /* code */
                                    music_send_cmd(CMD_PLAY_STOP);
                                    // OLED_UI_OnEvent(UI_EVT_PAUSE);
                                     OLED_UI_SetPage(UI_PAGE_STOP);
                                    g_music_state.music_played = 0;

                                    OLED_UI_SetPage(UI_PAGE_STOP);
                                 }
                                 else
                                 {
                                    /* code */
                                    music_send_cmd(CMD_PLAY_STOP);
                                    // OLED_UI_OnEvent(UI_EVT_PLAY);
                                    g_music_state.music_played = 1;
                                    OLED_UI_SetPage(UI_PAGE_PLAY);
                                 }

                                
                                   LED_Driver_SendCmd(&led_blue, LED_MODE_PWM, LED_Heartbeat_Handler, 2000, 2000, NULL);
                                  log_printf("CMD_PLAY_STOP Button %d, event %d", keycmd.id, keycmd.event); 
                                //   OLED_UI_OnEvent(g_ui.event = UI_EVT_PAUSE);
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK) // 双击
                            {
                                // xMusicCmd = MUSIC_CMD_SWITCH_MODE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                  OLED_UI_OnEvent(UI_EVT_BATTERY_CHARGING);
                                log_printf("UI_EVT_BATTERY_CHARGING Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START) // 长按
                            {
                                // xMusicCmd = MUSIC_CMD_NEXT_TRACK;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                // Buzzer_Off();
                             
                                log_printf("UI_EVT_BATTERY_CHARGING Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            break;

                        case 2: // 上键短按
                            if(keycmd.event == BTN_SINGLE_CLICK) // 短按
                            {
                                // xMusicCmd = MUSIC_CMD_PLAY_PAUSE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0); // 非阻塞发送
                                 music_send_cmd(CMD_NEXT);
                                 LED_Driver_SendCmd(&led_blue, LED_MODE_PWM, LED_Heartbeat_Handler, 2000, 2000, NULL);
                                  log_printf("CMD_NEXT Button %d, event %d", keycmd.id, keycmd.event); 
                                  OLED_UI_OnEvent(UI_EVT_NEXT);
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK) // 双击
                            {
                                // xMusicCmd = MUSIC_CMD_SWITCH_MODE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                                        music_send_cmd(CMD_VOL_UP);
                                    LED_Driver_SendCmd(&led_blue, LED_MODE_PWM, LED_Heartbeat_Handler, 2000, 2000, NULL);
                                    OLED_UI_OnEvent(UI_EVT_VOL_UP);
                                log_printf("CMD_VOL_UP Button %d, event %d", keycmd.id, keycmd.event);         
                                
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START) // 长按
                            {
                                // xMusicCmd = MUSIC_CMD_NEXT_TRACK;
                                log_printf("MUSIC_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            break;

                        default:
                            break;
                    }
                    break;

                // ========== 显示状态：转发到显示队列 ==========
                case DISPLAY_ON:
                    switch(keycmd.id)
                    {
                        case 0: // OK键
                            if(keycmd.event == BTN_SINGLE_CLICK) // 短按
                            {
                                // xMusicCmd = MUSIC_CMD_PLAY_PAUSE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0); // 非阻塞发送
                             
                                  log_printf("DISPLAY_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK) // 长按
                            {
                                // xMusicCmd = MUSIC_CMD_SWITCH_MODE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                log_printf("DISPLAY_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START) // 双击
                            {
                                // xMusicCmd = MUSIC_CMD_NEXT_TRACK;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                log_printf("DISPLAY_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            break;

                        case 1: // 上键短按
                            if(keycmd.event == BTN_SINGLE_CLICK) // 短按
                            {
                                // xMusicCmd = MUSIC_CMD_PLAY_PAUSE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0); // 非阻塞发送
                                  log_printf("DISPLAY_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK) // 长按
                            {
                                // xMusicCmd = MUSIC_CMD_SWITCH_MODE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                log_printf("DISPLAY_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START) // 双击
                            {
                                // xMusicCmd = MUSIC_CMD_NEXT_TRACK;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                log_printf("DISPLAY_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            break;

                        case 2: // 上键短按
                            if(keycmd.event == BTN_SINGLE_CLICK) // 短按
                            {
                                // xMusicCmd = MUSIC_CMD_PLAY_PAUSE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0); // 非阻塞发送
                                  log_printf("DISPLAY_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK) // 长按
                            {
                                // xMusicCmd = MUSIC_CMD_SWITCH_MODE;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                                log_printf("DISPLAY_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START) // 双击
                            {
                                // xMusicCmd = MUSIC_CMD_NEXT_TRACK;
                                // xQueueSend(xMusicQueue, &xMusicCmd, 0);
                               
                                log_printf("DISPLAY_ON Button %d, event %d", keycmd.id, keycmd.event); 
                            }
                            break;

                        default:
                            break;
                    }
                    break;

                default:
                    g_app_current_state = MUSIC_ON; // 异常状态切回默认
                    break;
            }
        }
    }
}



void myTask()
{
//     Buzzer_Init();
//    I2C_Bus_Init();  // 初始化互斥锁
UserMonitor_Init(); // 初始化监控系统和定时器

    // 创建按键扫描任务
    
    BaseType_t ret;
    buttonQueue = xQueueCreate(16, sizeof(ButtonCommand_t));
    ret         = xTaskCreate((TaskFunction_t)vKey_Scan_Task,
                              "Key_Scan_Task",
                              Key_Scan_Task_STACK_SIZE,
                              NULL,
                              Key_Scan_Task_PRIORITY,
                              &Key_Scan_Task_Handle);
    if (ret != pdPASS) {
        log_printf("key task create FAIL\r\n");
    }

// 创建按键分配任务
    ret = xTaskCreate((TaskFunction_t)vKey_Manllege_Task,
                       "Key_Manllegr_Task",
                       Key_Manllegrn_Task_STACK_SIZE,
                       NULL,
                       Key_Manllegr_Task_PRIORITY,
                       &Key_Manllegr_Task_Handle);
    if (ret != pdPASS) {
        log_printf("key Manllegr task create FAIL\r\n");
    }

// 创建音乐播放任务
    ret = xTaskCreate((TaskFunction_t)vMusic_Task,
                       "Music_Task",
                       Music_Task_STACK_SIZE,
                       NULL,
                       Music_Task_PRIORITY,
                       &Music_Task_Handle);
    if (ret != pdPASS) {
        log_printf("music task create FAIL\r\n");
    }
// 创建LED任务
    ret = xTaskCreate((TaskFunction_t)vLED_Task,
                       "LED_Task",
                       LED_Task_STACK_SIZE,
                       NULL,
                       LED_Task_PRIORITY,
                       &LED_Task_Handle);
    if (ret != pdPASS) {
        log_printf("LED task create FAIL\r\n");
    }

    // 创建OLED任务
    ret = xTaskCreate((TaskFunction_t)vOLED_Task,
                      "My_OLED_Task",
                      OLED_Task_STACK_SIZE,
                      NULL,
                      OLED_Task_PRIORITY,
                      &oled_Task_Handle);
    if (ret != pdPASS) {
        log_printf("OLED task create FAIL\r\n");
    }

//     // 创建I2C传感器任务
//     ret = xTaskCreate((TaskFunction_t)vI2C_Sensor_Task,
//                       "My_I2C_Sensor_Task",
//                       I2C_SENSOR_TASK_STACK_SIZE,
//                       NULL,
//                       I2C_SENSOR_TASK_PRIORITY,
//                       &i2c_sensor_Task_Handle);
//     if (ret != pdPASS) {
//         log_printf("I2C Sensor task create FAIL\r\n");
//     }
 
//     // 创建传感器任务
//     ret = xTaskCreate((TaskFunction_t)vADC_Sensor_Task,
//                       "My_Sensor_Task",
//                       SENSOR_TASK_STACK_SIZE,
//                       NULL,
//                       SENSOR_TASK_PRIORITY,
//                       &sensor_Task_Handle);
//     if (ret != pdPASS) {
//         log_printf("Sensor task create FAIL\r\n");
//     }

//     // 创建传感器任务
//     ret = xTaskCreate((TaskFunction_t)vTFT_Display_Task,
//                       "My_TFT_Display_Task",
//                       TFT_Display_Task_STACK_SIZE,
//                       NULL,
//                       TFT_Display_Task_PRIORITY,
//                       &tft_display_Task_Handle);
//     if (ret != pdPASS) {
//         log_printf("TFT Display task create FAIL\r\n");
//     }

//     // 创建EEPROM任务
//     ret = xTaskCreate((TaskFunction_t)vEeprom_Task,
//                       "My_Eeprom_Task",
//                       EEPROM_TASK_STACK_SIZE,
//                       NULL,
//                       EEPROM_TASK_PRIORITY,
//                       &eeprom_task_Handle);
//     if (ret != pdPASS) {
//         log_printf("EEPROM task create FAIL\r\n");
//     }

//     // 创建FLASH任务
//     ret = xTaskCreate((TaskFunction_t)vFlash_Task,
//                       "My_Flash_Task",
//                       FLASH_TASK_STACK_SIZE,
//                       NULL,
//                       FLASH_TASK_PRIORITY,
//                       &flash_task_Handle);
//     if (ret != pdPASS) {
//         log_printf("FLASH task create FAIL\r\n");
//     }




//     // 创建WS2812任务
//     // ret = xTaskCreate((TaskFunction_t)vLED_ws2812_Task,
//     //                   "My_ws2812_Task",
//     //                   WS2812_TASK_STACK_SIZE,
//     //                   NULL,
//     //                   WS2812_TASK_PRIORITY, &ws2812_task_Handle);
//     // if (ret != pdPASS) {
//     //     log_printf("WS2812 task create FAIL\r\n");
//     // }
//   // log_printf("StartTask!\n");

}


void vMusic_Task(void *pvParameters)
{
    MusicCtrlCmd cmd;
    Music_Ctrl_Init(); // 初始化状态和按键
    // 调试的时候关机
    // Music_PowerOn();    // 开机  
     vTaskDelay(pdMS_TO_TICKS(500)); // 等待设备稳定
    while (1)
    {
        if (xQueueReceive(music_cmd_queue, &cmd, portMAX_DELAY) == pdPASS)
        {
            switch (cmd)
            {
                case CMD_PLAY_STOP:   Music_Play_Stop();     break;
                case CMD_PREV:        Music_Up();     break;
                case CMD_NEXT:        Music_Next();         break;
                case CMD_PAIR:        Music_Pair();         break;
                case CMD_CLEAR_PAIR:  Music_ClearPair();    break;
                case CMD_POWER_ON:    Music_PowerOn();      break;
                case CMD_POWER_OFF:   Music_PowerOff();     break;
                case CMD_VOL_UP:      Music_VolumeUp();     break;
                case CMD_VOL_DOWN:    Music_VolumeDown();   break;
                default: break;
            }
        }
    }
}


void vOLED_Task(void *arg)
{
    // vTaskDelay(pdMS_TO_TICKS(500));
    // 初始化 UI 框架
    OLED_UI_Init(&hi2c2);
    
OLED_UI_SetBattery(60, 1);

    while (1)
    {
        // 核心渲染逻辑
         OLED_UI_Update();
        OLED_UI_Render();

        vTaskDelay(pdMS_TO_TICKS(16)); // 约 60FPS
    }
}

void vLED_Task(void *pvParameters)
{


       LED_Driver_System_Init();
// #define LED_B_Pin GPIO_PIN_3
// #define LED_B_GPIO_Port GPIOE
// #define LED_G_Pin GPIO_PIN_4
// #define LED_G_GPIO_Port GPIOE
// #define LED_R_Pin GPIO_PIN_5
// #define LED_R_GPIO_Port GPIOE

    LED_Driver_Init(&led_green, LED_G_GPIO_Port, LED_G_Pin, &htim3, TIM_CHANNEL_2, 1);
    LED_Driver_Init(&led_blue, LED_B_GPIO_Port, LED_B_Pin, &htim3, TIM_CHANNEL_1, 1);
    LED_Driver_Init(&led_red, LED_R_GPIO_Port, LED_R_Pin, &htim3, TIM_CHANNEL_3, 1);
   LED_Driver_SendCmd(&led_blue, LED_MODE_PWM, LED_Off_Handler, 0, 0, NULL);
     LED_Driver_SendCmd(&led_green, LED_MODE_PWM, LED_Off_Handler, 0, 0, NULL);
   LED_Driver_SendCmd(&led_red, LED_MODE_PWM, LED_Heartbeat_Handler, 2000, 0, NULL);      
       TickType_t led_scan_tick_count = xTaskGetTickCount();
    while (1) {

        LED_Driver_Update(); 
        //  log_printf("Hello, this is a test log message.");
        vTaskDelayUntil(&led_scan_tick_count, LED_Task_PERIOD_MS);
    }
}







// void vI2C_Sensor_Task(void *pvParameters)
// {
    
//     TickType_t i2c_sensor_tick_count = xTaskGetTickCount();

//     // 任务启动时先检测传感器是否就绪
//     if(SHT40_IsReady() == HAL_OK)
//     {
//         log_printf("SHT40 is ready.\n");
//     }
//     else
//     {
//         log_printf("SHT40 is not ready.\n");  
//     }
//     vTaskDelay(pdMS_TO_TICKS(100)); // 等待传感器稳定
//     while (1)
//     {
//         // 读取温湿度（直接传全局data地址）
//         if(SHT40_Read((SHT40_Data_t*)&data) == HAL_OK)
//         {
//             // 可选：打印日志验证数据更新
//             // log_printf("T=%.1f H=%.1f\n", data.temperature, data.humidity);
//         }
//         else
//         {
//             log_printf("SHT40 read failed!\n");
//         }

//         // 固定周期读取（1秒/次，避免频繁读取）
//         vTaskDelayUntil(&i2c_sensor_tick_count, pdMS_TO_TICKS(I2C_SENSOR_TASK_PERIOD_MS));
//     }
// }


// void vADC_Sensor_Task(void *pvParameters)
// {

//               //   先校准adc
//     HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
//     TickType_t sensor_tick_count = xTaskGetTickCount();
//     while (1)
//     {

//         // 启动 ADC 单次转换
//         HAL_ADC_Start(&hadc1);

//         // 等待转换完成
//         if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)  // 最多等待 10ms
//         {
//             uint16_t bat_adc = HAL_ADC_GetValue(&hadc1);
//             // float bat_voltage = 3.28f * bat_adc * 2.0f / 4095.0f;
//        float bat_voltage =  Calc_BatteryVoltage(bat_adc);
//             // 滤波处理
//             Battery.Voltage = IIR_Filter(Battery.Voltage, bat_voltage, BAT_IIR_ALPHA);
//             Battery.Percent = Calc_BatteryPercent(Battery.Voltage);

//             // log_printf("ADC:%d | Voltage:%.2fV | Percent:%.0f%%",
//             //            bat_adc, Battery.Voltage, Battery.Percent);
//         }
//         vTaskDelayUntil(&sensor_tick_count, SENSOR_TASK_PERIOD_MS);
//     }
// }


// void vTFT_Display_Task(void *pvParameters) {


// TFTLCD_Init();
    
// TFTLCD_Fill(0,0,TFTLCD_W,TFTLCD_H,BLACK);
// // TFTLCD_Fill(0,0,TFTLCD_W,TFTLCD_H,YELLOW);
//  TickType_t tft_tick_count = xTaskGetTickCount();
//     while(1) {


//         // 合并临界区：一次性读取所有共享数据，减少开销
//         float bat_volt, bat_percent, temp, humi;
//         taskENTER_CRITICAL();
//         // 读取电池数据
//         bat_volt = Battery.Voltage;
//         bat_percent = Battery.Percent;
//         // 读取温湿度数据
//         temp = data.temperature;
//         humi = data.humidity;
//         taskEXIT_CRITICAL();
//         // 显示所有数据（用局部变量，避免跨任务访问）

//  TFTLCD_ShowFloatNum1(5,10,bat_percent,0,GREEN,BLACK,16);  // 电池百分比（0位小数）
//         // TFTLCD_ShowString(110, 10, "ABCabc", GREEN, BLACK, 16, 0);
//         // TFTLCD_DrawRectangle(110, 0, 5, 5, GREEN);
//         // TFTLCD_DrawLine(50, 5, 110, 5, GREEN);
//     //    TFTLCD_ShowFloatNum1(110,10,88.88,2,GREEN,BLACK,16);
//         TFTLCD_ShowString(30, 10, "%", GREEN, BLACK, 16, 0);
//         TFTLCD_ShowFloatNum1(5,30,bat_volt,2,RED,BLACK,16);       // 电池电压（2位小数）
//         TFTLCD_ShowFloatNum1(60,30,temp,1,RED,BLACK,16);          // 温度（1位小数）
//         TFTLCD_ShowFloatNum1(110,30,humi,1,RED,BLACK,16);         // 湿度（1位小数） 
//          vTaskDelayUntil(&tft_tick_count, TFT_Display_Task_PERIOD_MS);

 


//     }
// }


// void vLED_ws2812_Task(void *pvParameters)
// {


//     hws.htim = &htim15;
//     hws.GPIOx = GPIOB;
//     hws.GPIO_Pin = GPIO_PIN_15;
//     hws.LED_Num = 2;
//     hws.WS_0_Cnt = 28;        // 根据 TIM15 时钟计算
//     hws.WS_1_Cnt = 56;
//     hws.WS_Reset_Cnt = 4000; // 50us

//     WS2812_Init(&hws);
//     WS2812_Clear(&hws);

//          // 打开 LED
//     WS2812_SetBrightnessPercent(5); // 默认亮度 50%


//    TickType_t ws2812_tick_count = xTaskGetTickCount();
//     // uint8_t frame = 0;

//     while(1)
//     {
//       WS2812_SetBrightnessPercent(50); // 0~100
// WS2812_SetLED(&hws,0,255,0,0);
// WS2812_SetLED(&hws,1,0,255,0);
// WS2812_Refresh(&hws);
// vTaskDelay(1000);
// WS2812_Off(&hws);
// vTaskDelay(1000);
// WS2812_On(&hws); 
// vTaskDelay(1000);
//         // vTaskDelayUntil(&ws2812_tick_count,pdMS_TO_TICKS(WS2812_TASK_PERIOD_MS));
//     }
    
    
// }

// void vEeprom_Task(void *argument)

// {

//     // NetConfig_t live_net_cfg; // 内存中的数据副本
//     // CalibData_t live_calib_cfg;
// // 1. 底层初始化
//     EE24_Init(&ee_chip, &hi2c2, 0xA0);
    
//     // 2. 配置自检与修复
//     AppConfig_System_Check();

//     TickType_t eeprom_tick_count = xTaskGetTickCount();
    
//     for(;;)
//     {
// // 3. 循环读取：仅读取 A 区数据到内存副本
//         // if (AppConfig_Load(0, OFF_NET_CONFIG, &live_net_cfg, sizeof(NetConfig_t))) 
//         // {
//         //     // 读取成功，可以在这里把数据同步给其他全局变量，或者打印出来观察
//         //     log_printf("Live SSID: %s", live_net_cfg.ssid);
//         // }
//         //   if (AppConfig_Load(0, OFF_CALIB_DATA, &live_calib_cfg, sizeof(CalibData_t))) 
//         // {
//         //     // 读取成功，可以在这里把数据同步给其他全局变量，或者打印出来观察
//         //     log_printf("Live Calib Offset: %d", live_calib_cfg.calibration_val);
//         // }      
//         // else 
//         // {
//         //     // 如果运行中突然读取失败，可能是硬件抖动
//         //     log_printf("EEPROM: Runtime read error!\r\n");
//         // }
//         // AppConfig_System_Check();
//         // 建议改为 5000ms (5秒)，1秒有点快
//         vTaskDelayUntil(&eeprom_tick_count, pdMS_TO_TICKS(5000)); 
//     }
// }





// // void vFlash_Task(void *argument)
// // {

// // if (W25Q_Init() != 0) {
// //     log_printf("W25Q256 Init Failed! Suspend Task.\n");
// //     vTaskSuspend(NULL); // 初始化失败就挂起任务，不要继续操作
// // }
// //     // 1. 准备 100 字节的测试数据
// //     uint8_t tx_data[100];
// //     uint8_t rx_data[100];
// //     for (int i = 0; i < 100; i++) {
// //         tx_data[i] = i + 0xA0; // 填充一些容易分辨的数据
// //     }

// //     // 2. 选择一个跨越扇区边界的地址 (4KB = 0x1000)
// //     // 0x000FFFFA 位于第一个扇区的末尾，写入 100 字节会跨入第二个扇区
// //     uint32_t test_addr = 0x000FFFFA; 

// //     log_printf("\r\n--- W25Q256 Reliability Test ---\r\n");
// //     log_printf("Target Addr: 0x%08X\r\n", test_addr);

// //     // 3. 擦除受影响的两个扇区 (Flash 写入前必须手动擦除)
// //     log_printf("Erasing sectors...\r\n");
// //     W25Q_EraseSector(test_addr);          // 擦除第一个扇区 (0x0000F000 - 0x0000FFFF)
// //     W25Q_EraseSector(test_addr + 100);    // 擦除第二个扇区 (0x00010000 - 0x00010FFF)

// //     // 4. 写入 100 字节 (W25Q_WriteBuffer 会自动处理页对齐)
// //     log_printf("Writing 100 bytes...\r\n");
// //     // W25Q_WriteBuffer(test_addr, tx_data, 100);

// //     TickType_t flash_tick_count = xTaskGetTickCount();
    
// //     for(;;)
// //     {
// //     // 5. 读回数据
// //     log_printf("Reading back...\r\n");
// //     memset(rx_data, 0, 100);
// //     W25Q_Read(test_addr, rx_data, 100);

// //     // 6. 验证结果
// //     uint8_t error_count = 0;
// //     for (int i = 0; i < 100; i++) {
// //         if (rx_data[i] != tx_data[i]) {
// //             error_count++;
// //             log_printf("Error at [%d]: Exp 0x%02X, Got 0x%02X\r\n", i, tx_data[i], rx_data[i]);
// //         }
// //     }

// //     if (error_count == 0) {
// //         log_printf(">> SUCCESS: Data matches perfectly!\r\n");
// //     } else {
// //         log_printf(">> FAILED: %d bytes mismatch.\r\n", error_count);
// //     }
// //     log_printf("--------------------------------\r\n");
// //         vTaskDelayUntil(&flash_tick_count, pdMS_TO_TICKS(FLASH_TASK_PERIOD_MS)); 
// //     }
// // }

// void vFlash_Task(void *argument)
// {

//     if(littlefs_init()==0)
//     {
//         log_printf("LittleFS Mount OK\r\n");
   
//     }
//     else
//     {
//         log_printf("LittleFS Mount Fail\r\n");
//         vTaskSuspend(NULL); // 初始化失败就挂起任务，不要继续操作
//     }
    
//     TickType_t flash_tick_count = xTaskGetTickCount();
//       for(;;)
//     {
//     // log_printf("Flash Init OK\r\n");

//     // log_printf("Manufacturer ID : 0x%02X\r\n", w25q_info.ID[0]);
//     // log_printf("Memory Type     : 0x%02X\r\n", w25q_info.ID[1]);
//     // log_printf("Capacity Code   : 0x%02X\r\n", w25q_info.ID[2]);

//     // log_printf("Capacity        : %lu Bytes\r\n", w25q_info.Capacity);
//     // log_printf("Capacity        : %lu MB\r\n", w25q_info.Capacity / 1024 / 1024);

//     // if(w25q_info.AddrMode)
//     //     log_printf("Address Mode    : 4-Byte\r\n");
//     // else
//     //     log_printf("Address Mode    : 3-Byte\r\n");

//    log_printf("LittleFS Mount OK");

//        vTaskDelayUntil(&flash_tick_count, pdMS_TO_TICKS(FLASH_TASK_PERIOD_MS));  
//     }
// }


static void LED_Update(void)
{
    /* =========================
     * 1. BLE未连接（最高优先级）
     * ========================= */
    if(g_ble_state.ble_connected == 0)
    {
        LED_Driver_SendCmd(&led_green,
                           LED_MODE_PWM,
                           LED_Off_Handler,
                           0, 0, NULL);

        LED_Driver_SendCmd(&led_red,
                           LED_MODE_PWM,
                           LED_Blink_Handler,
                           1000, 0, NULL);

                                    //  music_send_cmd(CMD_PLAY_STOP);
                                    OLED_UI_OnEvent(UI_EVT_PAUSE);
                                    // // g_music_state.music_played = 0;

                                    // OLED_UI_SetPage(UI_PAGE_STOP);
                                    //       music_send_cmd(CMD_PLAY_STOP);
                                    // OLED_UI_OnEvent(UI_EVT_PLAY);
                                    // // g_music_state.music_played = 1;
                                    // OLED_UI_SetPage(UI_PAGE_PLAY);        
        return;
    }

    /* =========================
     * 2. BLE已连接
     * ========================= */
    LED_Driver_SendCmd(&led_red,
                       LED_MODE_PWM,
                       LED_Off_Handler,
                       0, 0, NULL);

    /* =========================
     * 3. MUSIC状态决定绿灯
     * ========================= */
    if(g_music_state.music_played == 0)
    {
        /* 未播放 → 闪绿灯 */
        LED_Driver_SendCmd(&led_green,
                           LED_MODE_PWM,
                           LED_Blink_Handler,
                           1000, 0, NULL);

                                        OLED_UI_SetPage(UI_PAGE_STOP);
                                        //   music_send_cmd(CMD_PLAY_STOP);
                                    // OLED_UI_OnEvent(UI_EVT_PLAY);
                                    // // g_music_state.music_played = 1;
                                    // OLED_UI_SetPage(UI_PAGE_PLAY);                         
    }
    else
    {
        /* 播放中 → 呼吸灯 */
        LED_Driver_SendCmd(&led_green,
                           LED_MODE_PWM,
                           LED_Breath_Handler,
                           5000, 0, NULL);
                                        //   OLED_UI_OnEvent(UI_EVT_PLAY);
                                    // // g_music_state.music_played = 1;
                                    OLED_UI_SetPage(UI_PAGE_PLAY);                      
    }
}