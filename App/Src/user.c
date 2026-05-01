/**
  ******************************************************************************
  * @file    : user.c
  * @brief   : FreeRTOS任务创建与调度
  ******************************************************************************
  */
#include "user.h"
#include "app_config.h"
#include "log_uart.h"
#include "modbus_master.h"
#include "modbus_slave.h"
#include "sensor_manager.h"
#include "hikvision_isapi.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* 任务栈大小 */
#define TASK_LOG_STK        128
#define TASK_MBM_STK        256
#define TASK_MBS_STK        128
#define TASK_SEN_STK        256
#define TASK_API_STK        512
#define TASK_MON_STK        128

/* 任务句柄 */
static TaskHandle_t hTaskLog, hTaskMBM, hTaskMBS, hTaskSen, hTaskAPI, hTaskMon;
static TimerHandle_t hTmr10ms, hTmr100ms;

/* 定时器回调 */
static void Tmr10msCb(TimerHandle_t t)  { (void)t; Modbus_Master_Timer10ms(); Modbus_Slave_Timer10ms(); }
static void Tmr100msCb(TimerHandle_t t) { (void)t; Sensor_Manager_Timer100ms(); }

/* 任务函数 */
static void TaskLog(void *p)   { (void)p; for(;;) { Log_UART_Process(); vTaskDelay(10); } }
static void TaskMBM(void *p)   { (void)p; for(;;) { Modbus_Master_Process(); vTaskDelay(TIM3_PERIOD_MS); } }
static void TaskMBS(void *p)   { (void)p; for(;;) { Modbus_Slave_Process(); vTaskDelay(10); } }
static void TaskSen(void *p)   { (void)p; for(;;) { Sensor_Manager_Process(); vTaskDelay(TIM3_PERIOD_MS); } }
static void TaskAPI(void *p)   { (void)p; for(;;) { Hikvision_ISAPI_Process(); vTaskDelay(100); } }
static void TaskMon(void *p)   { (void)p; for(;;) {
    Log_Printf("SYS: heap=%lu min=%lu\r\n",
               (unsigned long)xPortGetFreeHeapSize(),
               (unsigned long)xPortGetMinimumEverFreeHeapSize());
    vTaskDelay(30000);
}}

void User_Init(void)
{
    /* 初始化日志 */
    Log_UART_Init(&huart5, UART5_EN_GPIO_Port, UART5_EN_Pin);
    Log_Printf("\r\n==============================\r\n");
    Log_Printf("  MimoCH395 v%s (%s)\r\n", FW_VERSION, FW_DATE);
    Log_Printf("==============================\r\n");

    /* 初始化模块 */
    Modbus_Master_Init(&huart4, UART4_EN_GPIO_Port, UART4_EN_Pin);
    Modbus_Slave_Init(&huart1, UART1_EN_GPIO_Port, UART1_EN_Pin);
    Sensor_Manager_Init();
    Hikvision_ISAPI_Init();

    /* 创建定时器 */
    hTmr10ms  = xTimerCreate("T10",  pdMS_TO_TICKS(10),  pdTRUE, NULL, Tmr10msCb);
    hTmr100ms = xTimerCreate("T100", pdMS_TO_TICKS(100), pdTRUE, NULL, Tmr100msCb);

    /* 创建任务 */
    xTaskCreate(TaskLog, "Log",  TASK_LOG_STK, NULL, osPriorityLow,          &hTaskLog);
    xTaskCreate(TaskMBM, "MBM",  TASK_MBM_STK, NULL, osPriorityAboveNormal,  &hTaskMBM);
    xTaskCreate(TaskMBS, "MBS",  TASK_MBS_STK, NULL, osPriorityAboveNormal,  &hTaskMBS);
    xTaskCreate(TaskSen, "Sen",  TASK_SEN_STK, NULL, osPriorityNormal,       &hTaskSen);
    xTaskCreate(TaskAPI, "API",  TASK_API_STK, NULL, osPriorityNormal,       &hTaskAPI);
    xTaskCreate(TaskMon, "Mon",  TASK_MON_STK, NULL, osPriorityBelowNormal,  &hTaskMon);

    /* 启动定时器 */
    xTimerStart(hTmr10ms, 0);
    xTimerStart(hTmr100ms, 0);

    Log_Printf("[INIT] Tasks+Timers started\r\n");
}
