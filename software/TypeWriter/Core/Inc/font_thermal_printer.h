#ifndef __FONT_THERMAL_PRINTER_H
#define __FONT_THERMAL_PRINTER_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private defines -----------------------------------------------------------*/
// This is a fixed-width font
#define FONT_CHAR_WIDTH             8
#define FONT_CHAR_HEIGHT            16
#define FONT_BACKSPACE_CHAR_CODE    0x08
#define FONT_CHAR_BEGIN_POS         32
/* Exported variables --------------------------------------------------------*/
//extern uint8_t FONT_BACKSPACE_CHAR_RASTER[FONT_CHAR_HEIGHT];
extern uint8_t FONT_CHAR_RASTERS[][FONT_CHAR_HEIGHT];

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
