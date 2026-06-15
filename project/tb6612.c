#include "tb6612.h"

#include "ti_msp_dl_config.h"

#define TB6612_PWM_PERIOD 4000U
#define TB6612_PWM_MAX    (TB6612_PWM_PERIOD - 1U)

static uint16_t tb6612_clamp_speed(int16_t speed)
{
    int32_t magnitude = speed;

    if (magnitude < 0) {
        magnitude = -magnitude;
    }

    if ((uint32_t)magnitude > TB6612_PWM_MAX) {
        return TB6612_PWM_MAX;
    }

    return (uint16_t)magnitude;
}

static void tb6612_set_motor(uint32_t in1Pin, uint32_t in2Pin, uint16_t ccIndex, int16_t speed)
{
    if (speed > 0) {
        DL_GPIO_setPins(MOTOR_DIR_PORT, in1Pin);
        DL_GPIO_clearPins(MOTOR_DIR_PORT, in2Pin);
    } else if (speed < 0) {
        DL_GPIO_clearPins(MOTOR_DIR_PORT, in1Pin);
        DL_GPIO_setPins(MOTOR_DIR_PORT, in2Pin);
    } else {
        DL_GPIO_clearPins(MOTOR_DIR_PORT, in1Pin | in2Pin);
    }

    DL_TimerG_setCaptureCompareValue(PWM_0_INST, tb6612_clamp_speed(speed), ccIndex);
}

void TB6612_Init(void)
{
    DL_GPIO_clearPins(MOTOR_DIR_PORT,
        MOTOR_DIR_MOTOR_A_IN1_PIN | MOTOR_DIR_MOTOR_A_IN2_PIN |
        MOTOR_DIR_MOTOR_B_IN1_PIN | MOTOR_DIR_MOTOR_B_IN2_PIN);

    DL_TimerG_setCaptureCompareValue(PWM_0_INST, 0, GPIO_PWM_0_C0_IDX);
    DL_TimerG_setCaptureCompareValue(PWM_0_INST, 0, GPIO_PWM_0_C1_IDX);
    DL_TimerG_startCounter(PWM_0_INST);
}

void TB6612_SetMotorA(int16_t speed)
{
    tb6612_set_motor(MOTOR_DIR_MOTOR_A_IN1_PIN, MOTOR_DIR_MOTOR_A_IN2_PIN, GPIO_PWM_0_C0_IDX, speed);
}

void TB6612_SetMotorB(int16_t speed)
{
    tb6612_set_motor(MOTOR_DIR_MOTOR_B_IN1_PIN, MOTOR_DIR_MOTOR_B_IN2_PIN, GPIO_PWM_0_C1_IDX, speed);
}

void TB6612_SetMotors(int16_t motorA, int16_t motorB)
{
    TB6612_SetMotorA(motorA);
    TB6612_SetMotorB(motorB);
}

void TB6612_Stop(void)
{
    TB6612_SetMotors(0, 0);
}
