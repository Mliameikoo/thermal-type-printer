#ifndef __BSP_LED_H
#define __BSP_LED_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private includes ----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

uint8_t led_shining_pin_register(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t valid_value);
void led_shining_loop(void);

/* Private defines -----------------------------------------------------------*/

#endif
