/* Includes ------------------------------------------------------------------*/
#include "bsp_thermal_printer.h"
/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "user_app.h"
#include "font_thermal_printer.h"
/* Private define ------------------------------------------------------------*/
#define PRINTER_ROW_WIDTH       384 // 2'inch thermal-printer-head, 384 points / 48 byte
#define PRINTER_HEATING_ON()    HAL_GPIO_WritePin(PRN_STROBE_GPIO_Port, PRN_STROBE_Pin, GPIO_PIN_SET)
#define PRINTER_HEATING_OFF()   HAL_GPIO_WritePin(PRN_STROBE_GPIO_Port, PRN_STROBE_Pin, GPIO_PIN_RESET)
#define PRINTER_LATCH_ON()      HAL_GPIO_WritePin(PRN_LATCH_GPIO_Port, PRN_LATCH_Pin, GPIO_PIN_SET)
#define PRINTER_LATCH_OFF()     HAL_GPIO_WritePin(PRN_LATCH_GPIO_Port, PRN_LATCH_Pin, GPIO_PIN_RESET)
#define PRINTER_PAPER_EXIST_READ()  HAL_GPIO_ReadPin(PRN_SNS_GPIO_Port, PRN_SNS_Pin)
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
struct printerInfoDef printerInfo;
/* Private macro -------------------------------------------------------------*/
static void motor_set_phases(const uint8_t phases[4]);
static void motor_single_step(uint8_t direction);


/**
  * @brief  thermal printer init
  */
uint8_t printer_init(void){
    uint8_t retval = 0;
    PRINTER_HEATING_OFF();
    PRINTER_LATCH_OFF();
    uint8_t empty_array[PRINTER_ROW_WIDTH/8];
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
    if(printerInfo.cnt != 0){
        motor_single_step(printerInfo.cnt > 0);
        printerInfo.cnt += (printerInfo.cnt>0)?-1:1;
    }
}
/**
  * @brief  reset the motor to idle 
  */
void motor_set_idle(void){
    if(printerInfo.isMotorIDLE){
        return;
    }
    motor_set_phases(MOTOR_PHASES[8]);
    printerInfo.isMotorIDLE = true;
}


//void printer_send_data(){
//  HAL_SPI_Transmit
//}



/* Static user code ---------------------------------------------------------*/
/**
  * @brief  motor step
  * @param  direction: 1:forward, 0:backward
  */
static void motor_single_step(uint8_t direction){
    static uint8_t current_step = 7;
    printerInfo.isMotorIDLE = false;
    current_step = (current_step + (direction?1:-1) + 8) % 8;
    usb_printf_isr("current_step:%d\r\n", current_step);
    motor_set_phases(MOTOR_PHASES[current_step]);
}
/**
  * @brief  
  * @param  phases: 
  */
static void motor_set_phases(const uint8_t phases[4]){
    // usb_printf_isr("set phases:%d %d %d %d\r\n", phases[0],phases[1],phases[2],phases[3]);
    HAL_GPIO_WritePin(MOTOR_IN1_GPIO_Port, MOTOR_IN1_Pin, phases[0]?GPIO_PIN_SET:GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, phases[1]?GPIO_PIN_SET:GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN3_GPIO_Port, MOTOR_IN3_Pin, phases[2]?GPIO_PIN_SET:GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR_IN4_GPIO_Port, MOTOR_IN4_Pin, phases[3]?GPIO_PIN_SET:GPIO_PIN_RESET);
}
