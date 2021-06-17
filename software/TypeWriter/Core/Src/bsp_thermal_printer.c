/* Includes ------------------------------------------------------------------*/
#include "bsp_thermal_printer.h"
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "user_app.h"
#include "font_thermal_printer.h"
#include <stdarg.h>
/* Private define ------------------------------------------------------------*/
#define PRINTER_ROW_WIDTH 384 // 2'inch thermal-printer-head, 384 points / 48 byte
// Default font size is too small to read.
// Set scale to 2 for normal use. Set to 3 for demonstration.
#define PRINTER_TEXT_SCALE 2

#define PRINTER_POWER_ON() HAL_GPIO_WritePin(PRN_POWER_GPIO_Port, PRN_POWER_Pin, GPIO_PIN_SET)
#define PRINTER_POWER_OFF() HAL_GPIO_WritePin(PRN_POWER_GPIO_Port, PRN_POWER_Pin, GPIO_PIN_RESET)
#define PRINTER_HEATING_ON() HAL_GPIO_WritePin(PRN_STROBE_GPIO_Port, PRN_STROBE_Pin, GPIO_PIN_SET)
#define PRINTER_HEATING_OFF() HAL_GPIO_WritePin(PRN_STROBE_GPIO_Port, PRN_STROBE_Pin, GPIO_PIN_RESET)
#define PRINTER_LATCH_ON() HAL_GPIO_WritePin(PRN_LATCH_GPIO_Port, PRN_LATCH_Pin, GPIO_PIN_SET)
#define PRINTER_LATCH_OFF() HAL_GPIO_WritePin(PRN_LATCH_GPIO_Port, PRN_LATCH_Pin, GPIO_PIN_RESET)
#define PRINTER_PAPER_EXIST_READ() HAL_GPIO_ReadPin(PRN_SNS_GPIO_Port, PRN_SNS_Pin) // 缺纸输出高电平

#define my_min(a, b) ((a > b) ? b : a)

//#define DEBUG_DISABLE_HEATING   // 调试模式下禁止热敏打印机加热

/* Private variables ---------------------------------------------------------*/
const uint8_t MOTOR_PHASES[][4] = {
    {1, 0, 0, 0},
    {1, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 1, 0, 1},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {1, 0, 1, 0},
    {0, 0, 0, 0} /* idle */
};
// Backspace glyph (dotted block)
static uint8_t font_backspace_char_raster[FONT_CHAR_HEIGHT] = {
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, /*backspace*/
};
struct printerInfoDef printerInfo;
/* Private macro -------------------------------------------------------------*/

static int8_t text_limit_length(const uint8_t start_nums, const uint8_t write_nums, uint8_t *single_line_char_nums);
static int8_t text_handle_illegal_character(uint8_t *write_buf, const uint8_t single_line_char_nums, uint8_t *(*rasters_pointer));
static int8_t text_set_alignment(const enum textSetAlignDef align, const uint8_t single_line_char_nums, uint16_t *start_x_pos);
static int8_t text_print_pixel_row(uint8_t *(*rasters_pointer), const uint8_t single_line_char_nums, const uint16_t start_x_pos);

static void printer_spi_send_data(uint8_t *data);
static int8_t printer_feed_paper_with_lines(int16_t lines_nums);
static int8_t printer_print_single_pixel_line(uint8_t *data);

static int8_t motor_set_idle(void);
static void motor_set_phases(const uint8_t phases[4]);
static void motor_single_step(uint8_t direction);

/* Private user code ---------------------------------------------------------*/

/**
  * @brief  thermal printer init
  */
int8_t printer_init(void)
{
  int8_t retval = 0;
  // init param
  printerInfo.scale = PRINTER_TEXT_SCALE;
  printerInfo.xpos_char_max_nums = PRINTER_ROW_WIDTH / 8 / printerInfo.scale;
  // init device
  PRINTER_HEATING_OFF();
  PRINTER_LATCH_OFF();
  uint8_t empty_array[PRINTER_ROW_WIDTH / 8] = {0};
  HAL_SPI_Transmit(&hspi1, empty_array, PRINTER_ROW_WIDTH / 8, 5);
  PRINTER_LATCH_ON();
  motor_set_idle();
  return retval;
}

