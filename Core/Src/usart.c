/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "stm32f0xx.h"
#include "stm32f0xx_hal_conf.h"

/* USER CODE BEGIN 0 */
#define EMBEDDED_CLI_IMPL
#include "tim.h"
#include <stdio.h>
#include "embedded_cli.h"

#define RxBuf_SIZE   1
uint8_t RxBuf[RxBuf_SIZE];
EmbeddedCli *cli;
static void cli_init(void);

/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  cli_init();
  printf("PWM(Motor) Driver Test v0.1\n");
  HAL_UART_Receive_IT(&huart2, RxBuf, 1);
  /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, USART_TX_Pin|USART_RX_Pin);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

int _write(int file, char *ptr, int len)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
  return len;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2) {
    embeddedCliReceiveChar(cli, RxBuf[0]);

    /* start the DMA again */
    HAL_UART_Receive_IT(&huart2, RxBuf, 1);
  }
}

static void Period(EmbeddedCli *cli, char *args, void *context) {
  printf("period:%d\r\n", atoi(args));
  MX_TIM1_Period_Set(atoi(args));
}

static void DeadTime(EmbeddedCli *cli, char *args, void *context) {
  printf("deadtime:%d\r\n", atoi(args));
  MX_TIM1_Deadtime_Set(atoi(args));
}

static void Duty(EmbeddedCli *cli, char *args, void *context) {
  printf("Duty Percent: %d\r\n", atoi(args));
  MX_TIM1_Duty_Set(atoi(args));
}

void writeChar(EmbeddedCli *embeddedCli, char c) {
  HAL_UART_Transmit(&huart2, &c, 1, HAL_MAX_DELAY);
}

static void cli_init(void)
{
  cli = embeddedCliNewDefault();

  struct CliCommandBinding onPeriodCmd = {
    "set_period",
    "set period val\n\tval from 1~65536, Unit about 1.6ms\n\t100: 6.67Khz; 200: 3.3Khz",
    false,
    NULL,
    Period
  };

  struct CliCommandBinding onDeadTimeCmd = {
    "set_deadtime",
    "Set deadtime val\n\tfrom 0 to ~ 255",
    false,
    NULL,
    DeadTime
  };

  struct CliCommandBinding onDutyCmd = {
    "set_duty",
    "Set duty_percent val\n\tfrom 0 to 99",
    false,
    NULL,
    Duty
  };

  embeddedCliAddBinding(cli, onPeriodCmd);
  embeddedCliAddBinding(cli, onDeadTimeCmd);
  embeddedCliAddBinding(cli, onDutyCmd);
  cli->writeChar = writeChar;
}
/* USER CODE END 1 */
