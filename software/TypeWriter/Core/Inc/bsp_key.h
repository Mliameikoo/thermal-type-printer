/**
  ******************************************************************************
  * File Name          : bsp_key.h
  * Description        : Header for button driver
  * Author             : Jadechen
  ******************************************************************************
  * @attention
  *
  * The button driver uses non-blocking signal reading, no delay function is 
  * required, and only needs to be called cyclically in a fixed cycle. The 
  * driver realizes signal detection such as long press, short press, press 
  * and release of the button. The main function is <key_scan_signal>, 
  * which returns the key edge signal and updates the 
  * <struct keyPressDef keyPress> parameter. 
  *
  ******************************************************************************
  */
#ifndef __BSP_KEY_H
#define __BSP_KEY_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private includes ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
#define KEY_NUMS        2       // 按键数量
/* Exported types ------------------------------------------------------------*/
typedef enum
{
    KEY_SIGNAL_IDLE = 0u,       // 信号：按键空闲
    KEY_SIGNAL_PRESS = 1u,      // 信号：按键按下（脉冲边沿）
    KEY_SIGNAL_RELEASE = 2u     // 信号：按键释放（脉冲边沿）
} KEY_PinState;
struct keyPressDef{
    uint8_t valid[KEY_NUMS];    // 按键有效信号值
};
extern struct keyPressDef keyPress;
/* Exported functions prototypes ---------------------------------------------*/

KEY_PinState key_scan_signal(uint8_t ch, uint8_t current_value);

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
