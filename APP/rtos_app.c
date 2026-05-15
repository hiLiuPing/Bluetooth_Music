#include "rtos_app.h"
#include "key_app.h"
#include "log.h"
#include "music_app.h"
// #include "lis3dh_app.h"
#include "SystemMonitor_app.h"
#include "aht20.h"
#include "defin_common.h"
#include "main.h"
#include "multi_led.h"
#include "oled_ui.h"
#include "tim.h"
// #include "tftlcd.h"
// #include "pic.h"
// #include "ws2812.h"
// #include "buzzer.h"
// #include "eeprom_app.h"
// #include "string.h"

#include "file_transfer.h"
#include "lfs_port.h"
#include "ymodem.h"

// // #include "sfud.h"
// // #include "spi.h" // 如果 spi 句柄定义在这里
#include "spi_flash.h"
#include "uart_dma.h"
// #include "lfs_port.h"
// 按键扫描任务

void vKey_Scan_Task(void *pvParameters);
#define Key_Scan_Task_PRIORITY (tskIDLE_PRIORITY + 2)
#define Key_Scan_Task_STACK_SIZE 128
#define Key_Scan_Task_PERIOD_MS 10
TaskHandle_t Key_Scan_Task_Handle;

// 按键分配任务
void vKey_Manllege_Task(void *pvParameters);
#define Key_Manllegr_Task_PRIORITY (tskIDLE_PRIORITY + 2)
#define Key_Manllegrn_Task_STACK_SIZE 256
#define Key_Manllegr_Task_PERIOD_MS 10
TaskHandle_t Key_Manllegr_Task_Handle;

// 音乐播放任务
void vMusic_Task(void *pvParameters);
#define Music_Task_PRIORITY (tskIDLE_PRIORITY + 2)
#define Music_Task_STACK_SIZE 128
#define Music_Task_PERIOD_MS 10
TaskHandle_t Music_Task_Handle;

// LED任务
void vLED_Task(void *pvParameters);
#define LED_Task_PRIORITY (tskIDLE_PRIORITY + 2)
#define LED_Task_STACK_SIZE 128
#define LED_Task_PERIOD_MS 10
TaskHandle_t LED_Task_Handle;
LED_Object_t led_blue;  // PE3 -> TIM3_CH1
LED_Object_t led_green; // PE4 -> TIM3_CH2
LED_Object_t led_red;   // PE5 -> TIM3_CH3
void LED_Update(void);

// //OLED任务队列
extern I2C_HandleTypeDef hi2c2;
void vOLED_Task(void *pvParameters);
#define OLED_Task_PRIORITY (tskIDLE_PRIORITY + 2)
#define OLED_Task_STACK_SIZE 512
#define OLED_Task_PERIOD_MS 20
TaskHandle_t oled_Task_Handle;

// Transmit任务队列
QueueHandle_t CommQueue;
void vTransmit_Task(void *pvParameters);
#define Transmit_Task_PRIORITY (tskIDLE_PRIORITY + 2)
#define Transmit_Task_STACK_SIZE 1024
#define Transmit_Task_PERIOD_MS 1000
TaskHandle_t Transmit_Task_Handle = NULL;

void vKey_Scan_Task(void *pvParameters)
{
    Key_Init();
    // vTaskDelay(3000 / portTICK_PERIOD_MS); // 等待系统稳定
    TickType_t key_scan_tick_count = xTaskGetTickCount();
    buttonQueue = xQueueCreate(16, sizeof(ButtonCommand_t));
    while (1)
    {

        Key_Scan();

        //  log_printf("Hello, this is a test log message.");
        vTaskDelayUntil(&key_scan_tick_count, Key_Scan_Task_PERIOD_MS);
    }
}

/**
 * @brief 按键管理任务（事件分发核心）
 *
 * 职责：
 * 1. 接收按键队列事件
 * 2. 处理系统级按键（BLE / 播放状态）
 * 3. 根据当前状态分发到 MUSIC / DISPLAY
 * 4. 控制 UI / 音乐 / LED
 */
