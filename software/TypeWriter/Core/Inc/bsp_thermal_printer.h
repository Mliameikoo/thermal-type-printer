#ifndef __BSP_THERMAL_PRINTER_H
#define __BSP_THERMAL_PRINTER_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private includes ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
struct printerInfoDef{
    uint8_t isMotorIDLE;
    int8_t cnt; // 4 cnt as a line
};
/* Exported macro ------------------------------------------------------------*/
extern struct printerInfoDef printerInfo; // thermal printer infolist
/* Exported functions prototypes ---------------------------------------------*/

uint8_t printer_init(void);
void motor_phases_update_loop(void);
void motor_set_idle(void);


#endif
