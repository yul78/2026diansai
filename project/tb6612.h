#ifndef TB6612_H
#define TB6612_H

#include <stdint.h>

/**
 * @brief TB6612 电机通道枚举
 */
typedef enum {
    TB6612_MOTOR_CHANNEL_A = 0,
    TB6612_MOTOR_CHANNEL_B
} TB6612_MotorChannel;

/**
 * @brief TB6612 电机控制结构体
 */
typedef struct {
    TB6612_MotorChannel channel; /**< 电机硬件通道（A或B） */
    int16_t target_speed;        /**< 当前设定的目标速度,可正可负 */
    int16_t max_speed;           /**< 允许的最大速度限幅 */
    int8_t direction;            /**< 软件方向修正系数：1 保持默认，-1 调换正反转方向 */
} TB6612_Motor;

extern TB6612_Motor TB6612_MotorA;
extern TB6612_Motor TB6612_MotorB;

void TB6612_Init(void);
void TB6612_SetMotor(TB6612_Motor *motor, int16_t speed);
void TB6612_SetMotorA(int16_t speed);
void TB6612_SetMotorB(int16_t speed);
void TB6612_SetMotors(int16_t motorA, int16_t motorB);
void TB6612_Stop(void);

#endif