/**
  * @brief  update motor-phases in tim4_irq.  
  *         run interval as 500us/0.5ms 
  */
void motor_phases_update_loop(void)
{
  if (printerInfo.motor_run_cnt != 0)
  {
    motor_single_step(printerInfo.motor_run_cnt > 0);
    printerInfo.motor_run_cnt += (printerInfo.motor_run_cnt > 0) ? (-1) : (1);
  }
}

int8_t printer_change_char_scale(uint8_t scale)
{
  if (scale > 3)
  {
    return -1;
  }
  printerInfo.scale = scale;
  printerInfo.xpos_char_max_nums = PRINTER_ROW_WIDTH / 8 / printerInfo.scale;
  return 0;
}

int8_t printer_write_single_char(uint8_t character)
{
  int8_t retval;
  uint8_t write_buf[1] = {0};
  uint8_t write_nums = 1;

  PRINTER_POWER_ON();

  write_buf[0] = character;

  uint8_t next_lines = 0;
  if (printerInfo.xpos_char >= printerInfo.xpos_char_max_nums)
  {
    next_lines = printerInfo.xpos_char / printerInfo.xpos_char_max_nums;
    printerInfo.xpos_char = printerInfo.xpos_char % printerInfo.xpos_char_max_nums;
  }
  printer_feed_paper_with_lines(-64 + 32 * next_lines);
  motor_set_idle();

  uint8_t single_line_char_nums = 0;
  retval = text_limit_length(printerInfo.xpos_char, write_nums, &single_line_char_nums);
  if (retval < 0)
  {
    return retval;
  }

  uint8_t *rasters_pointer[PRINTER_ROW_WIDTH / 8] = {0};
  retval = text_handle_illegal_character(write_buf, single_line_char_nums, rasters_pointer);

  uint16_t start_x_pos = printerInfo.xpos_char * 8 * printerInfo.scale;
  retval |= text_set_alignment(printerInfo.align, single_line_char_nums, &start_x_pos);

  retval |= text_print_pixel_row(rasters_pointer, single_line_char_nums, start_x_pos);

  printer_feed_paper_with_lines(32);
  motor_set_idle();

  // motor_set_idle();
  printerInfo.xpos_char++;

  PRINTER_POWER_OFF();

  return retval;
}

/**
  * @brief  切换新行
  * @param  lines: 正负行数
  * @retval 0:success, 1:缺纸
  */
int8_t printer_new_lines(int16_t lines)
{
  int8_t retval;
  PRINTER_POWER_ON();
  retval = printer_feed_paper_with_lines(lines * 32);
  motor_set_idle();
  printerInfo.xpos_char = 0;
  PRINTER_POWER_OFF();
  return retval;
}

/**
  * @brief  格式化打印字符串
  * @param  xpos_char: x轴起点(字符为单位)，范围0 ~ PRINTER_ROW_WIDTH / 8 / scale - 1
  * @param  fmt: 格式化输入，最长输入长度PRINTER_ROW_WIDTH/8=48
  * @retval 0:success; -1:缺纸, -2: 长度过大，无法写入; 1:长度过大，已截去多余部分, 2:存在非法字符，已替换
  */
int8_t printer_write_text(uint8_t xpos_char, char *fmt, ...)
{
  int8_t retval;
  uint8_t write_buf[PRINTER_ROW_WIDTH / 8];
  uint8_t write_nums;

  if (xpos_char >= PRINTER_ROW_WIDTH / 8 / printerInfo.scale)
  {
    return -2;
  }

  va_list argptr;
  va_start(argptr, fmt);
  write_nums = vsnprintf((char *)write_buf, sizeof(write_buf) / sizeof(uint8_t), fmt, argptr);
  va_end(argptr);

  uint8_t single_line_char_nums = 0;
  retval = text_limit_length(xpos_char, write_nums, &single_line_char_nums);
  if (retval < 0)
  {
    return retval;
  }

  uint8_t *rasters_pointer[PRINTER_ROW_WIDTH / 8] = {0};
  retval = text_handle_illegal_character(write_buf, single_line_char_nums, rasters_pointer);

  uint16_t start_x_pos = xpos_char * 8 * printerInfo.scale;
  retval |= text_set_alignment(printerInfo.align, single_line_char_nums, &start_x_pos);

  retval |= text_print_pixel_row(rasters_pointer, single_line_char_nums, start_x_pos);

  motor_set_idle();

  return retval;
}

