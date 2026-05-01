/**
  ******************************************************************************
  * @file    : log_uart.h
  * @brief   : 日志串口系统 (注册制)
  ******************************************************************************
  */
#ifndef __LOG_UART_H
#define __LOG_UART_H

#include "main.h"
#include <stdint.h>

void Log_UART_Init(UART_HandleTypeDef *huart, GPIO_TypeDef *en_port, uint16_t en_pin);
void Log_Printf(const char *fmt, ...);
void Log_UART_Process(void);

#endif
