#include "myxunji.h"

#define XUNJI_FORWARD_4_SPEED          25
#define XUNJI_FORWARD_5_SPEED          25

/*
 * 第二问严格只使用中间XJ4、XJ5两路做方向控制。
 * 中间两路间距1.5cm，小于1.8cm的黑线宽度，所以车身位于黑线中间时，
 * 中间两路通常会同时检测到黑线，此时误差为0。
 * 其他对管的状态不会进入循迹误差和PID计算；XJ6只与XJ4、XJ5一起
 * 用于识别5cm宽的横向停车线。
 *
 * PID采用整数定点运算，实际增益 = 参数 / Q2_PID_SCALE。
 * 第一轮调试先关闭积分(KI=0)，因为数字量传感器的位置分辨率较低，
 * 积分很容易累积后造成左右慢摆；先根据串口数据把KP、KD调稳，再考虑KI。
 */
#define Q2_PID_KP                       9
#define Q2_PID_KI                       0
#define Q2_PID_KD                       2
#define Q2_PID_SCALE                    100
#define Q2_PID_INTEGRAL_LIMIT           600
#define Q2_PID_OUTPUT_LIMIT             12
#define Q2_LOST_ERROR                   200
#define Q2_NORMAL_SPEED                 24
#define Q2_LOST_SPEED                   14
#define Q2_SPEED_UP_STEP                 1
#define Q2_SPEED_DOWN_STEP               2

static int32_t q2_pid_integral = 0;
static int16_t q2_pid_last_error = 0;
static int16_t q2_last_nonzero_error = 0;
static int16_t q2_base_speed = Q2_NORMAL_SPEED;

Xunji_Q2PidDebug Xunji_Q2_Debug = {0};

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

// static void Xunji_Calc_Track_Speed(uint8_t bits, int16_t* left_speed, int16_t* right_speed)
// {
//     int16_t speed_error = 0;
//     if ((bits & (1U << 3)) != 0U) {speed_error = 3;}
//     else if ((bits & (1U << 4)) != 0U) {speed_error = -3;}
//     else if ((bits & (1U << 2)) != 0U) {speed_error = 5;}
//     else if ((bits & (1U << 5)) != 0U) {speed_error = -5;}
//     else if ((bits & (1U << 1)) != 0U) {speed_error = 8;}
//     else if ((bits & (1U << 6)) != 0U) {speed_error = -8;}
//     else if ((bits & (1U << 0)) != 0U) {speed_error = 15;}
//     else if ((bits & (1U << 7)) != 0U) {speed_error = -15;}

//     *left_speed = XUNJI_FORWARD_SPEED - speed_error;
//     *right_speed = XUNJI_FORWARD_SPEED + speed_error;
// }

void Xunji_Q2_Reset(void)
{
    /* 每次启动第二问前清空上一次运行留下的PID历史量。 */
    q2_pid_integral = 0;
    q2_pid_last_error = 0;
    q2_last_nonzero_error = 0;
    q2_base_speed = Q2_NORMAL_SPEED;
    Xunji_Q2_Debug = (Xunji_Q2PidDebug){0};
}

uint8_t Xunji_Q2_StopLineDetected(void)
{
    /*
     * 这里只读取XJ4、XJ5、XJ6，用于10ms中断中的停车线检测。
     * 三路同时为1说明扫到了5cm停车线；此函数不参与循迹PID计算。
     */
    return (uint8_t)((XUNJI_HW_READ_3() != 0U) &&
                     (XUNJI_HW_READ_4() != 0U) &&
                     (XUNJI_HW_READ_5() != 0U));
}