/* Static User code ----------------------------------------------------------*/

/**
  * @brief  文本处理：切除长字符串多余的部分
  * @param  write_nums: 从零点开始预计延伸长度
  * @param  single_line_char_nums: 本次最终需要的对象字符串长度
  * @retval 0:success, -1：写入长度为0, -2：起始位置过大，无法写入, 1：长度过大，已截去多余部分
  */
static int8_t text_limit_length(const uint8_t start_nums, const uint8_t write_nums, uint8_t *single_line_char_nums)
{
  int8_t retval = 0;
  if (!write_nums)
  {
    return -1;
  }
  if (start_nums == printerInfo.xpos_char_max_nums)
  {
    return -2;
  }
  if (start_nums + write_nums > printerInfo.xpos_char_max_nums)
  {
    retval = 1;
    *single_line_char_nums = printerInfo.xpos_char_max_nums - start_nums;
  }
  else
  {
    *single_line_char_nums = write_nums;
  }
  return retval;
}

/**
  * @brief  文本处理：根据输入字符串文本进行字库指针映射，并处理非法符号
  * @param  write_buf: 原始输入的字符串
  * @param  single_line_char_nums: 本次文本处理最终的对象字符串长度
  * @param  rasters_pointer: 字符索引(指针数组)，存放每个对象字符的字库映射
  * @retval 0:success, 1:存在非法字符，已进行替换处理
  */
static int8_t text_handle_illegal_character(uint8_t *write_buf, const uint8_t single_line_char_nums, uint8_t *(*rasters_pointer))
{
  // convert to raster image indexes.
  // unknown characters are replaced with '?'
  int8_t retval = 0;
  uint8_t val;
  for (uint8_t i = 0; i < single_line_char_nums; i++)
  {
    val = write_buf[i];
    if (val == FONT_BACKSPACE_CHAR_CODE)
    {
      rasters_pointer[i] = font_backspace_char_raster;
      continue;
    }
    val -= FONT_CHAR_BEGIN_POS;
    if (val >= 95)
    {
      val = '?' - FONT_CHAR_BEGIN_POS;
      retval = 1;
    }
    rasters_pointer[i] = FONT_CHAR_RASTERS[val];
  }
  return retval;
}

/**
  * @brief  文本处理：对文本进行缩进排版处理
  * @param  align: 原始输入的字符串
  * @param  single_line_char_nums: 本次文本处理最终的对象字符串长度
  * @param  start_x_pos: 本次覆写在X轴上开始的位置(像素点为单位)
  * @retval 0:success
  */
static int8_t text_set_alignment(const enum textSetAlignDef align, const uint8_t single_line_char_nums, uint16_t *start_x_pos)
{
  uint16_t padding_width;
  padding_width = PRINTER_ROW_WIDTH - single_line_char_nums * FONT_CHAR_WIDTH * printerInfo.scale;
  switch (align)
  {
  case _alignMiddle:
    *start_x_pos += padding_width / 2;
    break;
  case _alignRight:
    *start_x_pos += padding_width;
    break;
  default:
    break;
  }
  return 0;
}

/**
  * @brief  文本处理：将处理好的文本进行比例缩放并打印输出
  * @param  rasters_pointer: 字符索引(指针数组)，存放每个对象字符的字库映射
  * @param  single_line_char_nums: 本次文本处理最终的对象字符串长度
  * @param  start_x_pos: 本次覆写在X轴上开始的位置(像素点为单位)
  * @retval 0:success, -1:缺纸
  */