void vKey_Manllege_Task(void *pvParameters)
{
    ButtonCommand_t keycmd;
    BaseType_t xStatus;

    comm_msg_t msgapp;

    while (1)
    {
        /* ================= 阻塞等待按键 ================= */
        xStatus = xQueueReceive(buttonQueue, &keycmd, portMAX_DELAY);

        if (xStatus == pdPASS)
        {
            /* =======================================================
             * 一、系统级按键（不依赖当前页面）
             * =======================================================*/
            switch (keycmd.id)
            {
            case 3: // 蓝牙控制
                if (keycmd.event == BTN_PRESS_UP)
                {
                    g_ble_state.ble_connected = 1;
                    g_ble_state.ble_ever_connected = 1;
                    Bluetooth_Connected();
                }
                else if (keycmd.event == BTN_PRESS_DOWN)
                {
                    g_ble_state.ble_connected = 0;

                    if (g_ble_state.ble_ever_connected)
                    {
                        Bluetooth_Disconnected();
                        // OLED_UI_SetPage(UI_PAGE_STOP);
                    }
                }

                LED_Update(); // 刷新LED状态
                break;

            case 4: // 播放状态控制
                if (keycmd.event == BTN_PRESS_UP)
                {
                    g_music_state.music_played = 1;
                    g_music_state.music_ever_played = 1;
                    Music_Playing();
                    // OLED_UI_SetPage(UI_PAGE_PLAY);
                }
                else if (keycmd.event == BTN_PRESS_DOWN)
                {
                    g_music_state.music_played = 0;

                    if (g_music_state.music_ever_played)
                    {
                        Music_Pause();
                        // OLED_UI_SetPage(UI_PAGE_STOP);
                    }
                }

                LED_Update(); // 刷新LED状态
                break;

            default:
                break;
            }

            /* ================= 重置休眠计时 ================= */
            Key_Event();

            /* =======================================================
             * 二、应用层状态机分发
             * =======================================================*/
            switch (g_app_current_state)
            {
            /* ================= MUSIC模式 ================= */
            case MUSIC_ON:

                if (g_ble_state.ble_connected == 0)
                {
                    OLED_UI_OnEvent(UI_EVT_BLUETOOTH_DISCONNECTED);
                    break;
                }
                switch (keycmd.id)
                {
                case 0: // 上键
                    if (keycmd.event == BTN_SINGLE_CLICK)
                    {
                        music_send_cmd(CMD_PREV);

                        LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                           LED_Heartbeat_Handler, 2000, 2000, NULL);

                        log_printf("CMD_PREV Button %d, event %d", keycmd.id, keycmd.event);
                        OLED_UI_OnEvent(UI_EVT_PREV);
                    }
                    else if (keycmd.event == BTN_DOUBLE_CLICK)
                    {
                        music_send_cmd(CMD_VOL_DOWN);

                        LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                           LED_Heartbeat_Handler, 2000, 2000, NULL);

                        OLED_UI_OnEvent(UI_EVT_VOL_DOWN);
                        log_printf("CMD_VOL_DOWN Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_LONG_PRESS_START)
                    {
                        log_printf("MUSIC_ON Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    break;

                case 1: // 中键（播放/暂停）
                    if (keycmd.event == BTN_SINGLE_CLICK)
                    {
                        music_send_cmd(CMD_PLAY_STOP);

                        LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                           LED_Heartbeat_Handler, 2000, 2000, NULL);

                        log_printf("CMD_PLAY_STOP Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_DOUBLE_CLICK)
                    {
                        OLED_UI_OnEvent(UI_EVT_BATTERY_CHARGING);
                        log_printf("UI_EVT_BATTERY_CHARGING Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_LONG_PRESS_START)
                    {
                        log_printf("Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    break;

                case 2: // 下键（下一曲 / 音量+）
                    if (keycmd.event == BTN_SINGLE_CLICK)
                    {
                        music_send_cmd(CMD_NEXT);

                        LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                           LED_Heartbeat_Handler, 2000, 2000, NULL);

                        log_printf("CMD_NEXT Button %d, event %d", keycmd.id, keycmd.event);
                        OLED_UI_OnEvent(UI_EVT_NEXT);
                    }
                    else if (keycmd.event == BTN_DOUBLE_CLICK)
                    {
                        music_send_cmd(CMD_VOL_UP);

                        LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                           LED_Heartbeat_Handler, 2000, 2000, NULL);

                        OLED_UI_OnEvent(UI_EVT_VOL_UP);
                        log_printf("CMD_VOL_UP Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_LONG_PRESS_START)
                    {
                        log_printf("MUSIC_ON Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    break;

                default:
                    break;
                }
                break;

            /* ================= DISPLAY模式（预留） ================= */
            case DISPLAY_ON:
                switch (keycmd.id)
                {
                case 0: // 上键
                    if (keycmd.event == BTN_SINGLE_CLICK)
                    {

                        msgapp.type = CMD_GET_AIR_DETAIL;
                        if (CommQueue != NULL)
                        {
                            xQueueSend(CommQueue, &msgapp, 0);
                            // log_printf("CMD_GET_FILE_LIST   send to CommQueue\r\n");
                        }
                        log_printf("CMD_PREV Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_DOUBLE_CLICK)
                    {

                        log_printf("CMD_VOL_DOWN Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_LONG_PRESS_START)
                    {
                        log_printf("MUSIC_ON Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    break;

                case 1: // 中键（播放/暂停）
                    if (keycmd.event == BTN_SINGLE_CLICK)
                    {
                        msgapp.type = CMD_GET_TIME;
                        if (CommQueue != NULL)
                        {
                            xQueueSend(CommQueue, &msgapp, 0);
                            // log_printf("CMD_GET_FILE_LIST   send to CommQueue\r\n");
                        }

                        log_printf("CMD_PLAY_STOP Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_DOUBLE_CLICK)
                    {

                          msgapp.type = CMD_GET_FUTURE_7DAY;
                        if (CommQueue != NULL)
                        {
                            xQueueSend(CommQueue, &msgapp, 0);
                            // log_printf("CMD_GET_FILE_LIST   send to CommQueue\r\n");
                        }  
                        log_printf("UI_EVT_BATTERY_CHARGING Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_LONG_PRESS_START)
                    {
                        log_printf("Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    break;

                case 2: // 下键（下一曲 / 音量+）
                    if (keycmd.event == BTN_SINGLE_CLICK)
                    {
                        msgapp.type = CMD_GET_NOW_DETAIL;
                        if (CommQueue != NULL)
                        {
                            xQueueSend(CommQueue, &msgapp, 0);
                        }

                        log_printf("CMD_NEXT Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_DOUBLE_CLICK)
                    {

                        log_printf("CMD_VOL_UP Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    else if (keycmd.event == BTN_LONG_PRESS_START)
                    {
                        log_printf("MUSIC_ON Button %d, event %d", keycmd.id, keycmd.event);
                    }
                    break;

                default:
                    break;
                }
                break;
            default:
                g_app_current_state = MUSIC_ON; // 异常恢复
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
    ret = xTaskCreate((TaskFunction_t)vKey_Scan_Task,
                      "Key_Scan_Task",
                      Key_Scan_Task_STACK_SIZE,
                      NULL,
                      Key_Scan_Task_PRIORITY,
                      &Key_Scan_Task_Handle);
    if (ret != pdPASS)
    {
        log_printf("key task create FAIL\r\n");
    }

    // 创建按键分配任务
    ret = xTaskCreate((TaskFunction_t)vKey_Manllege_Task,
                      "Key_Manllegr_Task",
                      Key_Manllegrn_Task_STACK_SIZE,
                      NULL,
                      Key_Manllegr_Task_PRIORITY,
                      &Key_Manllegr_Task_Handle);
    if (ret != pdPASS)
    {
        log_printf("key Manllegr task create FAIL\r\n");
    }

    // 创建音乐播放任务
    ret = xTaskCreate((TaskFunction_t)vMusic_Task,
                      "Music_Task",
                      Music_Task_STACK_SIZE,
                      NULL,
                      Music_Task_PRIORITY,
                      &Music_Task_Handle);
    if (ret != pdPASS)
    {
        log_printf("music task create FAIL\r\n");
    }
    // 创建LED任务
    ret = xTaskCreate((TaskFunction_t)vLED_Task,
                      "LED_Task",
                      LED_Task_STACK_SIZE,
                      NULL,
                      LED_Task_PRIORITY,
                      &LED_Task_Handle);
    if (ret != pdPASS)
    {
        log_printf("LED task create FAIL\r\n");
    }

    // 创建OLED任务
    ret = xTaskCreate((TaskFunction_t)vOLED_Task,
                      "My_OLED_Task",
                      OLED_Task_STACK_SIZE,
                      NULL,
                      OLED_Task_PRIORITY,
                      &oled_Task_Handle);
    if (ret != pdPASS)
    {
        log_printf("OLED task create FAIL\r\n");
    }

    // 创建任务
    ret = xTaskCreate((TaskFunction_t)vTransmit_Task,
                      "My_Transmit_Task",
                      Transmit_Task_STACK_SIZE,
                      NULL,
                      Transmit_Task_PRIORITY,
                      &Transmit_Task_Handle);
    if (ret != pdPASS)
    {
        log_printf("Transmit task create FAIL\r\n");
    }
}

void vMusic_Task(void *pvParameters)
{
    MusicCtrlCmd cmd;
    music_app_init();
    // 测试的时候不开机
    // Music_PowerOn();    // 开机
    vTaskDelay(pdMS_TO_TICKS(50)); // 等待设备稳定
    while (1)
    {
        if (xQueueReceive(music_cmd_queue, &cmd, portMAX_DELAY) == pdPASS)
        {
            //   if(g_ble_state.ble_connected == 0)
            //          {
            //            return;
            //          }
            switch (cmd)
            {
            case CMD_PLAY_STOP:
                Music_Play_Stop();
                break;
            case CMD_PREV:
                Music_Up();
                break;
            case CMD_NEXT:
                Music_Next();
                break;
            case CMD_PAIR:
                Music_Pair();
                break;
            case CMD_CLEAR_PAIR:
                Music_ClearPair();
                break;
            case CMD_POWER_ON:
                Music_PowerOn();
                break;
            case CMD_POWER_OFF:
                Music_PowerOff();
                break;
            case CMD_VOL_UP:
                Music_VolumeUp();
                break;
            case CMD_VOL_DOWN:
                Music_VolumeDown();
                break;
            default:
                break;
            }
        }
    }
}

void vOLED_Task(void *arg)
{
    // 初始化 UI 框架
    OLED_UI_Init(&hi2c2);

    OLED_UI_SetBattery(60, 0);

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

    LED_Driver_Init(&led_green, LED_G_GPIO_Port, LED_G_Pin, &htim3, TIM_CHANNEL_2, 1);
    LED_Driver_Init(&led_blue, LED_B_GPIO_Port, LED_B_Pin, &htim3, TIM_CHANNEL_1, 1);
    LED_Driver_Init(&led_red, LED_R_GPIO_Port, LED_R_Pin, &htim3, TIM_CHANNEL_3, 1);
    LED_Driver_SendCmd(&led_blue, LED_MODE_PWM, LED_Off_Handler, 0, 0, NULL);
    LED_Driver_SendCmd(&led_green, LED_MODE_PWM, LED_Off_Handler, 0, 0, NULL);
    LED_Driver_SendCmd(&led_red, LED_MODE_PWM, LED_Heartbeat_Handler, 2000, 0, NULL);
    TickType_t led_scan_tick_count = xTaskGetTickCount();
    while (1)
    {

        LED_Driver_Update();
        //  log_printf("Hello, this is a test log message.");
        vTaskDelayUntil(&led_scan_tick_count, LED_Task_PERIOD_MS);
    }
}

static void LED_Update(void)

{
    /* =========================
     * 1. BLE未连接（最高优先级）
     * ========================= */
    if (g_ble_state.ble_connected == 0)
    {
        LED_Driver_SendCmd(&led_green,
                           LED_MODE_PWM,
                           LED_Off_Handler,
                           0, 0, NULL);

        LED_Driver_SendCmd(&led_red,
                           LED_MODE_PWM,
                           LED_Blink_Handler,
                           1000, 0, NULL);

        // OLED_UI_OnEvent(UI_EVT_PAUSE);
        OLED_UI_SetPage(UI_PAGE_HOME);
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
    if (g_music_state.music_played == 0)
    {
        /* 未播放 → 闪绿灯 */
        LED_Driver_SendCmd(&led_green,
                           LED_MODE_PWM,
                           LED_Blink_Handler,
                           1000, 0, NULL);

        OLED_UI_SetPage(UI_PAGE_STOP);
    }
    else
    {
        /* 播放中 → 呼吸灯 */
        LED_Driver_SendCmd(&led_green,
                           LED_MODE_PWM,
                           LED_Breath_Handler,
                           5000, 0, NULL);
        OLED_UI_SetPage(UI_PAGE_PLAY);
    }
}

// spi_flash_t flash_32mb = {0};

// #define TEST_FILE_NAME    "hello.txt"
// #define TEST_CONTENT      "1234567890"

// void vTransmit_Task(void *pvParameters)
// {
//     /* 1. 硬件与文件系统初始化 */
//     if (spi_flash_init(&flash_32mb, &hspi2, SPI2_CS_GPIO_Port, SPI2_CS_Pin) != 0) {
//         log_printf("Flash Hardware Init Failed!\r\n");
//         vTaskDelete(NULL);
//         return;
//     }
//     lfs_port_init(&flash_32mb);
//     lfs_t *lfs = lfs_port_get();
//     lfs_file_t file;

//     log_printf("--- LittleFS Simple Test Start ---\r\n");

//     /* 2. 准备测试数据（写文件） */
//     int err = lfs_file_open(lfs, &file, TEST_FILE_NAME, LFS_O_CREAT | LFS_O_RDWR | LFS_O_TRUNC);
//     if (err >= 0) {
//         lfs_file_write(lfs, &file, TEST_CONTENT, strlen(TEST_CONTENT));
//         lfs_file_sync(lfs, &file);
//         lfs_file_close(lfs, &file);
//         log_printf("Write Done: %s\r\n", TEST_CONTENT);
//     }

//     /* 3. 循环：列出文件 + 读取内容 */
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     static char read_buf[32];

//     for (;;)
//     {
//         log_printf("\r\n--- Storage Status ---\r\n");

//         /* A. 打印文件列表 (ls 功能) */
//         lfs_dir_t dir;
//         struct lfs_info info;
//         if (lfs_dir_open(lfs, &dir, "/") >= 0) {
//             log_printf("Files in root:\r\n");
//             while (lfs_dir_read(lfs, &dir, &info) > 0) {
//                 // 过滤掉 '.' 和 '..' 目录
//                 if (info.name[0] == '.') continue;
//                 log_printf("  - %s \t size: %ld bytes\r\n", info.name, info.size);
//             }
//             lfs_dir_close(lfs, &dir);
//         }

//         /* B. 读取文件内容验证 */
//         if (lfs_file_open(lfs, &file, TEST_FILE_NAME, LFS_O_RDONLY) >= 0) {
//             memset(read_buf, 0, sizeof(read_buf));
//             int rd = lfs_file_read(lfs, &file, read_buf, sizeof(read_buf) - 1);
//             if (rd > 0) {
//                 log_printf("[DATA] Content: %s\r\n", read_buf);
//             }
//             lfs_file_close(lfs, &file);
//         }

//         // 每 3 秒刷新一次
//         vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(3000));
//     }
// }

// void vTransmit_Task(void *pvParameters)
// {
//     uint8_t temp_buf[128];

//     uint32_t total_rx = 0;
//     uint32_t print_count = 0;

//     int bytes_read;

//     /* 初始化 */
//     uart_dma_init();

//     log_printf("UART DMA Ready!\r\n");

//     while (1)
//     {
//         /* 读取 RingBuffer */
//         while ((bytes_read = uart_dma_read(temp_buf,
//                                            sizeof(temp_buf),
//                                            0)) > 0)
//         {
//             total_rx += bytes_read;

//             /* 每1000字节打印一次 */
//             while (total_rx >= 1000)
//             {
//                 print_count++;

//                 log_printf("RX 1000 Bytes OK [%lu]\r\n",
//                            print_count);

//                 total_rx -= 1000;
//             }
//         }

//         /* 降低CPU占用 */
//         vTaskDelay(pdMS_TO_TICKS(10));
//     }
// }






void vTransmit_Task(void *pvParameters)
{
    transfer_init();
    comm_msg_t msg;
    uint8_t rx_buf[256]; // 用于接收响应字符串
    lfs_t *lfs = lfs_port_get();
     uart_dma_init();
    // 初始化队列（建议挪到 main，但留在这里请确保发送端有判空检查）
    CommQueue = xQueueCreate(16, sizeof(comm_msg_t));
    log_printf("[Comm] Transmit Task Started (ESP32 Protocol Aligned)...\r\n");

    for (;;)
    {
        if (xQueueReceive(CommQueue, &msg, portMAX_DELAY) == pdPASS)
        {
            lwrb_reset(&uart_rb); // 发送前清空接收区

            switch (msg.type)
            {
            /* --- 情况 A: 获取同步时间 --- */
            case CMD_GET_TIME:{
                log_printf("[Comm] Requesting Time...\r\n");
                HAL_UART_Transmit(&huart1, (uint8_t[]){FRAME_HEAD, CMD_GET_TIME, FRAME_TAIL}, 3, 100);

                // 等待字符串响应 (ESP32 发回的是字符串，如 "2024-05-14 12:00:00")
                memset(rx_buf, 0, sizeof(rx_buf));
                if (uart_dma_read(rx_buf, sizeof(rx_buf), 1000) > 0)
                {
                    log_printf("[ESP32] Time: %s\r\n", rx_buf);
                }
            
                break;}

            /* --- 情况 B: 获取天气详情 (逗号分隔字符串) --- */
            case CMD_GET_NOW_DETAIL:{
                log_printf("[Comm] Requesting Weather...\r\n");
                HAL_UART_Transmit(&huart1, (uint8_t[]){FRAME_HEAD, CMD_GET_NOW_DETAIL, FRAME_TAIL}, 3, 100);

                memset(rx_buf, 0, sizeof(rx_buf));
                if (uart_dma_read(rx_buf, sizeof(rx_buf), 1500) > 0)
                {
                    log_printf("[ESP32] Weather Data: %s\r\n", rx_buf);
                    // 这里可以用 strtok(rx_buf, ",") 来拆解数据
                }
                
                break;}
            case CMD_GET_FUTURE_7DAY:{
                log_printf("[Comm] Requesting 7-Day Forecast...\r\n");
                // 1. 发送请求给 ESP32: AA 04 55
                HAL_UART_Transmit(&huart1, (uint8_t[]){FRAME_HEAD, CMD_GET_FUTURE_7DAY, FRAME_TAIL}, 3, 100);

                // 2. 循环读取 ESP32 推送过来的每一行数据
                // ESP32 会发：LIST_START -> 7行数据 -> LIST_END
                bool list_finished = false;
                uint32_t retry_count = 0;

                while (!list_finished && retry_count < 20) // 增加保护，防止死循环
                {
                    memset(rx_buf, 0, sizeof(rx_buf));
                    // 注意：ESP32 的 protocol_send_frame 内部通常会有换行符或固定结束位
                    // 我们假设每行数据最长 128 字节，等待时间给 500ms
                    int len = uart_dma_read(rx_buf, sizeof(rx_buf), 500);

                    if (len > 0)
                    {
                        char *line = (char *)rx_buf;

                        if (strstr(line, "LIST_START"))
                        {
                            log_printf("[Weather] Start receiving list...\r\n");
                        }
                        else if (strstr(line, "LIST_END"))
                        {
                            log_printf("[Weather] All 7 days received.\r\n");
                            list_finished = true;
                        }
                        else
                        {
                            // 这里收到的就是具体的某一天数据，格式类似：0|日期|天气|高温|低温|图标ID
                            log_printf("[Weather] Day Info: %s\r\n", line);

                            // 如果需要解析：
                            // int day_idx = atoi(strtok(line, "|"));
                            // char* date = strtok(NULL, "|");
                            // ... 存入数组或显示在屏幕上
                        }
                    }
                    else
                    {
                        retry_count++; // 如果没收到数据，累加重试
                    }
                }

                if (!list_finished)
                {
                    log_printf("[Comm] Receive Forecast Timeout!\r\n");
                }
                break;}
            
case CMD_GET_AIR_DETAIL:{
    log_printf("[Comm] Requesting Air Quality Detail...\r\n");
    // 1. 发送请求: AA 03 55
    HAL_UART_Transmit(&huart1, (uint8_t[]){FRAME_HEAD, CMD_GET_AIR_DETAIL, FRAME_TAIL}, 3, 100);

    // 2. 等待 ESP32 返回字符串 (例如: "50,15,30,5,2,1,20")
    // 对应 ESP32 发送顺序：AQI, PM10, PM2.5, NO2, SO2, CO, O3
    memset(rx_buf, 0, sizeof(rx_buf));
    if (uart_dma_read(rx_buf, sizeof(rx_buf), 1500) > 0) 
    {
        log_printf("[ESP32] Air Data: %s\r\n", rx_buf);

        /* --- 选做：解析数据以便在 LCD 上显示 --- */
        int aqi, pm10, pm25, no2, so2;
        float co, o3;
        
        // 使用 sscanf 快速解析逗号分隔的数字
        // 注意：根据 ESP32 发送的是整数还是浮点数调整格式符
        int res = sscanf((char*)rx_buf, "%d,%d,%d,%d,%d,%f,%f", 
                         &aqi, &pm10, &pm25, &no2, &so2, &co, &o3);
                         
        if (res >= 3) { // 至少解析出了前三项重要指标
            log_printf("[Air] AQI:%d, PM2.5:%d\r\n", aqi, pm25);
            // 这里调用你的 UI 显示函数，例如：LCD_ShowAir(aqi, pm25...);
        }
    } 
    else 
    {
        log_printf("[Comm] Air Detail Timeout!\r\n");
    }
    break;}
    case CMD_FS_LIST:{
    log_printf("[Comm] Requesting File List from LittleFS...\r\n");
    // 1. 发送请求给 ESP32: AA 20 55
    HAL_UART_Transmit(&huart1, (uint8_t[]){FRAME_HEAD, CMD_FS_LIST, FRAME_TAIL}, 3, 100);

    // 2. 进入流式接收模式
    bool fs_list_end = false;
    uint32_t fs_timeout_tick = HAL_GetTick();

    while (!fs_list_end) 
    {
        // 检查总超时（例如 5 秒没收完则退出，防止死循环）
        if (HAL_GetTick() - fs_timeout_tick > 5000) {
            log_printf("[Comm] FS List Timeout!\r\n");
            break;
        }

        memset(rx_buf, 0, sizeof(rx_buf));
        // 每行数据等待时间设短一点（如 500ms），因为 ESP32 是连续发的
        int len = uart_dma_read(rx_buf, sizeof(rx_buf), 500);

        if (len > 0) 
        {
            char* line = (char*)rx_buf;

            // A. 检查是否是开始标识
            if (strstr(line, "LIST_START")) {
                log_printf("---- Remote File List Start ----\r\n");
            }
            // B. 检查是否是结束标识
            else if (strstr(line, "LIST_END")) {
                log_printf("---- Remote File List End ----\r\n");
                fs_list_end = true;
            }
            // C. 处理具体的文件信息行 (格式: filename|size)
            else if (strstr(line, "|")) {
                char *name = strtok(line, "|");
                char *size_str = strtok(NULL, "|");
                
                if (name && size_str) {
                    log_printf("File: %-16s  Size: %s bytes\r\n", name, size_str);
                    
                    // 如果你需要把列表存起来给 UI 显示，可以在这里写入数组
                    // update_file_list_array(name, atoi(size_str));
                }
            }
        }
    }
    break;}


            /* --- 情况 C: 选择文件并下载 (变长指令 + YModem) --- */
            case CMD_FS_SELECT:{
                log_printf("[Comm] Selecting File: %s\r\n", msg.filename);

                // 1. 发送变长指令: AA 22 [name] 55
                uint8_t head = FRAME_HEAD;
                uint8_t cmd = CMD_FS_SELECT;
                uint8_t tail = FRAME_TAIL;
                HAL_UART_Transmit(&huart1, &head, 1, 10);
                HAL_UART_Transmit(&huart1, &cmd, 1, 10);
                HAL_UART_Transmit(&huart1, (uint8_t *)msg.filename, strlen(msg.filename), 100);
                HAL_UART_Transmit(&huart1, &tail, 1, 10);

                // 2. 等待 ESP32 准备就绪响应 "READY"
                memset(rx_buf, 0, sizeof(rx_buf));
                uart_dma_read(rx_buf, 5, 1000); // 简单读取 "READY"

                if (strstr((char *)rx_buf, "READY"))
                {
                    log_printf("[Comm] ESP32 Ready, starting YModem...\r\n");
                    vTaskDelay(pdMS_TO_TICKS(100)); // 略微停顿

                    // 3. 发送启动指令: AA 11 55
                    HAL_UART_Transmit(&huart1, (uint8_t[]){FRAME_HEAD, CMD_OTA_START, FRAME_TAIL}, 3, 100);

                    // 4. 进入 YModem 接收流程 (你需要实现这个函数)
                    // handle_ymodem_receive(lfs, msg.filename);
                }
                else
                {
                    log_printf("[Comm] ESP32 Select File Failed!\r\n");
                }
                break;}

            case CMD_RESTART:{
                HAL_UART_Transmit(&huart1, (uint8_t[]){FRAME_HEAD, CMD_RESTART, FRAME_TAIL}, 3, 100);
                break;}

            default:
                break;
            }
        }
    }
}