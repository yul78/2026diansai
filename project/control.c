#include "control.h"
#include "encoder.h"


#define CONTROL_WHEEL_DIAMETER_MM 65.0f
#define CONTROL_ENCODER_PULSES_PER_REV 450.0f
#define CONTROL_PI 3.1415926f

static uint32_t control_abs_i32(int32_t value)
{
    if (value < 0) {
        return (uint32_t)(-value);
    }

    return (uint32_t)value;
}

static uint32_t control_distance_cm_to_pulses(uint16_t distance_cm)
{
    float distance_mm = (float)distance_cm * 10.0f;
    float wheel_circumference_mm = CONTROL_PI * CONTROL_WHEEL_DIAMETER_MM;
    float pulses = (distance_mm * CONTROL_ENCODER_PULSES_PER_REV) / wheel_circumference_mm;

    return (uint32_t)(pulses + 0.5f);
}

void Control_MoveDistanceCm(uint16_t distance_cm, int16_t speed)
{
    uint32_t target_pulses;
    uint32_t left_count;
    uint32_t right_count;
    uint32_t average_count;

    if ((distance_cm == 0U) || (speed == 0)) {
        TB6612_Stop();
        return;
    }

    target_pulses = control_distance_cm_to_pulses(distance_cm);
    EncoderA.count = 0;
    EncoderB.count = 0;

    TB6612_SetMotors(speed, speed);

    while (1) {
        left_count = control_abs_i32(Encoder_GetCountA());
        right_count = control_abs_i32(Encoder_GetCountB());
        average_count = (left_count + right_count) / 2U;

        if (average_count >= target_pulses) {
            break;
        }
    }

    TB6612_Stop();
}
