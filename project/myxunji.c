#include "myxunji.h"

#define XUNJI_FORWARD_SPEED          25

//以下下是PID的参数
#define MYXUNJI_PID_KP               180
#define MYXUNJI_PID_KI               15
#define MYXUNJI_PID_KD               120
#define MYXUNJI_PID_SCALE            100
#define MYXUNJI_SPEED_MIN            (-1000)
#define MYXUNJI_SPEED_MAX            1000
#define MYXUNJI_INTEGRAL_MIN         (-1000)
#define MYXUNJI_INTEGRAL_MAX         1000

static int32_t myxunji_pid_integral = 0;
static int16_t myxunji_pid_last_error = 0;

uint8_t Xunji_Read_Hardware(uint8_t index)
{
    switch (index) {
        case 0U: return XUNJI_HW_READ_0();
        case 1U: return XUNJI_HW_READ_1();
        case 2U: return XUNJI_HW_READ_2();
        case 3U: return XUNJI_HW_READ_3();
        case 4U: return XUNJI_HW_READ_4();
        case 5U: return XUNJI_HW_READ_5();
        case 6U: return XUNJI_HW_READ_6();
        case 7U: return XUNJI_HW_READ_7();
        default: return 0U;
    }
}

static uint8_t Xunji_Read_Bits(void)
{
    uint8_t bits = 0U;

    for (uint8_t i = 0U; i < 8U; ++i) {
        if (Xunji_Read_Hardware(i) != 0U) {
            bits |= (uint8_t) (1U << i);
        }
    }

    return bits;
}

static void Xunji_Calc_Track_Speed(uint8_t bits, int16_t* left_speed, int16_t* right_speed)
{
    int16_t speed_error = 0;
    if ((bits & (1U << 3)) != 0U) {speed_error = 3;}
    else if ((bits & (1U << 4)) != 0U) {speed_error = -3;}
    else if ((bits & (1U << 2)) != 0U) {speed_error = 5;}
    else if ((bits & (1U << 5)) != 0U) {speed_error = -5;}
    else if ((bits & (1U << 1)) != 0U) {speed_error = 8;}
    else if ((bits & (1U << 6)) != 0U) {speed_error = -8;}
    else if ((bits & (1U << 0)) != 0U) {speed_error = 15;}
    else if ((bits & (1U << 7)) != 0U) {speed_error = -15;}

    *left_speed = XUNJI_FORWARD_SPEED - speed_error;
    *right_speed = XUNJI_FORWARD_SPEED + speed_error;
}

void Xunji_Task(int16_t* left_speed, int16_t* right_speed)
{  

    uint8_t bits = Xunji_Read_Bits();
    Xunji_Calc_Track_Speed(bits, left_speed, right_speed);
}

static void MyXunji_ResetPidState(void)
{
    myxunji_pid_integral = 0;
    myxunji_pid_last_error = 0;
}

static uint8_t MyXunji_IsLost(uint8_t bits)
{
    return (uint8_t) (bits == 0U);
}

static int16_t MyXunji_ClampSpeed(int32_t speed)
{
    if (speed < MYXUNJI_SPEED_MIN) {
        return MYXUNJI_SPEED_MIN;
    }
    if (speed > MYXUNJI_SPEED_MAX) {
        return MYXUNJI_SPEED_MAX;
    }
    return (int16_t) speed;
}

static int32_t MyXunji_ClampIntegral(int32_t integral)
{
    if (integral < MYXUNJI_INTEGRAL_MIN) {
        return MYXUNJI_INTEGRAL_MIN;
    }
    if (integral > MYXUNJI_INTEGRAL_MAX) {
        return MYXUNJI_INTEGRAL_MAX;
    }
    return integral;
}

static int16_t MyXunji_CalcError(uint8_t bits)
{
    static const int16_t weights[8] = {-7, -5, -3, -1, 1, 3, 5, 7};
    int32_t weighted_sum = 0;
    uint8_t active_count = 0U;

    for (uint8_t i = 0U; i < 8U; ++i) {
        if ((bits & (uint8_t) (1U << i)) != 0U) {
            weighted_sum += weights[i];
            ++active_count;
        }
    }

    if (active_count == 0U) {
        return 0;
    }

    return (int16_t) (weighted_sum / (int32_t) active_count);
}

void MyXunji_PidTrackStraight(uint8_t bits, int16_t base_speed, int16_t* left_speed, int16_t* right_speed)
{
    int16_t error;
    int16_t derivative;
    int32_t output;

    if ((left_speed == 0) || (right_speed == 0)) {
        return;
    }

    if (MyXunji_IsLost(bits) != 0U) {
        MyXunji_ResetPidState();
        *left_speed = 0;
        *right_speed = 0;
        return;
    }

    error = MyXunji_CalcError(bits);
    myxunji_pid_integral = MyXunji_ClampIntegral(myxunji_pid_integral + error);
    derivative = (int16_t) (error - myxunji_pid_last_error);
    myxunji_pid_last_error = error;

    output = ((int32_t) MYXUNJI_PID_KP * error) +
             ((int32_t) MYXUNJI_PID_KI * myxunji_pid_integral) +
             ((int32_t) MYXUNJI_PID_KD * derivative);
    output /= MYXUNJI_PID_SCALE;

    *left_speed = MyXunji_ClampSpeed((int32_t) base_speed + output);
    *right_speed = MyXunji_ClampSpeed((int32_t) base_speed - output);
}
