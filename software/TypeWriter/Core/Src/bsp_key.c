/* Includes ------------------------------------------------------------------*/
#include "bsp_key.h"
/* Private define ------------------------------------------------------------*/
#define KEY_VALID_LEVEL					0u		 // 按键有效电平(1/0)
#define KEY_PRESS_ELiSHAKING_TIME       20u      // 20ms按键消抖时间(const value)
#define KEY_PRESS_ELiSHAKING_INTERVAL   5u       // 扫描间隔时间，单位ms
#define KEY_PRESS_ELiSHAKING_NUMS       (KEY_PRESS_ELiSHAKING_TIME/KEY_PRESS_ELiSHAKING_INTERVAL)   //(const value)
/* Private variables ---------------------------------------------------------*/
struct keyPressDef keyPress = {0};
/* Private user code ---------------------------------------------------------*/
/**
  * @brief  非阻塞扫描按键信号，运行所需间隔：KEY_PRESS_ELiSHAKING_INTERVAL(ms)
  * @param  ch: 选择KEY通道
  * @param  current_value: 当前读取的按键电平(原始1/0值)
  * @retval 返回即时边沿信号
  */
KEY_PinState key_scan_signal(uint8_t ch, uint8_t current_value){
    static uint8_t record_elishaking_time[KEY_NUMS] = {0};
    KEY_PinState retval = KEY_SIGNAL_IDLE;
    if(ch > KEY_NUMS-1){
        return KEY_SIGNAL_IDLE;
    }
    if( ! keyPress.valid[ch] ){
        // press not valid yet
        if( KEY_VALID_LEVEL?(current_value):(!current_value) ){
            if(record_elishaking_time[ch] >= KEY_PRESS_ELiSHAKING_NUMS){
                keyPress.valid[ch] = true;
                record_elishaking_time[ch] = 0; // ready for release elishake
                retval = KEY_SIGNAL_PRESS;
            }
            else{
                record_elishaking_time[ch]++;
            }
        }
        else{
            record_elishaking_time[ch] = 0;
        }
    }
    else{
        // press is already valid
        if( KEY_VALID_LEVEL?(current_value):(!current_value) ){
            record_elishaking_time[ch] = 0;
        }
        else{
            if(record_elishaking_time[ch] >= KEY_PRESS_ELiSHAKING_NUMS){
                keyPress.valid[ch] = false;
                record_elishaking_time[ch] = 0; // ready for press elishake
                retval = KEY_SIGNAL_RELEASE;
            }
            else{
                record_elishaking_time[ch]++;
            }
        }
    }
    return retval;
}
