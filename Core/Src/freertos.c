/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "message_handle.h"
#include "gui_poll_handle.h"
#include "myLcd.h"
#include "touch.h"
#include "GUI.h"
#include <stdint.h>
#include "mygui_api.h"
#include "dev_handle.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId MessageHandleHandle;
osThreadId GuiPollHandleHandle;
osThreadId DevHandleHandle;
osThreadId DevUrgentHandleHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void MessageHandle_task(void const * argument);
void GuiPollHandle_task(void const * argument);
void DevHandle_task(void const * argument);
void DevUrgentHandle_task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
  /* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of MessageHandle */
  osThreadDef(MessageHandle, MessageHandle_task, osPriorityAboveNormal, 0, 256);
  MessageHandleHandle = osThreadCreate(osThread(MessageHandle), NULL);

  /* definition and creation of GuiPollHandle */
  osThreadDef(GuiPollHandle, GuiPollHandle_task, osPriorityIdle, 0, 2048);
  GuiPollHandleHandle = osThreadCreate(osThread(GuiPollHandle), NULL);

  /* definition and creation of DevHandle */
  osThreadDef(DevHandle, DevHandle_task, osPriorityBelowNormal, 0, 256);
  DevHandleHandle = osThreadCreate(osThread(DevHandle), NULL);

  /* definition and creation of DevUrgentHandle */
  osThreadDef(DevUrgentHandle, DevUrgentHandle_task, osPriorityHigh, 0, 128);
  DevUrgentHandleHandle = osThreadCreate(osThread(DevUrgentHandle), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_MessageHandle_task */
/**
  * @brief  Function implementing the MessageHandle thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_MessageHandle_task */
void MessageHandle_task(void const * argument)
{
  /* USER CODE BEGIN MessageHandle_task */
  message_handle(MessageHandleHandle);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END MessageHandle_task */
}

/* USER CODE BEGIN Header_GuiPollHandle_task */
/**
* @brief Function implementing the GuiPollHandle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_GuiPollHandle_task */
void GuiPollHandle_task(void const * argument)
{
  /* USER CODE BEGIN GuiPollHandle_task */
	gui_poll_handle(GuiPollHandleHandle);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END GuiPollHandle_task */
}

/* USER CODE BEGIN Header_DevHandle_task */
/**
* @brief Function implementing the DevHandle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_DevHandle_task */
void DevHandle_task(void const * argument)
{
  /* USER CODE BEGIN DevHandle_task */
  dev_handle();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END DevHandle_task */
}

/* USER CODE BEGIN Header_DevUrgentHandle_task */
/**
* @brief Function implementing the DevUrgentHandle thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_DevUrgentHandle_task */
void DevUrgentHandle_task(void const * argument)
{
  /* USER CODE BEGIN DevUrgentHandle_task */
	dev_urgent_handle();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END DevUrgentHandle_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
