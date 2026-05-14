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

#include "ymodem.h"
#include "lfs_port.h"
#include "file_transfer.h"

// // #include "sfud.h"
// // #include "spi.h" // 如果 spi 句柄定义在这里
#include "spi_flash.h"
#include "uart_dma.h"
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

//Transmit任务队列
QueueHandle_t CommQueue;
void vTransmit_Task(void *pvParameters);     
#define Transmit_Task_PRIORITY   (tskIDLE_PRIORITY + 2)
#define Transmit_Task_STACK_SIZE 1024
#define Transmit_Task_PERIOD_MS  1000
TaskHandle_t Transmit_Task_Handle;





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



    while (1)
    {
        /* ================= 阻塞等待按键 ================= */
        xStatus = xQueueReceive(buttonQueue, &keycmd, portMAX_DELAY);

        if(xStatus == pdPASS)
        {
            /* =======================================================
             * 一、系统级按键（不依赖当前页面）
             * =======================================================*/
            switch(keycmd.id)
            {
                case 3: // 蓝牙控制
                    if(keycmd.event == BTN_PRESS_UP)
                    {
                        g_ble_state.ble_connected = 1;
                        g_ble_state.ble_ever_connected = 1;
                        Bluetooth_Connected();
                        
                    }
                    else if(keycmd.event == BTN_PRESS_DOWN)
                    {
                        g_ble_state.ble_connected = 0;

                        if(g_ble_state.ble_ever_connected)
                        {
                            Bluetooth_Disconnected();
                            // OLED_UI_SetPage(UI_PAGE_STOP);
                        }
                    }

                    LED_Update();   // 刷新LED状态
                    break;

                case 4: // 播放状态控制
                    if(keycmd.event == BTN_PRESS_UP)
                    {
                        g_music_state.music_played = 1;
                        g_music_state.music_ever_played = 1;
                        Music_Playing();
                        // OLED_UI_SetPage(UI_PAGE_PLAY);
                    }
                    else if(keycmd.event == BTN_PRESS_DOWN)
                    {
                        g_music_state.music_played = 0;

                        if(g_music_state.music_ever_played)
                        {
                            Music_Pause();
                            // OLED_UI_SetPage(UI_PAGE_STOP);
                        }
                    }

                    LED_Update();   // 刷新LED状态
                    break;

                default:
                    break;
            }

            /* ================= 重置休眠计时 ================= */
            Key_Event();


            /* =======================================================
             * 二、应用层状态机分发
             * =======================================================*/
            switch(g_app_current_state)
            {
                /* ================= MUSIC模式 ================= */
                case MUSIC_ON:

                       if(g_ble_state.ble_connected == 0)
                     {
                      OLED_UI_OnEvent(UI_EVT_BLUETOOTH_DISCONNECTED);
                       break;
                     }
                    switch(keycmd.id)
                    {
                        case 0: // 上键
                            if(keycmd.event == BTN_SINGLE_CLICK)
                            {
                                music_send_cmd(CMD_PREV);

                                LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                    LED_Heartbeat_Handler, 2000, 2000, NULL);

                                log_printf("CMD_PREV Button %d, event %d", keycmd.id, keycmd.event);
                                OLED_UI_OnEvent(UI_EVT_PREV);
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK)
                            {
                                music_send_cmd(CMD_VOL_DOWN);

                                LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                    LED_Heartbeat_Handler, 2000, 2000, NULL);

                                OLED_UI_OnEvent(UI_EVT_VOL_DOWN);
                                log_printf("CMD_VOL_DOWN Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START)
                            {
                                log_printf("MUSIC_ON Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            break;

                        case 1: // 中键（播放/暂停）
                            if(keycmd.event == BTN_SINGLE_CLICK)
                            {
                                music_send_cmd(CMD_PLAY_STOP);

                                LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                    LED_Heartbeat_Handler, 2000, 2000, NULL);

                                log_printf("CMD_PLAY_STOP Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK)
                            {
                                OLED_UI_OnEvent(UI_EVT_BATTERY_CHARGING);
                                log_printf("UI_EVT_BATTERY_CHARGING Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START)
                            {
                                log_printf("Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            break;

                        case 2: // 下键（下一曲 / 音量+）
                            if(keycmd.event == BTN_SINGLE_CLICK)
                            {
                                music_send_cmd(CMD_NEXT);

                                LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                    LED_Heartbeat_Handler, 2000, 2000, NULL);

                                log_printf("CMD_NEXT Button %d, event %d", keycmd.id, keycmd.event);
                                OLED_UI_OnEvent(UI_EVT_NEXT);
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK)
                            {
                                music_send_cmd(CMD_VOL_UP);

                                LED_Driver_SendCmd(&led_blue, LED_MODE_PWM,
                                    LED_Heartbeat_Handler, 2000, 2000, NULL);

                                OLED_UI_OnEvent(UI_EVT_VOL_UP);
                                log_printf("CMD_VOL_UP Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START)
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
                  switch(keycmd.id)
                    {
                        case 0: // 上键
                            if(keycmd.event == BTN_SINGLE_CLICK)
                            {

                                log_printf("CMD_PREV Button %d, event %d", keycmd.id, keycmd.event);

                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK)
                            {

                                log_printf("CMD_VOL_DOWN Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START)
                            {
                                log_printf("MUSIC_ON Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            break;

                        case 1: // 中键（播放/暂停）
                            if(keycmd.event == BTN_SINGLE_CLICK)
                            {

                                log_printf("CMD_PLAY_STOP Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK)
                            {

                                log_printf("UI_EVT_BATTERY_CHARGING Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START)
                            {
                                log_printf("Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            break;

                        case 2: // 下键（下一曲 / 音量+）
                            if(keycmd.event == BTN_SINGLE_CLICK)
                            {

                                log_printf("CMD_NEXT Button %d, event %d", keycmd.id, keycmd.event);

                            }
                            else if(keycmd.event == BTN_DOUBLE_CLICK)
                            {
       
                                log_printf("CMD_VOL_UP Button %d, event %d", keycmd.id, keycmd.event);
                            }
                            else if(keycmd.event == BTN_LONG_PRESS_START)
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

    // 创建任务
    ret = xTaskCreate((TaskFunction_t)vTransmit_Task,
                      "My_Transmit_Task",
                      Transmit_Task_STACK_SIZE,
                      NULL,
                      Transmit_Task_PRIORITY,
                      &Transmit_Task_Handle);
    if (ret != pdPASS) {
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
    while (1) {

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
    if(g_music_state.music_played == 0)
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

// #define TEST_BUF_SIZE  4096       // 验证 4KB 数据
// uint8_t test_buffer[TEST_BUF_SIZE]; // 静态大数组
// uint32_t test_index = 0;           // 当前写入的位置
// uint8_t is_test_full = 0;          // 完成标志

// void vTransmit_Task(void *pvParameters)
// {
//     uint8_t temp_buf[128]; // 从 LwRB 读取时的临时小缓冲
//     int bytes_read;

//     uart_dma_init();
//     log_printf("Ready! Please send a file (exactly %d bytes)...\r\n", TEST_BUF_SIZE);

//     while (1)
//     {
//         // 等待信号量或任务通知
//         if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) > 0)
//         {
//             // 只要 LwRB 有数据就持续读取
//             while ((bytes_read = uart_dma_read(temp_buf, sizeof(temp_buf), 0)) > 0)
//             {
//                 if (is_test_full == 0)
//                 {
//                     for (int i = 0; i < bytes_read; i++)
//                     {
//                         if (test_index < TEST_BUF_SIZE)
//                         {
//                             test_buffer[test_index++] = temp_buf[i];
//                         }
//                         else
//                         {
//                             is_test_full = 1; // 填满了
//                             break;
//                         }
//                     }
//                 }
//             }

//             // 当数组填满后，一次性打印出来
//             if (is_test_full == 1)
//             {
//                 log_printf("\r\n--- TEST DATA START ---\r\n");
//                 for (uint32_t i = 0; i < TEST_BUF_SIZE; i++)
//                 {
//                     // 打印十六进制格式，每 32 字节换一行
//                     log_printf("%02X ", test_buffer[i]);
//                     if ((i + 1) % 32 == 0) log_printf("\r\n");
//                 }
//                 log_printf("\r\n--- TEST DATA END (Total: %d) ---\r\n", TEST_BUF_SIZE);
                
//                 is_test_full = 2; // 标记已打印完，不再处理
//             }
//         }
//     }
// }


// 假设已经获取了 LittleFS 的指针
extern lfs_t *lfs_port_get(void);

void vTransmit_Task(void *pvParameters) 
{
    transfer_init();
    comm_msg_t msg;
    uint8_t rx_byte;
    lfs_file_t lfs_file;
    lfs_t *lfs = lfs_port_get();
   CommQueue = xQueueCreate(16, sizeof(comm_msg_t));
    log_printf("[Comm] Transmit Task Started...\r\n");

    for (;;) 
    {
        /* 1. 等待队列指令，永久阻塞直到有任务想发送东西 */
        if (xQueueReceive(CommQueue, &msg, portMAX_DELAY) == pdPASS) 
        {
            /* 每次发送指令前，清空 LwRB 环形缓冲区，防止之前的杂质数据干扰响应解析 */
            lwrb_reset(&uart_rb);

            switch (msg.type) 
            {
                /* -------------------------------------------
                   测试指令 1：同步时间 (AA 01 55)
                   ------------------------------------------- */
                case CMD_SYNC_TIME:
                    log_printf("[Comm] Sending Sync Time Command...\r\n");
                    HAL_UART_Transmit(&huart3, (uint8_t[]){0xAA, 0x01, 0x55}, 3, 100);
                    
                    // 等待响应示例：假设 ESP32 返回以 AA 开头的数据
                    if (uart_dma_read(&rx_byte, 1, 1000) > 0) {
                        if (rx_byte == 0xAA) {
                            log_printf("[Comm] ESP32 Responded OK!\r\n");
                        }
                    } else {
                        log_printf("[Comm] Sync Time Timeout!\r\n");
                    }
                    break;

                /* -------------------------------------------
                   测试指令 2：获取文件列表 (AA 20 55)
                   ------------------------------------------- */
                case CMD_GET_FILE_LIST:
                    log_printf("[Comm] Requesting File List...\r\n");
                    HAL_UART_Transmit(&huart3, (uint8_t[]){0xAA, 0x20, 0x55}, 3, 100);
                    
                    // 这里可以接一个循环读取逻辑，把 ESP32 发回的文件名打印出来
                    break;

                /* -------------------------------------------
                   测试指令 3：下载文件并保存到 LittleFS
                   ------------------------------------------- */
                case CMD_DOWNLOAD_FILE:
                    log_printf("[Comm] Target File: %s\r\n", msg.filename);
                    
                    // A. 告诉 ESP32 选中该文件 (需根据你定义的协议发送文件名)
                    // send_file_select_command(msg.filename); 
                    
                    vTaskDelay(pdMS_TO_TICKS(50)); // 稍作停顿

                    // B. 发送启动 YModem 指令
                    log_printf("[Comm] Starting YModem Transfer...\r\n");
                    HAL_UART_Transmit(&huart3, (uint8_t[]){0xAA, 0x11, 0x55}, 3, 100);

                    // C. YModem 逻辑
                    // 1. 等待 YModem 帧头
                    // if (ymodem_wait_receive_header(&g_file_info, 2000)) { ... }
                    // 2. 接收并写入 LFS
                    // ymodem_receive_file_with_callback(..., packet_callback_to_lfs, &lfs_file);
                    
                    log_printf("[Comm] YModem sequence finished.\r\n");
                    break;

                default:
                    break;
            }
        }
    }
}