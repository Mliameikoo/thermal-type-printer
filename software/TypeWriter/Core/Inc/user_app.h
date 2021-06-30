#ifndef __USER_APP_H
#define __USER_APP_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private includes ----------------------------------------------------------*/
#include "bsp_led.h"
#include "bsp_key.h"
#include "usbd_cdc_if.h"
#include "bsp_thermal_printer.h"
#include "adc.h"
/* Private defines -----------------------------------------------------------*/
//#define USBLINK_MESSAGE_INFO_MAX_LENGTH 20
#define ADC_RAW_DATA_DEPTH 16 // ADC采样深度
#define ADC_CHANNEL_NUMS 1    // ADC采样通道数
/* Exported types ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
extern uint8_t isSysInitOver;
extern uint16_t adc_raw_data[ADC_RAW_DATA_DEPTH][ADC_CHANNEL_NUMS];
/* Exported functions prototypes ---------------------------------------------*/

uint8_t USER_SYS_Init(void);
void usb_analog_plug(void);
uint16_t adc_get_filter_value(void);

void step(uint8_t direction);
void idle(void);

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
