/* Includes ------------------------------------------------------------------*/
#include "bsp_led.h"
/* Private define ------------------------------------------------------------*/
#define SHINING_LED_MAX_NUMS   4
#define SHINING_LED_ON(i)      HAL_GPIO_WritePin(shiningLed.ledx[i].port, shiningLed.ledx[i].pin, (shiningLed.ledx[i].valid)?GPIO_PIN_SET:GPIO_PIN_RESET)
#define SHINING_LED_OFF(i)     HAL_GPIO_WritePin(shiningLed.ledx[i].port, shiningLed.ledx[i].pin, (shiningLed.ledx[i].valid)?GPIO_PIN_RESET:GPIO_PIN_SET)
/* Private variables ---------------------------------------------------------*/
/**
  * @brief  sine's val with 128bit, unit byte
  */
const uint8_t sine_base[128] = {
 0x7F,0x85,0x8B,0x92,0x98,0x9E,0xA4,0xAA,0xB0,0xB6,0xBB,0xC1,0xC6,0xCB,0xD0,0xD5
,0xD9,0xDD,0xE2,0xE5,0xE9,0xEC,0xEF,0xF2,0xF5,0xF7,0xF9,0xFB,0xFC,0xFD,0xFE,0xFE
,0xFE,0xFE,0xFE,0xFD,0xFC,0xFB,0xF9,0xF7,0xF5,0xF2,0xEF,0xEC,0xE9,0xE5,0xE2,0xDD
,0xD9,0xD5,0xD0,0xCB,0xC6,0xC1,0xBB,0xB6,0xB0,0xAA,0xA4,0x9E,0x98,0x92,0x8B,0x85
,0x7F,0x79,0x73,0x6C,0x66,0x60,0x5A,0x54,0x4E,0x48,0x43,0x3D,0x38,0x33,0x2E,0x29
,0x25,0x21,0x1C,0x19,0x15,0x12,0x0F,0x0C,0x09,0x07,0x05,0x03,0x02,0x01,0x00,0x00
,0x00,0x00,0x00,0x01,0x02,0x03,0x05,0x07,0x09,0x0C,0x0F,0x12,0x15,0x19,0x1C,0x21
,0x25,0x29,0x2E,0x33,0x38,0x3D,0x43,0x48,0x4E,0x54,0x5A,0x60,0x66,0x6C,0x73,0x79
};
struct shiningLedPinDef
{
  struct pinDef{
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t valid;
  }ledx[SHINING_LED_MAX_NUMS];
  uint8_t lednums;
}shiningLed;
/* Private user code ---------------------------------------------------------*/
/**
  * @brief  呼吸灯获取PWM占空比函数
  * @param  ledx: 选择目标LED
  * @retval 返回当前LED应保持的亮度比，比例系数为xxx/255
  */
static uint8_t led_get_shining_duty(uint8_t ledx){
	static uint8_t pos[SHINING_LED_MAX_NUMS] = {0}; // 取余128
	if(++pos[ledx] > 0x7F){
		pos[ledx] = 0;
	}
	return sine_base[pos[ledx]];
}
/**
  * @brief  呼吸灯运行线程，放在循环中执行
  */
void led_shining_loop(void){
  static uint8_t step[SHINING_LED_MAX_NUMS] = {0};
  static uint8_t duty[SHINING_LED_MAX_NUMS] = {0};
  for(uint8_t i=0; i<shiningLed.lednums; i++){
    if(!step[i]){
      duty[i] = led_get_shining_duty(i);
    }
    if(step[i] < duty[i]){
      // low
      SHINING_LED_OFF(i);
    }
    else{
      // high
      SHINING_LED_ON(i);
    }
    step[i]++;
  }
}
/**
  * @brief  绑定呼吸灯的硬件引脚
  * @param  GPIOx: GPIOx
  * @param  GPIO_Pin: GPIO_PIN_x
  * @param  valid_value: 灯亮有效值
  * @retval 注册成功返回0，失败(注册满)返回1
  */
uint8_t led_shining_pin_register(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t valid_value){
  if(shiningLed.lednums >= SHINING_LED_MAX_NUMS){
    return 1; // ledx nums full
  }
  shiningLed.ledx[shiningLed.lednums].port = GPIOx;
  shiningLed.ledx[shiningLed.lednums].pin = GPIO_Pin;
  shiningLed.ledx[shiningLed.lednums].valid = valid_value;
  shiningLed.lednums++;
  return 0;
}
