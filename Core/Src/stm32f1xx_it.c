/**
  ******************************************************************************
  * @file    : stm32f1xx_it.c
  * @brief   : 中断处理
  ******************************************************************************
  */
#include "main.h"
#include "stm32f1xx_it.h"
#include "FreeRTOS.h"
#include "task.h"

extern TIM_HandleTypeDef htim2, htim3;
extern UART_HandleTypeDef huart1, huart3, huart4, huart5;

void NMI_Handler(void) {}
void HardFault_Handler(void) { while(1){} }
void MemManage_Handler(void) { while(1){} }
void BusFault_Handler(void) { while(1){} }
void UsageFault_Handler(void) { while(1){} }
void DebugMon_Handler(void) {}

void SysTick_Handler(void)
{
  HAL_IncTick();
#if (INCLUDE_xTaskGetSchedulerState == 1)
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    xPortSysTickHandler();
#endif
}

void TIM2_IRQHandler(void)  { HAL_TIM_IRQHandler(&htim2); }
void TIM3_IRQHandler(void)  { HAL_TIM_IRQHandler(&htim3); }
void USART1_IRQHandler(void) { HAL_UART_IRQHandler(&huart1); }
void USART3_IRQHandler(void) { HAL_UART_IRQHandler(&huart3); }
void UART4_IRQHandler(void)  { HAL_UART_IRQHandler(&huart4); }
void UART5_IRQHandler(void)  { HAL_UART_IRQHandler(&huart5); }