uint8_t Xunji_Q2_Task(int16_t* left_speed, int16_t* right_speed)
{
    uint8_t bits;
    uint8_t middle_state;
    int16_t desired_speed;
    int16_t base_speed;
    int16_t error;
    int16_t derivative;
    int32_t output;

    if((left_speed == 0) || (right_speed == 0))
    {
        return 0U;
    }

    bits = Xunji_Read_Bits();
    middle_state = (uint8_t)((bits >> 3) & 0x03U);

    Xunji_Q2_Debug.sensor_bits = bits;
    Xunji_Q2_Debug.sensor_state = middle_state;

    /*
     * XJ4、XJ5、XJ6对应bits的第3、4、5位。
     * 三路同时检测到黑线时说明扫到了5cm宽的横向停车线；
     * 普通循迹线只有1.8cm宽，不会同时覆盖这三路。
     */
    if(((bits >> 3) & 0x07U) == 0x07U)
    {
        *left_speed = 0;
        *right_speed = 0;
        Xunji_Q2_Debug.left_target = 0;
        Xunji_Q2_Debug.right_target = 0;
        return 1U;
    }

    if(middle_state == 0x01U)
    {
        /* 只有XJ4检测到黑线，产生正方向误差。 */
        error = 100;
        q2_last_nonzero_error = error;
        desired_speed = Q2_NORMAL_SPEED;
    }
    else if(middle_state == 0x02U)
    {
        /* 只有XJ5检测到黑线，产生负方向误差。 */
        error = -100;
        q2_last_nonzero_error = error;
        desired_speed = Q2_NORMAL_SPEED;
    }
    else if(middle_state == 0x03U)
    {
        /* XJ4、XJ5都检测到1.8cm黑线，车体位于轨迹中间。 */
        error = 0;
        desired_speed = Q2_NORMAL_SPEED;
    }
    else
    {
        /*
         * XJ4、XJ5都没有检测到黑线时，只沿中间两路最后一次给出的
         * 偏离方向找线。即使其他外侧对管检测到黑线，也不会改变PID误差。
         */
        if(q2_last_nonzero_error > 0)
        {
            error = Q2_LOST_ERROR;
        }
        else if(q2_last_nonzero_error < 0)
        {
            error = -Q2_LOST_ERROR;
        }
        else
        {
            error = 0;
        }

        desired_speed = Q2_LOST_SPEED;
    }

    /*
     * XJ4、XJ5无法区分“居中的弯道”和直线，因此正常循迹统一使用速度24，
     * 避免原来的直线/弯道模式反复切换。只有两路都丢线时才降到14；
     * 恢复轨迹后每10ms升1，丢线时每10ms降2，避免速度突变。
     */
    if(q2_base_speed < desired_speed)
    {
        q2_base_speed += Q2_SPEED_UP_STEP;
        if(q2_base_speed > desired_speed)
        {
            q2_base_speed = desired_speed;
        }
    }
    else if(q2_base_speed > desired_speed)
    {
        q2_base_speed -= Q2_SPEED_DOWN_STEP;
        if(q2_base_speed < desired_speed)
        {
            q2_base_speed = desired_speed;
        }
    }
    base_speed = q2_base_speed;

    q2_pid_integral += error;
    if(q2_pid_integral > Q2_PID_INTEGRAL_LIMIT)
    {
        q2_pid_integral = Q2_PID_INTEGRAL_LIMIT;
    }
    else if(q2_pid_integral < -Q2_PID_INTEGRAL_LIMIT)
    {
        q2_pid_integral = -Q2_PID_INTEGRAL_LIMIT;
    }

    derivative = (int16_t)(error - q2_pid_last_error);
    q2_pid_last_error = error;

    output = ((int32_t)Q2_PID_KP * error) +
             ((int32_t)Q2_PID_KI * q2_pid_integral) +
             ((int32_t)Q2_PID_KD * derivative);
    output /= Q2_PID_SCALE;

    /* 限制左右轮速度差，防止一次传感器跳变让小车猛打方向。 */
    if(output > Q2_PID_OUTPUT_LIMIT)
    {
        output = Q2_PID_OUTPUT_LIMIT;
    }
    else if(output < -Q2_PID_OUTPUT_LIMIT)
    {
        output = -Q2_PID_OUTPUT_LIMIT;
    }

    /* 与原来的方向一致：正误差时左轮减速、右轮加速。 */
    *left_speed = (int16_t)(base_speed - output);
    *right_speed = (int16_t)(base_speed + output);

    /* 保存本次数据，供主循环低频发送，不在这里阻塞循迹控制。 */
    Xunji_Q2_Debug.error = error;
    Xunji_Q2_Debug.integral = q2_pid_integral;
    Xunji_Q2_Debug.derivative = derivative;
    Xunji_Q2_Debug.p_term = (int16_t)(((int32_t)Q2_PID_KP * error) / Q2_PID_SCALE);
    Xunji_Q2_Debug.i_term = (int16_t)(((int32_t)Q2_PID_KI * q2_pid_integral) / Q2_PID_SCALE);
    Xunji_Q2_Debug.d_term = (int16_t)(((int32_t)Q2_PID_KD * derivative) / Q2_PID_SCALE);
    Xunji_Q2_Debug.output = (int16_t)output;
    Xunji_Q2_Debug.left_target = *left_speed;
    Xunji_Q2_Debug.right_target = *right_speed;

    return 0U;
}

void Xunji_Q4_Task(int16_t* left_speed, int16_t* right_speed)
{  

    uint8_t bits = Xunji_Read_Bits();

    if((bits & (1U << 0)) != 0U) //走完直线循迹，设置电机速度为0
    {
        *left_speed = 0;
        *right_speed = 0;
        return;
    }

    int16_t speed_error = 0;
    if ((bits & (1U << 3)) != 0U) {speed_error = 3;}
    else if ((bits & (1U << 4)) != 0U) {speed_error = -3;}
    else if ((bits & (1U << 2)) != 0U) {speed_error = 5;}
    else if ((bits & (1U << 5)) != 0U) {speed_error = -5;}
    else if ((bits & (1U << 1)) != 0U) {speed_error = 8;}
    else if ((bits & (1U << 6)) != 0U) {speed_error = -8;}
    else if ((bits & (1U << 0)) != 0U) {speed_error = 15;}
    else if ((bits & (1U << 7)) != 0U) {speed_error = -15;}

    *left_speed = XUNJI_FORWARD_4_SPEED - speed_error;
    *right_speed = XUNJI_FORWARD_4_SPEED + speed_error;
}

void Xunji_Q5_Task(int16_t* left_speed, int16_t* right_speed)
{  

    uint8_t bits = Xunji_Read_Bits();

    int16_t speed_error = 0;
    if ((bits & (1U << 3)) != 0U) {speed_error = 3;}
    else if ((bits & (1U << 4)) != 0U) {speed_error = -3;}
    else if ((bits & (1U << 2)) != 0U) {speed_error = 5;}
    else if ((bits & (1U << 5)) != 0U) {speed_error = -5;}
    else if ((bits & (1U << 1)) != 0U) {speed_error = 8;}
    else if ((bits & (1U << 6)) != 0U) {speed_error = -8;}
    else if ((bits & (1U << 0)) != 0U) {speed_error = 15;}
    else if ((bits & (1U << 7)) != 0U) {speed_error = -15;}

    *left_speed = XUNJI_FORWARD_5_SPEED - speed_error;
    *right_speed = XUNJI_FORWARD_5_SPEED + speed_error;
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
