#ifndef __BSP_THERMAL_PRINTER_H
#define __BSP_THERMAL_PRINTER_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private includes ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
struct printerInfoDef{
    uint8_t is_motor_idle;
    int8_t motor_run_cnt; // 4 cnt as a line
    uint8_t scale;
    enum{
        _alignLeft = 0u,
        _alignMiddle,
        _alignRight
    }align;
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
