#include "at8236.h"
#include "ti_msp_dl_config.h"
#include <stdbool.h>

/*
 * SysConfig mapping, period 1000, edge-aligned up counting:
 *   Motor A IN1/IN2: PWM1_AT8236 C0/C1 (TIMG6, PB6/PB7)
 *   Motor B IN1/IN2: PWM2_AT8236 C0/C1 (TIMA1, PB2/PB3)
 */

#define AT8236_PWM_PERIOD 1000U
#define AT8236_PWM_MAX    (AT8236_PWM_PERIOD - 1U)
#define AT8236_PWM_HIGH   (AT8236_PWM_PERIOD)

AT8236_Motor AT8236_MotorA = {
    .channel = AT8236_MOTOR_CHANNEL_A,
    .target_speed = 0,
    .max_speed = AT8236_PWM_MAX,
    .direction = 1,
};

AT8236_Motor AT8236_MotorB = {
    .channel = AT8236_MOTOR_CHANNEL_B,
    .target_speed = 0,
    .max_speed = AT8236_PWM_MAX,
    .direction = -1,
};

static uint16_t at8236_get_max_speed(const AT8236_Motor *motor)
{
    if (motor->max_speed <= 0) {
        return 0;
    }

    if ((uint32_t)motor->max_speed > AT8236_PWM_MAX) {
        return AT8236_PWM_MAX;
    }

    return (uint16_t)motor->max_speed;
}

static uint16_t at8236_clamp_speed(const AT8236_Motor *motor, int32_t speed)
{
    uint16_t max_speed = at8236_get_max_speed(motor);

    if (speed < 0) {
        speed = -speed;
    }

    if ((uint32_t)speed > max_speed) {
        return max_speed;
    }

    return (uint16_t)speed;
}

static int32_t at8236_get_output_speed(const AT8236_Motor *motor)
{
    int8_t direction = (motor->direction >= 0) ? 1 : -1;

    return (int32_t)motor->target_speed * direction;
}

static void at8236_set_inputs(AT8236_MotorChannel channel,
    uint16_t in1Compare, uint16_t in2Compare)
{
    if (channel == AT8236_MOTOR_CHANNEL_A) {
        DL_TimerG_setCaptureCompareValue(PWM1_AT8236_INST, in1Compare,
            GPIO_PWM1_AT8236_C0_IDX);
        DL_TimerG_setCaptureCompareValue(PWM1_AT8236_INST, in2Compare,
            GPIO_PWM1_AT8236_C1_IDX);
    } else {
        DL_TimerA_setCaptureCompareValue(PWM2_AT8236_INST, in1Compare,
            GPIO_PWM2_AT8236_C0_IDX);
        DL_TimerA_setCaptureCompareValue(PWM2_AT8236_INST, in2Compare,
            GPIO_PWM2_AT8236_C1_IDX);
    }
}

static void at8236_apply_motor(const AT8236_Motor *motor)
{
    int32_t output_speed = at8236_get_output_speed(motor);
    uint16_t pwm = at8236_clamp_speed(motor, output_speed);
    uint16_t activeCompare = AT8236_PWM_PERIOD - pwm;

    /*
     * The inactive input stays high. The active input is high while off and
     * low for pwm/period of each cycle, so both directions use the H/H stop
     * state and have matching decay behavior.
     */
    if (output_speed > 0) {
        at8236_set_inputs(motor->channel, AT8236_PWM_HIGH, activeCompare);
    } else if (output_speed < 0) {
        at8236_set_inputs(motor->channel, activeCompare, AT8236_PWM_HIGH);
    } else {
        at8236_set_inputs(motor->channel,
            AT8236_PWM_HIGH, AT8236_PWM_HIGH);
    }
}

void AT8236_Init(void)
{
    AT8236_SetMotor(&AT8236_MotorA, 0);
    AT8236_SetMotor(&AT8236_MotorB, 0);

    DL_TimerG_startCounter(PWM1_AT8236_INST);
    DL_TimerA_startCounter(PWM2_AT8236_INST);
}

void AT8236_SetMotor(AT8236_Motor *motor, int16_t speed)
{
    if (motor == 0) {
        return;
    }

    motor->target_speed = speed;
    at8236_apply_motor(motor);
}

void AT8236_SetMotorA(int16_t speed)
{
    AT8236_SetMotor(&AT8236_MotorA, speed);
}

void AT8236_SetMotorB(int16_t speed)
{
    AT8236_SetMotor(&AT8236_MotorB, speed);
}

void AT8236_SetMotors(int16_t motorA, int16_t motorB)
{
    AT8236_SetMotor(&AT8236_MotorA, motorA);
    AT8236_SetMotor(&AT8236_MotorB, motorB);
}

void AT8236_Stop(void)
{
    AT8236_SetMotors(0, 0);
}

AT8236_PIDController AT8236_PIDA = {0};
AT8236_PIDController AT8236_PIDB = {0};

