#ifndef __BSP_THERMAL_PRINTER_H
#define __BSP_THERMAL_PRINTER_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private includes ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
enum hostCommandDef
{
    _cmd_single_word_write = 0x01,
    _cmd_paragraph_write = 0x02
};
enum textSetAlignDef
{
    _alignLeft = 0u,
    _alignMiddle,
    _alignRight
};
struct printerInfoDef
{
    uint8_t is_motor_idle;
    int16_t motor_run_cnt; // 4次为一行像素点的距离，所以一个16行的字符需要16*4次cnt才能完成
    uint8_t scale;         // 对字体横向放大scale倍
    enum textSetAlignDef align;
    uint8_t xpos_char; // 光标位置
    uint8_t xpos_char_max_nums;
};
/* Exported macro ------------------------------------------------------------*/
extern struct printerInfoDef printerInfo; // thermal printer infolist
/* Exported functions prototypes ---------------------------------------------*/

int8_t printer_init(void);

void motor_phases_update_loop(void);

int8_t printer_change_char_scale(uint8_t scale);
int8_t printer_write_single_char(uint8_t character);

int8_t printer_new_lines(int16_t lines);
int8_t printer_write_text(uint8_t xpos_char, char *fmt, ...);

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
