/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "user_app.h"
#include "queue.h"
#include "font_thermal_printer.h"
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
/* Definitions for peripheralTask */
osThreadId_t peripheralTaskHandle;
const osThreadAttr_t peripheralTask_attributes = {
  .name = "peripheralTask",
  .stack_size = 64 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for usblinkTxTask */
osThreadId_t usblinkTxTaskHandle;
const osThreadAttr_t usblinkTxTask_attributes = {
  .name = "usblinkTxTask",
  .stack_size = 64 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for usblinkRxTask */
osThreadId_t usblinkRxTaskHandle;
const osThreadAttr_t usblinkRxTask_attributes = {
  .name = "usblinkRxTask",
  .stack_size = 64 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for inoutDeviceTask */
osThreadId_t inoutDeviceTaskHandle;
const osThreadAttr_t inoutDeviceTask_attributes = {
  .name = "inoutDeviceTask",
  .stack_size = 300 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for usblinkSendQueue */
osMessageQueueId_t usblinkSendQueueHandle;
const osMessageQueueAttr_t usblinkSendQueue_attributes = {
  .name = "usblinkSendQueue"
};
/* Definitions for usblinkRecEvent */
osEventFlagsId_t usblinkRecEventHandle;
const osEventFlagsAttr_t usblinkRecEvent_attributes = {
  .name = "usblinkRecEvent"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void peripheralStartTask(void *argument);
void usblinkTxStartTask(void *argument);
void usblinkRxStartTask(void *argument);
void inoutDeviceStartTask(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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

  /* Create the queue(s) */
  /* creation of usblinkSendQueue */
  usblinkSendQueueHandle = osMessageQueueNew (5, sizeof(struct usblinkMessageFormatDef), &usblinkSendQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of peripheralTask */
  peripheralTaskHandle = osThreadNew(peripheralStartTask, NULL, &peripheralTask_attributes);

  /* creation of usblinkTxTask */
  usblinkTxTaskHandle = osThreadNew(usblinkTxStartTask, NULL, &usblinkTxTask_attributes);

  /* creation of usblinkRxTask */
  usblinkRxTaskHandle = osThreadNew(usblinkRxStartTask, NULL, &usblinkRxTask_attributes);

  /* creation of inoutDeviceTask */
  inoutDeviceTaskHandle = osThreadNew(inoutDeviceStartTask, NULL, &inoutDeviceTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of usblinkRecEvent */
  usblinkRecEventHandle = osEventFlagsNew(&usblinkRecEvent_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_peripheralStartTask */
/**
  * @brief  Function implementing the peripheralTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_peripheralStartTask */
void peripheralStartTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN peripheralStartTask */
  printer_init();
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END peripheralStartTask */
}

/* USER CODE BEGIN Header_usblinkTxStartTask */
/**
* @brief Function implementing the usblinkTxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_usblinkTxStartTask */
void usblinkTxStartTask(void *argument)
{
	/* USER CODE BEGIN usblinkTxStartTask */
	while(!isSysInitOver);
  struct usblinkMessageFormatDef usblinkMessage;
	/* Infinite loop */
	for(;;)
	{
		if(xQueueReceive(usblinkSendQueueHandle, &usblinkMessage, portMAX_DELAY) == pdTRUE){
			CDC_Transmit_FS(usblinkMessage.info, usblinkMessage.info_length);
		}
	}
  /* USER CODE END usblinkTxStartTask */
}

/* USER CODE BEGIN Header_usblinkRxStartTask */
/**
* @brief Function implementing the usblinkRxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_usblinkRxStartTask */
void usblinkRxStartTask(void *argument)
{
  /* USER CODE BEGIN usblinkRxStartTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END usblinkRxStartTask */
}

/* USER CODE BEGIN Header_inoutDeviceStartTask */
/**
* @brief Function implementing the inoutDeviceTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_inoutDeviceStartTask */
void inoutDeviceStartTask(void *argument)
{
	/* USER CODE BEGIN inoutDeviceStartTask */
	while(!isSysInitOver);

	/* Infinite loop */
	for(;;)
	{
		KEY_PinState val;
		val = key_scan_signal(0, HAL_GPIO_ReadPin(USER_KEY1_GPIO_Port, USER_KEY1_Pin));
		if(val == KEY_SIGNAL_RELEASE){
			HAL_GPIO_TogglePin(USER_LED_B_GPIO_Port, USER_LED_B_Pin);
			HAL_GPIO_TogglePin(PRN_POWER_GPIO_Port, PRN_POWER_Pin);
			usb_printf("hello1\r\n");
//      usb_printf("%d %d", *FONT_CHAR_RASTERS[0], *FONT_CHAR_RASTERS[1]);
		}
		val = key_scan_signal(1, HAL_GPIO_ReadPin(USER_KEY2_GPIO_Port, USER_KEY2_Pin));
    if(val == KEY_SIGNAL_RELEASE){
      uint8_t val = printer_write_text("hello world");
      usb_printf("write result: %d\r\n", val);
    }
		osDelay(5);
	}
	/* USER CODE END inoutDeviceStartTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
