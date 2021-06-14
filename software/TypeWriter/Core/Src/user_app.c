/* Includes ------------------------------------------------------------------*/
#include "user_app.h"
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include <stdarg.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os.h"
/* Private macro -------------------------------------------------------------*/
extern osMessageQueueId_t usblinkSendQueueHandle;
/* Private variables ---------------------------------------------------------*/
uint8_t isSysInitOver = false;
/* Private user code ---------------------------------------------------------*/
/**
  * @brief  user system initial
  * @retval if return 0 is normal, else occur error
  */
uint8_t USER_SYS_Init(void){
	uint8_t retval = 0;

	// reset usb state
	usb_analog_plug();

	// register shining led
	led_shining_pin_register(USER_LED_GPIO_Port, USER_LED_Pin, 0);

	// thermal printer init
	// printer_init();

	// tim4 irq init
	__HAL_RCC_TIM4_CLK_ENABLE();
	HAL_NVIC_SetPriority(TIM4_IRQn, 5, 3);
	HAL_NVIC_EnableIRQ(TIM4_IRQn);
	HAL_TIM_Base_Start_IT(&htim4);
	isSysInitOver = true;
	return retval;
}

/**
  * @brief  before the usb enumerated, run analog-plug to let pc know
  */
void usb_analog_plug(void){
  GPIO_InitTypeDef GPIO_InitStruct;
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  /* Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
  /* Configure GPIO pin : PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);                                            
  HAL_Delay(65);
  // 先把PA12拉低再拉高，利用D+模拟USB的拔插动作   
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
  HAL_Delay(65);
}

/**
  * @brief  usb-virtual-com's printf through rtos's queue 
  */
void usb_printf(char *fmt, ...){
	struct usblinkMessageFormatDef usblinkMessage;
	uint16_t send_cnt;
	va_list argptr;
	va_start(argptr, fmt);
	send_cnt = vsnprintf((char *)usblinkMessage.info, USBLINK_MESSAGE_INFO_MAX_LENGTH, fmt, argptr);
	usblinkMessage.info_length = send_cnt;
	va_end(argptr);
	// xQueueSendFromISR(usblinkSendQueueHandle, )
	xQueueSend(usblinkSendQueueHandle, &usblinkMessage, 3);
}

/**
  * @brief  usb-virtual-com's printf through rtos's queue from ISR
  */
void usb_printf_isr(char *fmt, ...){
	struct usblinkMessageFormatDef usblinkMessage;
  portBASE_TYPE xHigherPriorityTaskWoken;
	uint16_t send_cnt;
	va_list argptr;
	va_start(argptr, fmt);
	send_cnt = vsnprintf((char *)usblinkMessage.info, USBLINK_MESSAGE_INFO_MAX_LENGTH, fmt, argptr);
	usblinkMessage.info_length = send_cnt;
	va_end(argptr);
	xQueueSendFromISR(usblinkSendQueueHandle, &usblinkMessage, &xHigherPriorityTaskWoken);
	// xQueueSend(usblinkSendQueueHandle, &usblinkMessage, 3);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


