/* Includes ------------------------------------------------------------------*/
#include "bsp_thermal_printer.h"
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "user_app.h"
#include "font_thermal_printer.h"
#include <stdarg.h>
/* Private define ------------------------------------------------------------*/
#define PRINTER_ROW_WIDTH       384 // 2'inch thermal-printer-head, 384 points / 48 byte
// Default font size is too small to read.
// Set scale to 2 for normal use. Set to 3 for demonstration.
#define PRINTER_TEXT_SCALE      2

#define PRINTER_HEATING_ON()    HAL_GPIO_WritePin(PRN_STROBE_GPIO_Port, PRN_STROBE_Pin, GPIO_PIN_SET)
#define PRINTER_HEATING_OFF()   HAL_GPIO_WritePin(PRN_STROBE_GPIO_Port, PRN_STROBE_Pin, GPIO_PIN_RESET)
#define PRINTER_LATCH_ON()      HAL_GPIO_WritePin(PRN_LATCH_GPIO_Port, PRN_LATCH_Pin, GPIO_PIN_SET)
#define PRINTER_LATCH_OFF()     HAL_GPIO_WritePin(PRN_LATCH_GPIO_Port, PRN_LATCH_Pin, GPIO_PIN_RESET)
#define PRINTER_PAPER_EXIST_READ()  HAL_GPIO_ReadPin(PRN_SNS_GPIO_Port, PRN_SNS_Pin)    // 缺纸输出高电平

#define my_min(a, b)            ( (a>b) ? b:a )

//#define DEBUG_DISABLE_HEATING   // 调试模式下禁止热敏打印机加热
/* Private typedef -----------------------------------------------------------*/

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

static void printer_send_data(uint8_t *data);
static uint8_t printer_print_single_pixel_line(uint8_t* data);
static void motor_set_phases(const uint8_t phases[4]);
static void motor_single_step(uint8_t direction);

/* Private user code ---------------------------------------------------------*/
/**
  * @brief  thermal printer init
  */
uint8_t printer_init(void){
    uint8_t retval = 0;
    // init param
    printerInfo.scale = PRINTER_TEXT_SCALE;
    // init device
    PRINTER_HEATING_OFF();
    PRINTER_LATCH_OFF();
    uint8_t empty_array[PRINTER_ROW_WIDTH/8] = {0};
    HAL_SPI_Transmit(&hspi1, empty_array, PRINTER_ROW_WIDTH/8, 5);
    PRINTER_LATCH_ON();
    motor_set_idle();
    return retval;
}
/**
  * @brief  update motor-phases in tim4_irq.  
  *         run interval as 500us/0.5ms 
  */
void motor_phases_update_loop(void){
    if(printerInfo.motor_run_cnt != 0){
        motor_single_step(printerInfo.motor_run_cnt > 0);
        printerInfo.motor_run_cnt += (printerInfo.motor_run_cnt>0)?(-1):(1);
    }
}
/**
  * @brief  reset the motor to idle 
  */
void motor_set_idle(void){
    if(printerInfo.is_motor_idle){
        return;
    }
    motor_set_phases(MOTOR_PHASES[8]);
    printerInfo.is_motor_idle = true;
}
/**
  * @brief  运转电机送纸（阻塞态）
  * @param  lines_nums: 行数
  * @retval 缺纸返回1，无误返回0
  */
uint8_t printer_feed_paper_with_lines(uint8_t lines_nums){
  if(PRINTER_PAPER_EXIST_READ()){
    return 1;
  }
  printerInfo.motor_run_cnt = 4 * lines_nums; // 4: stepsPerLine
  while(printerInfo.motor_run_cnt); // wait until finished
  return 0;
}
/**
  * @brief  驱动打印机打印文本数据
  * @param  fmt: 自适应文本模型
  * @retval 缺纸返回1，无误返回0
  */
