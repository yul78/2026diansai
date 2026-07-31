#ifndef __MYXUNJI_H
#define __MYXUNJI_H

#include "ti_msp_dl_config.h"
#include "delay.h"
#include "DL_OLED.h"
#include "at8236.h"

#define XUNJI_READ_PIN(port, pin) ((DL_GPIO_readPins((port), (pin)) != 0U) ? 1U : 0U)

#define XUNJI_HW_READ_0() XUNJI_READ_PIN(XUNJI_XJ1_PORT, XUNJI_XJ1_PIN)
#define XUNJI_HW_READ_1() XUNJI_READ_PIN(XUNJI_XJ2_PORT, XUNJI_XJ2_PIN)
#define XUNJI_HW_READ_2() XUNJI_READ_PIN(XUNJI_XJ3_PORT, XUNJI_XJ3_PIN)
#define XUNJI_HW_READ_3() XUNJI_READ_PIN(XUNJI_XJ4_PORT, XUNJI_XJ4_PIN)
#define XUNJI_HW_READ_4() XUNJI_READ_PIN(XUNJI_XJ5_PORT, XUNJI_XJ5_PIN)
#define XUNJI_HW_READ_5() XUNJI_READ_PIN(XUNJI_XJ6_PORT, XUNJI_XJ6_PIN)
#define XUNJI_HW_READ_6() XUNJI_READ_PIN(XUNJI_XJ7_PORT, XUNJI_XJ7_PIN)
#define XUNJI_HW_READ_7() XUNJI_READ_PIN(XUNJI_XJ8_PORT, XUNJI_XJ8_PIN)

/*
 * 第二问循迹PID的调试数据。
 * 这些量由Xunji_Q2_Task()每10ms更新一次，主循环低频发送到串口。
 */
typedef struct {
    uint8_t sensor_bits;        /* 完整8路状态，bit0~bit7对应序号0~7 */
    uint8_t sensor_state;       /* bit0=XJ4(序号3)，bit1=XJ5(序号4) */
    int16_t error;              /* 仅由XJ4、XJ5得到，范围-200~200 */
    int32_t integral;           /* 误差积分，当前KI为0，先保留用于后续调参 */
    int16_t derivative;         /* 本次误差与上次误差之差 */
    int16_t p_term;             /* 比例项对速度差的贡献 */
    int16_t i_term;             /* 积分项对速度差的贡献 */
    int16_t d_term;             /* 微分项对速度差的贡献 */
    int16_t output;             /* 限幅后的最终速度修正量 */
    int16_t left_target;        /* 左轮目标速度 */
    int16_t right_target;       /* 右轮目标速度 */
} Xunji_Q2PidDebug;

extern Xunji_Q2PidDebug Xunji_Q2_Debug;

/* 第四问直线阶段的调试数据，由Xunji_Q4_Task()在主循环中更新。 */
typedef struct {
    uint8_t sensor_bits;       /* 完整8路状态，仅用于观察，不全部参与控制 */
    uint8_t middle_state;      /* bit0=XJ4，bit1=XJ5 */
    int16_t base_speed;        /* 软启动或软刹车后的当前基础速度 */
    int16_t correction;        /* XJ4、XJ5产生的方向修正量 */
    int16_t left_target;       /* 左轮速度闭环目标值 */
    int16_t right_target;      /* 右轮速度闭环目标值 */
} Xunji_Q4Debug;

extern Xunji_Q4Debug Xunji_Q4_Debug;

uint8_t Xunji_Read_Hardware(uint8_t index);
void Xunji_Task(int16_t* left_speed, int16_t* right_speed);
void Xunji_Speed_Calc(int16_t* left_speed, int16_t* right_speed);
uint8_t Xunji_GetCornerCount(void);
void Xunji_ResetCornerCount(void);
void MyXunji_PidTrackStraight(uint8_t bits, int16_t base_speed, int16_t* left_speed, int16_t* right_speed);
void Xunji_Q2_Reset(void);
uint8_t Xunji_Q2_StopLineDetected(void);
uint8_t Xunji_Q2_Task(int16_t* left_speed, int16_t* right_speed);
void Xunji_Q4_Reset(void);
void Xunji_Q4_RequestStop(void);
uint8_t Xunji_Q4_Task(
    uint32_t elapsed_ms, int16_t* left_speed, int16_t* right_speed);
/* 第五问软启动椭圆循迹PID。 */
void Xunji_Q5_Reset(void);
void Xunji_Q5_Task(
    uint32_t elapsed_ms, int16_t* left_speed, int16_t* right_speed);
/* 第六问软启动椭圆循迹PID。 */
void Xunji_Q6_Reset(void);
void Xunji_Q6_Task(
    uint32_t elapsed_ms, int16_t* left_speed, int16_t* right_speed);

#endif
