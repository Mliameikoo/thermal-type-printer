#ifndef __BSP_THERMAL_PRINTER_H
#define __BSP_THERMAL_PRINTER_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private includes ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
enum textSetAlignDef
{
    _alignLeft = 0u,
    _alignMiddle,
    _alignRight
};
struct printerInfoDef
{
    uint8_t is_motor_idle;
    int8_t motor_run_cnt; // 4次为一行像素点的距离，所以一个16行的字符需要16*4次cnt才能完成
    uint8_t scale;        // 对字体横向放大scale倍
    enum textSetAlignDef align;
};
/* Exported macro ------------------------------------------------------------*/
extern struct printerInfoDef printerInfo; // thermal printer infolist
/* Exported functions prototypes ---------------------------------------------*/

uint8_t printer_init(void);
void motor_phases_update_loop(void);
void motor_set_idle(void);

uint8_t printer_feed_paper_with_lines(uint8_t lines_nums);
uint8_t printer_write_text(char *fmt, ...);

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
