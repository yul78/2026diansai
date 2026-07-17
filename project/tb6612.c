#include "tb6612.h"
#include "ti_msp_dl_config.h"

/*
    图形化配置说明
    .syscfg中TIMER_PWM命名为PWM_TB6612
*/

#define TB6612_PWM_PERIOD 4000U
#define TB6612_PWM_MAX    (TB6612_PWM_PERIOD - 1U)

TB6612_Motor TB6612_MotorA = {
    .channel = TB6612_MOTOR_CHANNEL_A,
    .target_speed = 0,
    .max_speed = TB6612_PWM_MAX,
    .direction = 1,
};

TB6612_Motor TB6612_MotorB = {
    .channel = TB6612_MOTOR_CHANNEL_B,
    .target_speed = 0,
    .max_speed = TB6612_PWM_MAX,
    .direction = 1,
};

/**
 * @brief 获取电机允许的最大速度限幅
 * @param motor 电机对象指针
 * @return 归一化后的最大速度值
 */
static uint16_t tb6612_get_max_speed(const TB6612_Motor *motor)
{
    if (motor->max_speed <= 0) {
        return 0;
    }

    if ((uint32_t)motor->max_speed > TB6612_PWM_MAX) {
        return TB6612_PWM_MAX;
    }

    return (uint16_t)motor->max_speed;
}

/**
 * @brief 按电机配置对目标速度做绝对值限幅
 * @param motor 电机对象指针
 * @param speed 目标速度
 * @return 限幅后的速度值
 */
static uint16_t tb6612_clamp_speed(const TB6612_Motor *motor, int32_t speed)
{
    uint16_t max_speed = tb6612_get_max_speed(motor);

    if (speed < 0) {
        speed = -speed;
    }

    if ((uint32_t)speed > max_speed) {
        return max_speed;
    }

    return (uint16_t)speed;
}

/**
 * @brief 计算输出到驱动器的实际速度方向
 * @param motor 电机对象指针
 * @return 带方向修正后的输出速度
 */
static int32_t tb6612_get_output_speed(const TB6612_Motor *motor)
{
    int8_t direction = (motor->direction >= 0) ? 1 : -1;

    return (int32_t)motor->target_speed * direction;
}

/**
 * @brief 获取指定通道对应的引脚与PWM通道配置
 * @param channel 电机通道
 * @param[in,out] in1Pin IN1引脚编号输出地址
 * @param[in,out] in2Pin IN2引脚编号输出地址
 * @param[in,out] ccIndex PWM比较通道索引输出地址
 */
static void tb6612_get_channel_config(TB6612_MotorChannel channel,
    uint32_t *in1Pin, uint32_t *in2Pin, uint16_t *ccIndex)
{
    if (channel == TB6612_MOTOR_CHANNEL_A) {
        *in1Pin = MOTOR_DIR_MOTOR_A_IN1_PIN;
        *in2Pin = MOTOR_DIR_MOTOR_A_IN2_PIN;
        *ccIndex = GPIO_PWM_TB6612_C0_IDX;
    } else {
        *in1Pin = MOTOR_DIR_MOTOR_B_IN1_PIN;
        *in2Pin = MOTOR_DIR_MOTOR_B_IN2_PIN;
        *ccIndex = GPIO_PWM_TB6612_C1_IDX;
    }
}

/**
 * @brief 将电机当前设定值输出到方向引脚和PWM比较寄存器
 * @param motor 电机对象指针
 */
static void tb6612_apply_motor(const TB6612_Motor *motor)
{
    uint32_t in1Pin;
    uint32_t in2Pin;
    uint16_t ccIndex;
    int32_t output_speed = tb6612_get_output_speed(motor);

    tb6612_get_channel_config(motor->channel, &in1Pin, &in2Pin, &ccIndex);

    if (output_speed > 0) {
        DL_GPIO_setPins(MOTOR_DIR_PORT, in1Pin);
        DL_GPIO_clearPins(MOTOR_DIR_PORT, in2Pin);
    } else if (output_speed < 0) {
        DL_GPIO_clearPins(MOTOR_DIR_PORT, in1Pin);
        DL_GPIO_setPins(MOTOR_DIR_PORT, in2Pin);
    } else {
        DL_GPIO_clearPins(MOTOR_DIR_PORT, in1Pin | in2Pin);
    }

    int32_t clamp_speed = tb6612_clamp_speed(motor, output_speed);
    DL_TimerG_setCaptureCompareValue(PWM_TB6612_INST,clamp_speed, ccIndex);
}

/**
 * @brief 初始化TB6612电机驱动
 * @note 会清空方向引脚并启动PWM计数器
 */
void TB6612_Init(void)
{
    DL_GPIO_clearPins(MOTOR_DIR_PORT,MOTOR_DIR_MOTOR_A_IN1_PIN | MOTOR_DIR_MOTOR_A_IN2_PIN | MOTOR_DIR_MOTOR_B_IN1_PIN | MOTOR_DIR_MOTOR_B_IN2_PIN);

    TB6612_SetMotor(&TB6612_MotorA, 0);
    TB6612_SetMotor(&TB6612_MotorB, 0);
    DL_TimerG_startCounter(PWM_TB6612_INST);
}

/**
 * @brief 设置指定电机的速度
 * @param motor 电机对象指针
 * @param speed 目标速度，正负表示方向
 */
void TB6612_SetMotor(TB6612_Motor *motor, int16_t speed)
{
    if (motor == 0) {
        return;
    }

    motor->target_speed = speed;
    tb6612_apply_motor(motor);
}

/**
 * @brief 设置A路电机速度
 * @param speed 目标速度，正负表示方向
 */
void TB6612_SetMotorA(int16_t speed)
{
    TB6612_SetMotor(&TB6612_MotorA, speed);
}

/**
 * @brief 设置B路电机速度
 * @param speed 目标速度，正负表示方向
 */
void TB6612_SetMotorB(int16_t speed)
{
    TB6612_SetMotor(&TB6612_MotorB, speed);
}

/**
 * @brief 同时设置A、B两路电机速度
 * @param motorA A路电机速度
 * @param motorB B路电机速度
 */
void TB6612_SetMotors(int16_t motorA, int16_t motorB)
{
    TB6612_SetMotor(&TB6612_MotorA, motorA);
    TB6612_SetMotor(&TB6612_MotorB, motorB);
}

/**
 * @brief 停止所有电机输出
 */
void TB6612_Stop(void)
{
    TB6612_SetMotors(0, 0);
}

