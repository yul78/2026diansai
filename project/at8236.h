#ifndef AT8236_H
#define AT8236_H

#include <stdint.h>

/**
 * @brief AT8236 motor channel.
 */
typedef enum {
    AT8236_MOTOR_CHANNEL_A = 0,
    AT8236_MOTOR_CHANNEL_B
} AT8236_MotorChannel;

/**
 * @brief AT8236 motor control object.
 */
typedef struct {
    AT8236_MotorChannel channel; /**< Hardware motor channel A or B. */
    int16_t target_speed;        /**< Signed PWM command, -999..999. */
    int16_t max_speed;           /**< Maximum allowed PWM magnitude. */
    int8_t direction;            /**< Motor polarity: 1 normal, -1 inverted. */
} AT8236_Motor;

/*
 * Speed PID gains use fixed-point arithmetic:
 * actual gain = gain macro / AT8236_PID_GAIN_SCALE.
 * The default control period is 10 ms, matching Encoder_GetSpeeds() in
 * TIMER_TICK_INST_IRQHandler. Retune the gains if that period changes.
 */
#define AT8236_PID_GAIN_SCALE       100L

#define AT8236_PID_A_KP             800L
#define AT8236_PID_A_KI             25L
#define AT8236_PID_A_KD             0L

#define AT8236_PID_B_KP             800L
#define AT8236_PID_B_KI             25L
#define AT8236_PID_B_KD             0L

#define AT8236_PID_INTEGRAL_LIMIT   8000L
#define AT8236_PID_OUTPUT_LIMIT     999L
#define AT8236_PID_MIN_OUTPUT       0L
#define AT8236_PID_LOW_TARGET_LIMIT 10L
#define AT8236_PID_LOW_TARGET_OUTPUT_LIMIT 220L
#define AT8236_PID_UPDATE_PERIOD_MS 10U

/**
 * @brief Runtime state of one motor speed PID controller.
 */
typedef struct {
    int32_t target_speed;     /**< Signed target encoder counts per update. */
    int32_t measured_speed;   /**< Signed measured encoder counts per update. */
    int32_t error;
    int32_t integral;
    int32_t previous_error;
    int16_t output;           /**< Signed PWM output, -999..999. */
    uint8_t initialized;
} AT8236_PIDController;

extern AT8236_Motor AT8236_MotorA;
extern AT8236_Motor AT8236_MotorB;
extern AT8236_PIDController AT8236_PIDA;
extern AT8236_PIDController AT8236_PIDB;

void AT8236_Init(void);
void AT8236_SetMotor(AT8236_Motor *motor, int16_t speed);
void AT8236_SetMotorA(int16_t speed);
void AT8236_SetMotorB(int16_t speed);
void AT8236_SetMotors(int16_t motorA, int16_t motorB);
void AT8236_Stop(void);

void AT8236_PID_Init(void);
void AT8236_PID_SetTargets(int32_t targetA, int32_t targetB);
void AT8236_PID_Update(int32_t measuredA, int32_t measuredB);
void AT8236_PID_SetMotors(int32_t targetA, int32_t targetB,
    int32_t measuredA, int32_t measuredB);
void AT8236_PID_Stop(void);

#endif
