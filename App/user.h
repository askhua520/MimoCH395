/**
  ******************************************************************************
  * @file    : user.h
  * @brief   : 用户应用主入口
  ******************************************************************************
  */
#ifndef __USER_H
#define __USER_H

#include "main.h"
#include "cmsis_os.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef  htim2;
extern TIM_HandleTypeDef  htim3;

void User_Init(void);

#endif
