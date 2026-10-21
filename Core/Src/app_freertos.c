/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* USER CODE BEGIN Includes */
#include "app_tasks.h"
#include "emm_5v.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};

/* USER CODE BEGIN Variables2 */
osThreadId_t appTaskHandle;
const osThreadAttr_t appTask_attributes = {
  .name = "APP_TASK",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 4096
};

osThreadId_t imuTaskHandle;
const osThreadAttr_t imuTask_attributes = {
  .name = "IMU_TASK",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 1024
};
/* USER CODE END Variables2 */

/* Private function prototypes -----------------------------------------------*/
void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void);

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  appTaskHandle = osThreadNew(APP_Task, NULL, &appTask_attributes);
  imuTaskHandle = osThreadNew(IMU_Task, NULL, &imuTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

  /* 使能 6 个 EMM 电机（1~4 底盘，5/6 机械臂）。 */
  for (uint8_t id = 1U; id <= 6U; id++) {
    Emm_V5_En_Control(id, 1, 0);
  }

  for (;;) {
    osDelay(1000U);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Application */

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  (void)xTask;
  taskDISABLE_INTERRUPTS();
  for (;;) {
    __NOP();
  }
}

/* USER CODE END Application */