static int8_t text_print_pixel_row(uint8_t *(*rasters_pointer), const uint8_t single_line_char_nums, const uint16_t start_x_pos)
{
  int8_t retval = 0;
  uint8_t row_bit_data[PRINTER_ROW_WIDTH / 8] = {0};
  uint8_t row_copy_bit_data[PRINTER_ROW_WIDTH / 8] = {0};
  uint16_t xpos;
  uint8_t raster_line_data;
  for (uint8_t offset = 0; offset < FONT_CHAR_HEIGHT; offset++)
  {
    memset(row_bit_data, 0, PRINTER_ROW_WIDTH / 8 * sizeof(uint8_t));
    xpos = start_x_pos;
    for (uint8_t index = 0; index < single_line_char_nums; index++)
    {
      raster_line_data = *(uint8_t *)(rasters_pointer[index] + offset);
      for (uint8_t i = 0; i < 8; i++)
      {
        if ((raster_line_data & (1 << (7 - i))) == 0)
        {
          xpos += printerInfo.scale;
          continue;
        }
        // 在x轴上进行scale比例缩放
        for (uint8_t j = 0; j < printerInfo.scale; j++)
        {
          row_bit_data[xpos / 8] |= 1 << (7 - xpos % 8);
          xpos++;
        }
      }
    }
    // 逐行的打印每一排像素点(单个字符为16行的8bit数据)
    for (uint8_t i = 0; i < printerInfo.scale; i++)
    {
      memcpy(row_copy_bit_data, row_bit_data, PRINTER_ROW_WIDTH / 8 * sizeof(uint8_t));
      retval = printer_print_single_pixel_line(row_copy_bit_data);
      if (retval)
      {
        break;
      }
    }
  }
  return retval;
}

/**
  * @brief  spi驱动发送一整行像素点的数据
  */
static void printer_spi_send_data(uint8_t *data)
{
  PRINTER_LATCH_OFF();
  HAL_SPI_Transmit(&hspi1, data, PRINTER_ROW_WIDTH / 8, 5);
  PRINTER_LATCH_ON();
}

/**
  * @brief  运转电机送纸（阻塞态）
  * @param  lines_nums: 行数
  * @retval 缺纸返回-1，无误返回0
  */
static int8_t printer_feed_paper_with_lines(int16_t lines_nums)
{
  if (PRINTER_PAPER_EXIST_READ())
  {
    return -1;
  }
  printerInfo.motor_run_cnt = 4 * lines_nums; // 4: stepsPerLine
  while (printerInfo.motor_run_cnt)
    ; // wait until finished
  return 0;
}

/**
  * @brief  驱动打印机打印一行像素点
  * @param  data: pixel data, format as 1bpp data(1 is black)
  * @retval 缺纸返回-1，无误返回0
  */
static int8_t printer_print_single_pixel_line(uint8_t *data)
{
  // Print one line of pixels, 1bpp data format (1 is black).
  // After calling this, `data` will be cleared to 0's
  int8_t retval = 0;
  if (PRINTER_PAPER_EXIST_READ())
  {
    return -1;
  }
  PRINTER_HEATING_OFF(); // stop heating
  printer_spi_send_data(data);
#ifndef DEBUG_DISABLE_HEATING
  PRINTER_HEATING_ON();
#endif
  retval = printer_feed_paper_with_lines(1);
  PRINTER_HEATING_OFF(); // stop heating
  return retval;
}

/**
  * @brief  reset the motor to idle 
  * @retval 0:状态切换成功, 1:无需切换
  */
static int8_t motor_set_idle(void)
{
  if (printerInfo.is_motor_idle)
  {
    return 1;
  }
  motor_set_phases(MOTOR_PHASES[8]);
  printerInfo.is_motor_idle = true;
  return 0;
}

/**
  * @brief  motor step
  * @param  direction: 1:forward, 0:backward
  */
static void motor_single_step(uint8_t direction)
{
  static uint8_t current_step = 7;
  printerInfo.is_motor_idle = false;
  current_step = (current_step + (direction ? 1 : -1) + 8) % 8;
  motor_set_phases(MOTOR_PHASES[current_step]);
}

/**
  * @brief  电机底层驱动函数
  * @param  phases: 二相半步驱动
  */
static void motor_set_phases(const uint8_t phases[4])
{
  HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, phases[0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, phases[1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MOTOR_IN3_GPIO_Port, MOTOR_IN3_Pin, phases[2] ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MOTOR_IN4_GPIO_Port, MOTOR_IN4_Pin, phases[3] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