uint8_t printer_write_text(char *fmt, ...){
  uint8_t retval = 0;
  // format the writing messages
  uint8_t write_buf[20] = {0};
  uint16_t write_nums = 0;
  uint8_t *rasters[24] = {0};
  uint16_t i,j,x,y, line_char_nums=0;
  uint8_t c = 0;
  
  uint8_t raster = 0;
  uint8_t rowBitData[PRINTER_ROW_WIDTH / 8] = {0};
  uint8_t rowCopy[PRINTER_ROW_WIDTH / 8] = {0};

  va_list argptr;
  va_start(argptr, fmt);
  write_nums = vsnprintf((char *)write_buf, 20, fmt, argptr);
  va_end(argptr);

  // cut long text
  // 暂时只切除长段
  line_char_nums = PRINTER_ROW_WIDTH / FONT_CHAR_WIDTH / printerInfo.scale;
  line_char_nums = my_min(line_char_nums, write_nums);
  if (line_char_nums < 1) {
    return 2;
  }
  usb_printf("line_nums: %d\r\n", line_char_nums);

  // convert to raster image indexes.
  // unknown characters are replaced with '?'
  for (i = 0; i < line_char_nums; i++) {
    c = write_buf[i];
    if (c == ' ') {
      rasters[i] = font_backspace_char_raster;
      continue;
    }
    c -= FONT_CHAR_BEGIN_POS;
    if (c >= 95) {
      c = '?' - FONT_CHAR_BEGIN_POS;
    }
    rasters[i] = FONT_CHAR_RASTERS[c]; // 地址赋值
//    usb_printf("raster[%d]:%d\r\n", i, *rasters[i]);
  }
  usb_printf("convert raster\r\n");

  // alignment
  uint16_t startX = 0;
  uint16_t paddingX = PRINTER_ROW_WIDTH - line_char_nums * FONT_CHAR_WIDTH * printerInfo.scale;
  switch (printerInfo.align) {
    case _alignMiddle:
      startX = paddingX / 2;
      break;
    case _alignRight:
      startX = paddingX;
      break;
    default:
      startX = 0;
      break;
  }
  usb_printf("align down\r\n");

  // rasterize row by row
  
  for (y = 0; y < FONT_CHAR_HEIGHT; y++) {
    memset(rowBitData, 0, PRINTER_ROW_WIDTH / 8);

    x = startX;
    raster = 0;
    for (uint8_t chIndex = 0; chIndex < line_char_nums; chIndex++) {
      // simply copy and place images, 1bpp format
      raster = rasters[chIndex][y];
      for (i = 0; i < 8; i++) {
        if ((raster & (1 << (7 - i))) == 0) {
          x += printerInfo.scale;
          continue;
        }
        // scale in X direction
        for (j = 0; j < printerInfo.scale; j++) {
          uint8_t b = 0;
          b = rowBitData[x / 8];
          b |= (1 << (7 - x % 8));
          rowBitData[x / 8] |= (1 << (7 - x % 8));
          x++;
        }
      }
    }

    // print multiply times to scale in Y direction
    
    for (i = 0; i < printerInfo.scale; i++) {
      memcpy(rowCopy, rowBitData, PRINTER_ROW_WIDTH / 8);
      retval = printer_print_single_pixel_line(rowCopy);
      if(retval){
        break;
      }
    }
  }
  return retval;
}
/* Static user code ----------------------------------------------------------*/

//static void text_limit_length(void){
//  
//}








/**
  * @brief  spi驱动发送数据
  */
static void printer_send_data(uint8_t *data){
  PRINTER_LATCH_OFF();
  HAL_SPI_Transmit(&hspi1, data, PRINTER_ROW_WIDTH/8, 5);
  PRINTER_LATCH_ON();
}
// Print one line of pixels, 1bpp data format (1 is black).
// After calling this, `data` will be cleared to 0's
/**
  * @brief  驱动打印机打印一行像素点
  * @param  data: pixel data, format as 1bpp data(1 is black)
  * @retval 缺纸返回1，无误返回0
  */
static uint8_t printer_print_single_pixel_line(uint8_t* data){
  uint8_t retval = 0;
  if(PRINTER_PAPER_EXIST_READ()){
    return 1;
  }
  PRINTER_HEATING_OFF(); // stop heating
  printer_send_data(data);
  #ifndef DEBUG_DISABLE_HEATING
  PRINTER_HEATING_ON();
  #endif
  retval = printer_feed_paper_with_lines(1);
  PRINTER_HEATING_OFF(); // stop heating
  return retval;
}
/**
  * @brief  motor step
  * @param  direction: 1:forward, 0:backward
  */
static void motor_single_step(uint8_t direction){
    static uint8_t current_step = 7;
    printerInfo.is_motor_idle = false;
    current_step = (current_step + (direction?1:-1) + 8) % 8;
    // usb_printf_isr("current_step:%d\r\n", current_step);
    motor_set_phases(MOTOR_PHASES[current_step]);
}
/**
  * @brief  电机底层驱动函数
  * @param  phases: 二相半步驱动
  */
static void motor_set_phases(const uint8_t phases[4]){
    // usb_printf_isr("set phases:%d %d %d %d\r\n", phases[0],phases[1],phases[2],phases[3]);
    HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, phases[0]?GPIO_PIN_SET:GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, phases[1]?GPIO_PIN_SET:GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN3_GPIO_Port, MOTOR_IN3_Pin, phases[2]?GPIO_PIN_SET:GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN4_GPIO_Port, MOTOR_IN4_Pin, phases[3]?GPIO_PIN_SET:GPIO_PIN_RESET);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