static int32_t at8236_pid_clamp_i32(int64_t value,
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

static void at8236_pid_reset_controller(AT8236_PIDController *controller)
{
    controller->target_speed = 0;
    controller->measured_speed = 0;
    controller->error = 0;
    controller->integral = 0;
    controller->previous_error = 0;
    controller->output = 0;
    controller->initialized = 0U;
}

static void at8236_pid_set_target(AT8236_PIDController *controller,
    int32_t target)
{
    bool directionChanged =
        ((controller->target_speed > 0) && (target <= 0)) ||
        ((controller->target_speed < 0) && (target >= 0));

    controller->target_speed = target;

    if (directionChanged || (target == 0)) {
        controller->error = 0;
        controller->integral = 0;
        controller->previous_error = 0;
        controller->output = 0;
        controller->initialized = 0U;
    }
}

static int16_t at8236_pid_calculate(AT8236_PIDController *controller,
    int32_t measured, int32_t kp, int32_t ki, int32_t kd,
    int32_t motorOutputLimit)
{
    int32_t derivative;
    int32_t outputLimit;
    int64_t output;

    controller->measured_speed = measured;

    if (controller->target_speed == 0) {
        controller->error = -measured;
        controller->integral = 0;
        controller->previous_error = 0;
        controller->output = 0;
        controller->initialized = 0U;
        return 0;
    }

    controller->error = controller->target_speed - measured;
    controller->integral = at8236_pid_clamp_i32(
        (int64_t)controller->integral + controller->error,
        -AT8236_PID_INTEGRAL_LIMIT, AT8236_PID_INTEGRAL_LIMIT);

    if (controller->initialized != 0U) {
        derivative = controller->error - controller->previous_error;
    } else {
        derivative = 0;
        controller->initialized = 1U;
    }
    controller->previous_error = controller->error;

    output = ((int64_t)kp * controller->error) +
        ((int64_t)ki * controller->integral) +
        ((int64_t)kd * derivative);
    output /= AT8236_PID_GAIN_SCALE;

    outputLimit = motorOutputLimit;
    if (outputLimit > AT8236_PID_OUTPUT_LIMIT) {
        outputLimit = AT8236_PID_OUTPUT_LIMIT;
    }
    if (outputLimit < 0) {
        outputLimit = 0;
    }
    if ((controller->target_speed <= AT8236_PID_LOW_TARGET_LIMIT) &&
        (controller->target_speed >= -AT8236_PID_LOW_TARGET_LIMIT) &&
        (outputLimit > AT8236_PID_LOW_TARGET_OUTPUT_LIMIT)) {
        outputLimit = AT8236_PID_LOW_TARGET_OUTPUT_LIMIT;
    }

    output = at8236_pid_clamp_i32(output, -outputLimit, outputLimit);

    /* A speed loop must not reverse the motor merely to correct overspeed. */
    if ((controller->target_speed > 0) && (output < 0)) {
        output = 0;
    } else if ((controller->target_speed < 0) && (output > 0)) {
        output = 0;
    }

    if ((output > 0) && (output < AT8236_PID_MIN_OUTPUT)) {
        output = AT8236_PID_MIN_OUTPUT;
    } else if ((output < 0) && (output > -AT8236_PID_MIN_OUTPUT)) {
        output = -AT8236_PID_MIN_OUTPUT;
    }

    output = at8236_pid_clamp_i32(output, -outputLimit, outputLimit);

    controller->output = (int16_t)output;
    return controller->output;
}

void AT8236_PID_Init(void)
{
    at8236_pid_reset_controller(&AT8236_PIDA);
    at8236_pid_reset_controller(&AT8236_PIDB);
}

void AT8236_PID_SetTargets(int32_t targetA, int32_t targetB)
{
    at8236_pid_set_target(&AT8236_PIDA, targetA);
    at8236_pid_set_target(&AT8236_PIDB, targetB);
}

void AT8236_PID_Update(int32_t measuredA, int32_t measuredB)
{
    int16_t outputA = at8236_pid_calculate(&AT8236_PIDA, measuredA,
        AT8236_PID_A_KP, AT8236_PID_A_KI, AT8236_PID_A_KD,
        at8236_get_max_speed(&AT8236_MotorA));
    int16_t outputB = at8236_pid_calculate(&AT8236_PIDB, measuredB,
        AT8236_PID_B_KP, AT8236_PID_B_KI, AT8236_PID_B_KD,
        at8236_get_max_speed(&AT8236_MotorB));

    AT8236_SetMotors(outputA, outputB);
}

void AT8236_PID_SetMotors(int32_t targetA, int32_t targetB,
    int32_t measuredA, int32_t measuredB)
{
    AT8236_PID_SetTargets(targetA, targetB);
    AT8236_PID_Update(measuredA, measuredB);
}

void AT8236_PID_Stop(void)
{
    AT8236_PID_Init();
    AT8236_Stop();
}
