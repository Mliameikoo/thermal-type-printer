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
/* Private user code ---------------------------------------------------------*/
/**
  * @brief  user system initial
  * @retval if return 0 is normal, else occur error
  */
uint8_t USER_SYS_Init(void){

  uint8_t retval = 0;

  // register shining led
  led_shining_pin_register(USER_LED_GPIO_Port, USER_LED_Pin, 0);
  // tim4 irq init
  __HAL_RCC_TIM4_CLK_ENABLE();
  HAL_NVIC_SetPriority(TIM4_IRQn, 5, 3);
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  HAL_TIM_Base_Start_IT(&htim4);
  return retval;
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
  // xQueueSend()
  xQueueSend(usblinkSendQueueHandle, &usblinkMessage, 3);
}


