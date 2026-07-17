#ifndef __MYXUNJI_H
#define __MYXUNJI_H

#include "ti_msp_dl_config.h"
#include "delay.h"
#include "tb6612.h"
#include "DL_OLED.h"


#define XUNJI_READ_PIN(port, pin) ((DL_GPIO_readPins((port), (pin)) != 0U) ? 1U : 0U)

#define XUNJI_HW_READ_0() XUNJI_READ_PIN(XUNJI_XJ1_PORT, XUNJI_XJ1_PIN)
#define XUNJI_HW_READ_1() XUNJI_READ_PIN(XUNJI_XJ2_PORT, XUNJI_XJ2_PIN)
#define XUNJI_HW_READ_2() XUNJI_READ_PIN(XUNJI_XJ3_PORT, XUNJI_XJ3_PIN)
#define XUNJI_HW_READ_3() XUNJI_READ_PIN(XUNJI_XJ4_PORT, XUNJI_XJ4_PIN)
#define XUNJI_HW_READ_4() XUNJI_READ_PIN(XUNJI_XJ5_PORT, XUNJI_XJ5_PIN)
#define XUNJI_HW_READ_5() XUNJI_READ_PIN(XUNJI_XJ6_PORT, XUNJI_XJ6_PIN)
#define XUNJI_HW_READ_6() XUNJI_READ_PIN(XUNJI_XJ7_PORT, XUNJI_XJ7_PIN)
#define XUNJI_HW_READ_7() XUNJI_READ_PIN(XUNJI_XJ8_PORT, XUNJI_XJ8_PIN)

uint8_t Xunji_Read_Hardware(uint8_t index);
void Xunji_Task(int16_t* left_speed, int16_t* right_speed);
void Xunji_Speed_Calc(int16_t* left_speed, int16_t* right_speed);
uint8_t Xunji_GetCornerCount(void);
void Xunji_ResetCornerCount(void);
void MyXunji_PidTrackStraight(uint8_t bits, int16_t base_speed, int16_t* left_speed, int16_t* right_speed);

#endif
