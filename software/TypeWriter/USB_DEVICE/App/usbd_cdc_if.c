/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
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
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include <stdbool.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

struct usblinkMessageFormatDef usblinkMessage = {0};
struct hostComProtocolDef hostComProtocol = {0};

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

extern osMessageQueueId_t usblinkRxRequestQueueHandle;

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t *pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
    {
        CDC_Init_FS,
        CDC_DeInit_FS,
        CDC_Control_FS,
        CDC_Receive_FS};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);

  usblinkMessage.tx.info = UserTxBufferFS;
  usblinkMessage.tx.status_busy = false;
  usblinkMessage.rx.info = UserRxBufferFS;

  hostComProtocol.head = 0xF5;
  hostComProtocol.tail = 0x5F;

  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

  case CDC_SET_COMM_FEATURE:

    break;

  case CDC_GET_COMM_FEATURE:

    break;

  case CDC_CLEAR_COMM_FEATURE:

    break;

    /*******************************************************************************/
    /* Line Coding Structure                                                       */
    /*-----------------------------------------------------------------------------*/
    /* Offset | Field       | Size | Value  | Description                          */
    /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
    /* 4      | bCharFormat |   1  | Number | Stop bits                            */
    /*                                        0 - 1 Stop bit                       */
    /*                                        1 - 1.5 Stop bits                    */
    /*                                        2 - 2 Stop bits                      */
    /* 5      | bParityType |  1   | Number | Parity                               */
    /*                                        0 - None                             */
    /*                                        1 - Odd                              */
    /*                                        2 - Even                             */
    /*                                        3 - Mark                             */
    /*                                        4 - Space                            */
    /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
    /*******************************************************************************/
  case CDC_SET_LINE_CODING:

    break;

  case CDC_GET_LINE_CODING:

    break;

  case CDC_SET_CONTROL_LINE_STATE:

    break;

  case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t *Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);

  static enum usblinkRxStateDef rx_state = _rx_state_idle;
  static uint16_t length_of_valid_data_under_transmit = 0;

  uint8_t *pos = Buf;

  uint16_t remain_data_length = 0;
  uint8_t sum_check = 0;

  while (pos < Buf + (*Len))
  {
    switch (rx_state)
    {
    case _rx_state_idle:
      if (*pos == FRAME_HEAD_CODE)
      {
        rx_state = _rx_state_head_ok;
      }
      break;

    case _rx_state_head_ok:
      hostComProtocol.info.valid_length = *pos * 256;
      // length_of_valid_data_under_transmit = hostComProtocol.info.valid_length;
      rx_state = _rx_state_length_high8bits_ok;
      break;

    case _rx_state_length_high8bits_ok:
      hostComProtocol.info.valid_length += *pos;
      length_of_valid_data_under_transmit = hostComProtocol.info.valid_length;
      rx_state = _rx_state_length_low8bits_ok;
      break;

    case _rx_state_length_low8bits_ok:
      hostComProtocol.info.command = *pos;
      rx_state = _rx_state_command_ok;
      break;

    case _rx_state_command_ok:
      remain_data_length = (*Len) - (pos - Buf);

      if (length_of_valid_data_under_transmit > remain_data_length)
      {
        // 若当前接收缓存中剩余字符长度小于待传输字符，下次接收继续传输
        memcpy(hostComProtocol.info.valid_data + (hostComProtocol.info.valid_length - length_of_valid_data_under_transmit), pos, remain_data_length);
        pos += remain_data_length - 1;
        length_of_valid_data_under_transmit -= remain_data_length;
      }
      else
      {
        memcpy(hostComProtocol.info.valid_data + (hostComProtocol.info.valid_length - length_of_valid_data_under_transmit), pos, length_of_valid_data_under_transmit);
        pos += length_of_valid_data_under_transmit - 1;
        length_of_valid_data_under_transmit = 0;
      }

      if (!length_of_valid_data_under_transmit)
      {
        rx_state = _rx_state_data_ok;
      }
      break;

    case _rx_state_data_ok:
      sum_check = hostComProtocol.info.command;
      for (uint16_t i = 0; i < hostComProtocol.info.valid_length; i++)
      {
        sum_check += hostComProtocol.info.valid_data[i];
      }
      if (sum_check == *pos)
      {
        rx_state = _rx_state_sum_check_ok;
      }
      else
      {
        rx_state = _rx_state_idle;
      }
      break;

    case _rx_state_sum_check_ok:
      if (*pos == FRAME_TAIL_CODE)
      {
        // receive all down
        uint8_t request = 1;
        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(usblinkRxRequestQueueHandle, &request, &xHigherPriorityTaskWoken);
      }
      rx_state = _rx_state_idle;
      break;

    default:
      rx_state = _rx_state_idle;
      break;
    }
    pos++;
  }

  //   if (Buf[0] == hostComProtocol.head && (*Len) > 5 && Buf[(*Len) - 1] == hostComProtocol.tail)
  //   {
  //     vaild_length = Buf[1];
  //     if (*Len == vaild_length + 5)
  //     {
  //       // check sum
  //       sum_check = 0;
  //       for (uint8_t i = 2; i < vaild_length + 1 + 2; i++)
  //       {
  //         sum_check += Buf[i];
  //       }
  //       if (sum_check == Buf[(*Len) - 2])
  //       {
  //         // check sum pass
  //         // get data
  //         usb_printf("receive ok\r\n");

  //         hostComProtocol.info.command = Buf[2];
  //         hostComProtocol.info.valid_length = vaild_length;
  //         memcpy(hostComProtocol.info.valid_data, Buf + 3, hostComProtocol.info.valid_length);
  //         uint8_t request = 1;
  //         portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  //         xQueueSendFromISR(usblinkRxRequestQueueHandle, &request, &xHigherPriorityTaskWoken);

  //         goto TARGET_RECEIVE_OK;
  //       }
  //     }
  //   }
  //   usb_printf("receive error\r\n");
  // TARGET_RECEIVE_OK:
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0)
  {
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @brief  send frame
  * @retval 0: success, -1: usb-tx-cache full
  */
int8_t usb_send_frame(uint8_t cmd, uint16_t valid_length, uint8_t *valid_data_buf)
{
  if (usblinkMessage.tx.length + FRAME_STRUCTURE_CODE_LENGTH + valid_length > APP_TX_DATA_SIZE)
  {
    // 缓存已满，无法发送
    return -1;
  }
  uint8_t frame_head_list[4]; // head + length_h8bits + length_l8bits + cmd
  uint8_t frame_tail_list[2]; // sum_check + tail
  frame_head_list[0] = FRAME_HEAD_CODE;
  frame_head_list[1] = valid_length >> 8;
  frame_head_list[2] = valid_length & 0xFF;
  frame_head_list[3] = cmd;
  usb_send_string(4, frame_head_list);
  usb_send_string(valid_length, valid_data_buf);
  uint8_t sum_check = cmd;
  for (uint16_t i = 0; i < valid_length; i++)
  {
    sum_check += valid_data_buf[i];
  }
  frame_tail_list[0] = sum_check;
  frame_tail_list[1] = FRAME_TAIL_CODE;
  usb_send_string(2, frame_tail_list);
  return 0;
}

/**
  * @brief  usb-virtual-com's sendbuf with unblocking
  * @retval 0: success, 1: buf empty, -1: usb-tx-cache full
  */
int8_t usb_send_string(uint16_t length, uint8_t *buf)
{
  if (length == 0)
  {
    return 1;
  }
  if (usblinkMessage.tx.length + length <= APP_TX_DATA_SIZE)
  {
    memcpy((char *)(usblinkMessage.tx.info + usblinkMessage.tx.length), buf, length * sizeof(uint8_t));
    usblinkMessage.tx.length += length;
  }
  else
  {
    return -1;
  }
  return 0;
}

/**
  * @brief  usb-virtual-com's printf with unblocking
  * @retval 0: success
  */
int8_t usb_printf(char *fmt, ...)
{
  va_list argptr;
  va_start(argptr, fmt);
  usblinkMessage.tx.length += vsnprintf((char *)(usblinkMessage.tx.info + usblinkMessage.tx.length), APP_TX_DATA_SIZE - usblinkMessage.tx.length, fmt, argptr);
  va_end(argptr);
  return 0;
}

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
