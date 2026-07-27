#include "angle_ctrl.h"
#include "at8236.h"

/**
 * @brief 全局角度 PID 控制器实例。
 */
AngleCtrl_Controller AngleCtrl_PID = {0};

/**
 * @brief 将 64 位有符号数限制到指定的 32 位范围内。
 */
static int32_t angle_ctrl_clamp_i32(int64_t value,
    int32_t minimum, int32_t maximum)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return (int32_t)value;
}

/**
 * @brief 将浮点角度转换为控制器内部的定点角度单位。
 */
static int32_t angle_ctrl_deg_to_scaled(float angleDeg)
{
    if (angleDeg >= 0.0f) {
        return (int32_t)(angleDeg * (float)ANGLE_CTRL_ANGLE_SCALE + 0.5f);
    }

    return (int32_t)(angleDeg * (float)ANGLE_CTRL_ANGLE_SCALE - 0.5f);
}

/**
 * @brief 将 yaw 角误差归一化到正负 180 度以内。
 */
static int32_t angle_ctrl_normalize_error(int32_t error)
{
#if ANGLE_CTRL_ENABLE_WRAP
    const int32_t fullCircle = 360L * ANGLE_CTRL_ANGLE_SCALE;
    const int32_t halfCircle = 180L * ANGLE_CTRL_ANGLE_SCALE;

    while (error > halfCircle) {
        error -= fullCircle;
    }
    while (error < -halfCircle) {
        error += fullCircle;
    }
#endif

    return error;
}

/**
 * @brief 清除 PID 运行状态，但不修改目标角度。
 */
static void angle_ctrl_clear_state(AngleCtrl_Controller *controller)
{
    controller->measured_angle = 0;
    controller->error = 0;
    controller->integral = 0;
    controller->previous_error = 0;
    controller->output = 0;
    controller->initialized = 0U;
}

void AngleCtrl_Init(void)
{
    AngleCtrl_PID.target_angle = 0;
    angle_ctrl_clear_state(&AngleCtrl_PID);
}

void AngleCtrl_InitWithTarget(float targetAngleDeg)
{
    AngleCtrl_PID.target_angle = angle_ctrl_deg_to_scaled(targetAngleDeg);
    angle_ctrl_clear_state(&AngleCtrl_PID);
}

void AngleCtrl_Reset(void)
{
    angle_ctrl_clear_state(&AngleCtrl_PID);
}

void AngleCtrl_SetTarget(float targetAngleDeg)
{
    int32_t target = angle_ctrl_deg_to_scaled(targetAngleDeg);

    if (target != AngleCtrl_PID.target_angle) {
        angle_ctrl_clear_state(&AngleCtrl_PID);
    }

    AngleCtrl_PID.target_angle = target;
}

void AngleCtrl_LockCurrent(float measuredAngleDeg)
{
    AngleCtrl_InitWithTarget(measuredAngleDeg);
}

int16_t AngleCtrl_Update(float measuredAngleDeg)
{
    int32_t derivative;
    int64_t output;

    AngleCtrl_PID.measured_angle = angle_ctrl_deg_to_scaled(measuredAngleDeg);
    AngleCtrl_PID.error = angle_ctrl_normalize_error(
        AngleCtrl_PID.target_angle - AngleCtrl_PID.measured_angle);

    if ((AngleCtrl_PID.error <= ANGLE_CTRL_DEADBAND) &&
        (AngleCtrl_PID.error >= -ANGLE_CTRL_DEADBAND)) {
        AngleCtrl_PID.error = 0;
    }

    if (ANGLE_CTRL_KI != 0L) {
        AngleCtrl_PID.integral = angle_ctrl_clamp_i32(
            (int64_t)AngleCtrl_PID.integral + AngleCtrl_PID.error,
            -ANGLE_CTRL_INTEGRAL_LIMIT, ANGLE_CTRL_INTEGRAL_LIMIT);
    } else {
        AngleCtrl_PID.integral = 0;
    }

    if (AngleCtrl_PID.initialized != 0U) {
        derivative = AngleCtrl_PID.error - AngleCtrl_PID.previous_error;
    } else {
        derivative = 0;
        AngleCtrl_PID.initialized = 1U;
    }
    AngleCtrl_PID.previous_error = AngleCtrl_PID.error;

    output = ((int64_t)ANGLE_CTRL_KP * AngleCtrl_PID.error) +
        ((int64_t)ANGLE_CTRL_KI * AngleCtrl_PID.integral) +
        ((int64_t)ANGLE_CTRL_KD * derivative);
    output /= (ANGLE_CTRL_GAIN_SCALE * ANGLE_CTRL_ANGLE_SCALE);

    output = angle_ctrl_clamp_i32(output,
        -ANGLE_CTRL_OUTPUT_LIMIT, ANGLE_CTRL_OUTPUT_LIMIT);

    AngleCtrl_PID.output = (int16_t)output;
    return AngleCtrl_PID.output;
}

void AngleCtrl_GetMotorTargets(int32_t baseSpeed, float measuredAngleDeg,
    int32_t *motorA, int32_t *motorB)
{
    int32_t correction = (int32_t)AngleCtrl_Update(measuredAngleDeg) *
        ANGLE_CTRL_STEER_POLARITY;

    if (motorA != 0) {
        *motorA = baseSpeed + correction;
    }
    if (motorB != 0) {
        *motorB = baseSpeed - correction;
    }
}

void AngleCtrl_UpdateMotors(int32_t baseSpeed, float measuredAngleDeg)
{
    int32_t motorA;
    int32_t motorB;

    AngleCtrl_GetMotorTargets(baseSpeed, measuredAngleDeg, &motorA, &motorB);
    AT8236_PID_SetTargets(motorA, motorB);
}

void AngleCtrl_Stop(void)
{
    AngleCtrl_Reset();
    AT8236_PID_SetTargets(0, 0);
}
