/**
  ******************************************************************************
  * @file    : main.c
  * @brief   : CubeMX生成主程序
  * @note    外设初始化代码，User_Init()在BEGIN/END区域调用
  ******************************************************************************
  */
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

/* 外设句柄 */
UART_HandleTypeDef huart1, huart3, huart4, huart5;
TIM_HandleTypeDef  htim2, htim3;

/* CubeMX函数声明 */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART4_Init(void);
static void MX_UART5_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN Includes */
#include "user.h"
/* USER CODE END Includes */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  User_Init();
  /* USER CODE END 2 */

  osKernelInitialize();
  MX_FREERTOS_Init();
  osKernelStart();

  while (1) {}
}

/* ======================== 时钟配置: 72MHz HSE=8MHz ======================== */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/* ======================== GPIO初始化 ======================== */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* 输出默认低电平 */
  HAL_GPIO_WritePin(CH_RSTI_GPIO_Port, CH_RSTI_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(UART1_EN_GPIO_Port, UART1_EN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(UART4_EN_GPIO_Port, UART4_EN_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(UART5_EN_GPIO_Port, UART5_EN_Pin, GPIO_PIN_RESET);

  /* CH395 输入引脚 */
  GPIO_InitStruct.Pin = CH_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(CH_INT_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = CH_RST_Pin;
  HAL_GPIO_Init(CH_RST_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = CH_ACT_Pin;
  HAL_GPIO_Init(CH_ACT_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = CH_LINK_Pin;
  HAL_GPIO_Init(CH_LINK_GPIO_Port, &GPIO_InitStruct);

  /* CH395 复位输出 */
  GPIO_InitStruct.Pin = CH_RSTI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CH_RSTI_GPIO_Port, &GPIO_InitStruct);

  /* RS485方向控制 */
  GPIO_InitStruct.Pin = UART1_EN_Pin;
  HAL_GPIO_Init(UART1_EN_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = UART4_EN_Pin;
  HAL_GPIO_Init(UART4_EN_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = UART5_EN_Pin;
  HAL_GPIO_Init(UART5_EN_GPIO_Port, &GPIO_InitStruct);
}

/* ======================== UART初始化 ======================== */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
}

static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart3);
}

static void MX_UART4_Init(void)
{
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart4);
}

static void MX_UART5_Init(void)
{
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart5);
}

/* ======================== TIM初始化 ======================== */
static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClk = {0};
  TIM_MasterConfigTypeDef sMst = {0};
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_Base_Init(&htim2);
  sClk.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClk);
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMst);
}

static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClk = {0};
  TIM_MasterConfigTypeDef sMst = {0};
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 99999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_Base_Init(&htim3);
  sClk.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim3, &sClk);
  HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMst);
}

void Error_Handler(void) { __disable_irq(); while(1) {} }
