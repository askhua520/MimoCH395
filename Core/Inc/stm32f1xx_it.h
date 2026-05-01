/**
  ******************************************************************************
  * @file    : stm32f1xx_it.h
  ******************************************************************************
  */
#ifndef __STM32F1xx_IT_H
#define __STM32F1xx_IT_H

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void SysTick_Handler(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void USART1_IRQHandler(void);
void USART3_IRQHandler(void);
void UART4_IRQHandler(void);
void UART5_IRQHandler(void);

#endif
