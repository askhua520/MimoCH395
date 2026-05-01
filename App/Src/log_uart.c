/**
  ******************************************************************************
  * @file    : log_uart.c
  * @brief   : 日志串口系统实现
  ******************************************************************************
  */
#include "log_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define LOG_BUF_SIZE    512
#define LOG_TIMEOUT     100

static UART_HandleTypeDef *s_huart = NULL;
static GPIO_TypeDef       *s_en_port = NULL;
static uint16_t            s_en_pin = 0;
static SemaphoreHandle_t   s_mutex = NULL;
static char                s_buf[LOG_BUF_SIZE];

void Log_UART_Init(UART_HandleTypeDef *huart, GPIO_TypeDef *en_port, uint16_t en_pin)
{
    s_huart = huart;
    s_en_port = en_port;
    s_en_pin = en_pin;
    if (s_mutex == NULL) s_mutex = xSemaphoreCreateMutex();
    HAL_GPIO_WritePin(s_en_port, s_en_pin, GPIO_PIN_RESET);
}

void Log_Printf(const char *fmt, ...)
{
    if (s_huart == NULL || s_mutex == NULL) return;
    if (xSemaphoreTake(s_mutex, pdMS_TO_TICKS(LOG_TIMEOUT)) != pdTRUE) return;

    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(s_buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    if (len > 0)
    {
        HAL_GPIO_WritePin(s_en_port, s_en_pin, GPIO_PIN_SET);
        HAL_UART_Transmit(s_huart, (uint8_t *)s_buf, (uint16_t)len, LOG_TIMEOUT);
        while (__HAL_UART_GET_FLAG(s_huart, UART_FLAG_TC) == RESET) {}
        HAL_GPIO_WritePin(s_en_port, s_en_pin, GPIO_PIN_RESET);
    }

    xSemaphoreGive(s_mutex);
}

void Log_UART_Process(void)
{
    /* 当前同步发送，预留异步扩展 */
}
