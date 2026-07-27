#ifndef ANGLE_CTRL_H
#define ANGLE_CTRL_H

#include <stdint.h>

/**
 * @file angle_ctrl.h
 * @brief 差速车角度闭环控制器。
 *
 * 角度环是外环，不直接控制 PWM。
 * 它根据目标角度和当前角度计算一个带符号的转向修正量，然后与基础前进速度混合：
 *
 *   motorA_target = baseSpeed + correction
 *   motorB_target = baseSpeed - correction
 *
 * 混合后的左右电机目标速度用于 AT8236_PID_SetTargets() 速度闭环。
 */

/**
 * @brief 角度定点缩放系数。
 *
 * 对外接口使用“度”为单位；控制器内部使用 0.1 度单位。
 * 例如 12.3 度会被保存为 123。
 */
#define ANGLE_CTRL_ANGLE_SCALE       10L

/**
 * @brief PID 参数定点缩放系数。
 *
 * 实际参数 = 宏定义参数 / ANGLE_CTRL_GAIN_SCALE。
 */
#define ANGLE_CTRL_GAIN_SCALE        100L

/** @brief 比例系数，实际 Kp = ANGLE_CTRL_KP / 100。 */
#define ANGLE_CTRL_KP                45L

/** @brief 积分系数，实际 Ki = ANGLE_CTRL_KI / 100。 */
#define ANGLE_CTRL_KI                0L

/** @brief 微分系数，实际 Kd = ANGLE_CTRL_KD / 100。 */
#define ANGLE_CTRL_KD                150L

/** @brief 积分限幅，单位为累计的 0.1 度误差。 */
#define ANGLE_CTRL_INTEGRAL_LIMIT    4000L

/** @brief 转向修正输出限幅，单位与速度闭环目标一致。 */
#define ANGLE_CTRL_OUTPUT_LIMIT      7L

/** @brief 角度死区，单位为 0.1 度；5 表示正负 0.5 度。 */
#define ANGLE_CTRL_DEADBAND          15L

/** @brief 推荐调用周期；如果周期改变，需要重新调 PID 参数。 */
#define ANGLE_CTRL_UPDATE_PERIOD_MS  10U

/**
 * @brief 转向极性。
 *
 * 正输出表示电机 A 目标速度增大、电机 B 目标速度减小。
 * 如果车头偏离目标后越修越偏，把这个宏改成 -1。
 */
#define ANGLE_CTRL_STEER_POLARITY    -1L

/**
 * @brief 是否启用 yaw 角最短路径误差计算。
 *
 * 用 yaw 做航向控制时保持为 1。
 * 如果以后用于 pitch 或 roll 控制，通常应改为 0。
 */
#define ANGLE_CTRL_ENABLE_WRAP       1

/**
 * @brief 角度 PID 控制器运行状态。
 */
typedef struct {
    int32_t target_angle;    /**< 目标角度，单位为 0.1 度。 */
    int32_t measured_angle;  /**< 当前测量角度，单位为 0.1 度。 */
    int32_t error;           /**< 当前角度误差，单位为 0.1 度。 */
    int32_t integral;        /**< 积分项累计值。 */
    int32_t previous_error;  /**< 上一次误差，用于计算微分项。 */
    int16_t output;          /**< 带符号转向修正输出。 */
    uint8_t initialized;     /**< 微分项是否已经完成首次采样。 */
} AngleCtrl_Controller;

extern AngleCtrl_Controller AngleCtrl_PID;

/**
 * @brief 初始化角度控制器，目标角度默认为 0 度。
 */
void AngleCtrl_Init(void);

/**
 * @brief 初始化角度控制器，并设置指定目标角度。
 *
 * @param targetAngleDeg 目标角度，单位为度。
 */
void AngleCtrl_InitWithTarget(float targetAngleDeg);

/**
 * @brief 清除 PID 运行状态，但保留当前目标角度。
 */
void AngleCtrl_Reset(void);

/**
 * @brief 设置新的目标角度。
 *
 * 当目标角度变化时，会清除积分和微分状态，避免旧状态影响新目标。
 *
 * @param targetAngleDeg 目标角度，单位为度。
 */
void AngleCtrl_SetTarget(float targetAngleDeg);

/**
 * @brief 将当前测量角度锁定为新的目标角度。
 *
 * 适合上电后把当前车头方向作为直行方向，或者按键重新校准当前方向。
 *
 * @param measuredAngleDeg 当前测量角度，单位为度。
 */
void AngleCtrl_LockCurrent(float measuredAngleDeg);

/**
 * @brief 执行一次角度 PID 计算。
 *
 * @param measuredAngleDeg 当前测量角度，单位为度。
 * @return 带符号转向修正输出。
 */
int16_t AngleCtrl_Update(float measuredAngleDeg);

/**
 * @brief 计算左右电机速度闭环目标值。
 *
 * @param baseSpeed 两个电机共同的基础前进速度目标。
 * @param measuredAngleDeg 当前测量角度，单位为度。
 * @param motorA 电机 A 目标速度输出指针，可以为空。
 * @param motorB 电机 B 目标速度输出指针，可以为空。
 */
void AngleCtrl_GetMotorTargets(int32_t baseSpeed, float measuredAngleDeg,
    int32_t *motorA, int32_t *motorB);

/**
 * @brief 计算左右电机目标，并直接写入 AT8236 速度闭环。
 *
 * @param baseSpeed 两个电机共同的基础前进速度目标。
 * @param measuredAngleDeg 当前测量角度，单位为度。
 */
void AngleCtrl_UpdateMotors(int32_t baseSpeed, float measuredAngleDeg);

/**
 * @brief 停止角度控制器，并清零 AT8236 速度闭环目标。
 */
void AngleCtrl_Stop(void);

#endif
