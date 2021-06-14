#ifndef __USER_APP_H
#define __USER_APP_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private includes ----------------------------------------------------------*/
#include "bsp_led.h"
#include "bsp_key.h"
#include "usbd_cdc_if.h"
#include "bsp_thermal_printer.h"
/* Private defines -----------------------------------------------------------*/
#define USBLINK_MESSAGE_INFO_MAX_LENGTH     20
/* Exported types ------------------------------------------------------------*/
struct usblinkMessageFormatDef{
    uint8_t info[USBLINK_MESSAGE_INFO_MAX_LENGTH];
    uint16_t info_length;
};
/* Exported variables --------------------------------------------------------*/
extern uint8_t isSysInitOver;
/* Exported functions prototypes ---------------------------------------------*/

uint8_t USER_SYS_Init(void);
void usb_analog_plug(void);
void usb_printf(char *fmt, ...);
void usb_printf_isr(char *fmt, ...);

void step(uint8_t direction);
void idle(void);

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
