/**
  ******************************************************************************
  * @file    : main.h
  * @brief   : GPIO引脚定义 (CubeMX生成，不要修改)
  * @chip    : STM32F103RGT6
  ******************************************************************************
  */
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* CH395 控制引脚 */
#define CH_INT_Pin              GPIO_PIN_0
#define CH_INT_GPIO_Port        GPIOB
#define CH_RST_Pin              GPIO_PIN_1
#define CH_RST_GPIO_Port        GPIOB
#define CH_RSTI_Pin             GPIO_PIN_2
#define CH_RSTI_GPIO_Port       GPIOB
#define CH_ACT_Pin              GPIO_PIN_4
#define CH_ACT_GPIO_Port        GPIOC
#define CH_LINK_Pin             GPIO_PIN_5
#define CH_LINK_GPIO_Port       GPIOC

/* RS485 方向控制引脚 */
#define UART1_EN_Pin            GPIO_PIN_9
#define UART1_EN_GPIO_Port      GPIOA
#define UART4_EN_Pin            GPIO_PIN_15
#define UART4_EN_GPIO_Port      GPIOA
#define UART5_EN_Pin            GPIO_PIN_3
#define UART5_EN_GPIO_Port      GPIOB

void Error_Handler(void);

#ifdef __cplusplus
}
#endif
#endif /* __MAIN_H */
