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

#define PRINTER_POWER_ON() HAL_GPIO_WritePin(PRN_POWER_GPIO_Port, PRN_POWER_Pin, GPIO_PIN_SET)
#define PRINTER_POWER_OFF() HAL_GPIO_WritePin(PRN_POWER_GPIO_Port, PRN_POWER_Pin, GPIO_PIN_RESET)

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
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for usblinkTxTask */
osThreadId_t usblinkTxTaskHandle;
const osThreadAttr_t usblinkTxTask_attributes = {
    .name = "usblinkTxTask",
    .stack_size = 64 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for usblinkRxTask */
osThreadId_t usblinkRxTaskHandle;
const osThreadAttr_t usblinkRxTask_attributes = {
    .name = "usblinkRxTask",
    .stack_size = 300 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for inoutDeviceTask */
osThreadId_t inoutDeviceTaskHandle;
const osThreadAttr_t inoutDeviceTask_attributes = {
    .name = "inoutDeviceTask",
    .stack_size = 64 * 4,
    .priority = (osPriority_t)osPriorityBelowNormal,
};
/* Definitions for usblinkRxRequestQueue */
osMessageQueueId_t usblinkRxRequestQueueHandle;
const osMessageQueueAttr_t usblinkRxRequestQueue_attributes = {
    .name = "usblinkRxRequestQueue"};
/* Definitions for usblinkRecEvent */
osEventFlagsId_t usblinkRecEventHandle;
const osEventFlagsAttr_t usblinkRecEvent_attributes = {
    .name = "usblinkRecEvent"};

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
void MX_FREERTOS_Init(void)
{
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
  /* creation of usblinkRxRequestQueue */
  usblinkRxRequestQueueHandle = osMessageQueueNew(1, sizeof(uint8_t), &usblinkRxRequestQueue_attributes);

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
  PRINTER_POWER_ON();
  osDelay(20);
  printer_new_lines(printerInfo.line_height, 2);
  /* Infinite loop */
  for (;;)
  {

    osDelay(1000);
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
  while (!isSysInitOver)
    ;
  /* Infinite loop */
  for (;;)
  {
    if (usblinkMessage.tx.length)
    {
      uint8_t retval;
      retval = CDC_Transmit_FS(usblinkMessage.tx.info, usblinkMessage.tx.length);
      if (retval == USBD_OK)
      {
        usblinkMessage.tx.length = 0;
      }
    }
    osDelay(1);
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

  // static uint16_t pos_char = 0;

  /* Infinite loop */
  for (;;)
  {
    uint8_t request;

    if (xQueueReceive(usblinkRxRequestQueueHandle, &request, 1000))
    {
      switch (hostComProtocol.info.command)
      {
      case _cmd_single_word_write:
      {
        for (uint8_t i = 0; i < hostComProtocol.info.valid_length; i++)
        {
          if (hostComProtocol.info.valid_data[i] == '\n')
          {
            printer_new_lines(printerInfo.line_height, 1);
          }
          else
          {
            printer_write_single_char(hostComProtocol.info.valid_data[i]);
          }
        }
        usb_printf("1");
        break;
      }

      case _cmd_image_text_write:
      {
        struct imageTransmitInfoDef image_text;
        image_text.width = hostComProtocol.info.valid_data[0];
        image_text.height = hostComProtocol.info.valid_data[1];
        image_text.buf.length = hostComProtocol.info.valid_data[2] * 256 + hostComProtocol.info.valid_data[3];
        image_text.buf.val = hostComProtocol.info.valid_data + 4;
        printer_write_image_text(image_text);
        // osDelay(50);
        usb_printf("1");
        break;
      }

      case _cmd_special_order:
      {
        uint8_t height;
        height = hostComProtocol.info.valid_data[1]; // 只取高度信息
        printer_new_lines(height, 1);
        usb_printf("1");
        break;
      }

      case _cmd_change_offset:
      {

        break;
      }
      }
    }
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
  while (!isSysInitOver)
    ;

  /* Infinite loop */
  for (;;)
  {
    KEY_PinState val;
    val = key_scan_signal(0, HAL_GPIO_ReadPin(USER_KEY1_GPIO_Port, USER_KEY1_Pin));
    if (val == KEY_SIGNAL_RELEASE)
    {
      HAL_GPIO_TogglePin(USER_LED_B_GPIO_Port, USER_LED_B_Pin);
      HAL_GPIO_TogglePin(PRN_POWER_GPIO_Port, PRN_POWER_Pin);
      // usb_printf("%d %d", *FONT_CHAR_RASTERS[0], *FONT_CHAR_RASTERS[1]);
      usb_printf("%.1lf\r\n", adc_raw_data[5][5] * 3.3 / 4096);
    }
    val = key_scan_signal(1, HAL_GPIO_ReadPin(USER_KEY2_GPIO_Port, USER_KEY2_Pin));
    //    if (val == KEY_SIGNAL_RELEASE)
    //    {
    //      printer_new_lines(printerInfo.line_height, 1);
    //    }
    if (keyPress.valid[1])
    {
      printer_new_lines(printerInfo.line_height, 1);
    }
    osDelay(5);
  }
  /* USER CODE END inoutDeviceStartTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
