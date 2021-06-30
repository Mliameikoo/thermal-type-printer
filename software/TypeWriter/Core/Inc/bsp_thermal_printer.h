#ifndef __BSP_THERMAL_PRINTER_H
#define __BSP_THERMAL_PRINTER_H

/* Includes ------------------------------------------------------------------*/
#include "config.h"
/* Private includes ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
// #define TRANSMIT_IMAGE_BUF_MAX_LENGTH 500 // 传输图像的有效字节长度
/* Exported types ------------------------------------------------------------*/
enum hostCommandDef
{
    _cmd_single_word_write = 0x01,
    _cmd_paragraph_write = 0x02, // unused
    _cmd_image_text_write = 0x03,
    _cmd_special_order = 0x04, // use for newlines
    _cmd_change_scale = 0x05,
    _cmd_change_offset = 0x06,
};
enum salveCommandDef
{
    _cmd_one_step_ack = 0x80,
    _cmd_temper_update,
};
enum textSetAlignDef
{
    _alignLeft = 0u,
    _alignMiddle,
    _alignRight
};

struct byteBufDef
{
    uint16_t length;
    uint8_t *val;
};
struct printerInfoDef
{
    uint8_t is_motor_idle;
    int16_t motor_run_cnt; // 4次为一行像素点的距离，所以一个16行的字符需要16*4次cnt才能完成
    uint8_t scale;         // 对字体横向放大scale倍
    uint8_t line_height;
    enum textSetAlignDef align;
    uint16_t xpos_pixel; //光标位置
    uint8_t xpos_char;   // 光标位置
    uint8_t xpos_char_max_nums;
    int16_t height_offset;
};

struct imageTransmitInfoDef
{
    uint8_t width;
    uint8_t height;
    struct byteBufDef buf;
};
/* Exported macro ------------------------------------------------------------*/
extern struct printerInfoDef printerInfo; // thermal printer infolist
/* Exported functions prototypes ---------------------------------------------*/

int8_t printer_init(void);

void motor_phases_update_loop(void);

int8_t printer_change_char_scale(uint8_t scale);
int8_t printer_write_single_char(uint8_t character);
int8_t printer_write_image_text(const struct imageTransmitInfoDef image);

int8_t printer_new_lines(uint16_t line_height, int16_t lines);
int8_t printer_write_text(uint8_t xpos_char, char *fmt, ...);

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
