#include "myxunji.h"

/*
 * 第四问只完成A到B的1.5m直线阶段。
 * 软启动直接使用题目运行时间计算，不依赖主循环实际执行频率。
 */
#define Q4_MAX_SPEED                    28
#define Q4_START_SPEED                   3
#define Q4_TRACK_CORRECTION              1
#define Q4_START_RAMP_INTERVAL_MS      300U
#define Q4_BRAKE_RAMP_INTERVAL_MS       50U

static int16_t q4_base_speed = 0;
static int16_t q4_last_correction = 0;
static uint8_t q4_stop_requested = 0U;
static uint8_t q4_stop_ramp_started = 0U;
static uint32_t q4_stop_start_ms = 0U;
static int16_t q4_stop_start_speed = 0;

Xunji_Q4Debug Xunji_Q4_Debug = {0};

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

/* 第五、六问目前使用相同PID参数，但分别设置速度，后续可以单独调节。 */
#define Q56_PID_KP                       9
#define Q56_PID_KI                       0
#define Q56_PID_KD                       2
#define Q56_PID_SCALE                  100
#define Q56_PID_INTEGRAL_LIMIT         600
#define Q56_PID_OUTPUT_LIMIT            12
#define Q56_LOST_ERROR                 200
#define Q56_SPEED_UP_STEP                1
#define Q56_SPEED_DOWN_STEP              2

/* 第五问速度参数，只修改这里不会影响第六问。 */
#define Q5_NORMAL_SPEED                 24
#define Q5_LOST_SPEED                   14
#define Q5_START_SPEED                   5
#define Q5_START_RAMP_INTERVAL_MS      150U

/* 第六问速度参数，只修改这里不会影响第五问。 */
#define Q6_NORMAL_SPEED                 24
#define Q6_LOST_SPEED                   14
#define Q6_START_SPEED                   5
#define Q6_START_RAMP_INTERVAL_MS      150U

/* 每一问独立保存自己的PID历史量和当前基础速度。 */
typedef struct
{
    int32_t pid_integral;
    int16_t pid_last_error;
    int16_t last_nonzero_error;
    int16_t base_speed;
} Xunji_Q56State;

static Xunji_Q56State q5_track_state = {0, 0, 0, Q5_START_SPEED};
static Xunji_Q56State q6_track_state = {0, 0, 0, Q6_START_SPEED};

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

static void Xunji_Q56_ResetCore(
    Xunji_Q56State* state, int16_t start_speed)
{
    state->pid_integral = 0;
    state->pid_last_error = 0;
    state->last_nonzero_error = 0;
    state->base_speed = start_speed;
}

static void Xunji_Q56_TaskCore(
    uint32_t elapsed_ms,
    int16_t* left_speed,
    int16_t* right_speed,
    Xunji_Q56State* state,
    int16_t normal_speed,
    int16_t lost_speed,
    int16_t start_speed,
    uint32_t ramp_interval_ms)
{
    uint8_t bits;
    uint8_t middle_state;
    int16_t desired_speed;
    int16_t startup_speed_limit;
    int16_t error;
    int16_t derivative;
    int32_t output;
    uint32_t ramp_speed;

    if((left_speed == 0) || (right_speed == 0))
    {
        return;
    }

    bits = Xunji_Read_Bits();
    middle_state = (uint8_t)((bits >> 3) & 0x03U);

    /*
     * 只使用中间XJ4、XJ5两路计算方向误差。
     * 其他红外对管即使在弯道扫到黑线，也不会参与第五、六问的方向控制。
     */
    if(middle_state == 0x01U)
    {
        error = 100;
        state->last_nonzero_error = error;
        desired_speed = normal_speed;
    }
    else if(middle_state == 0x02U)
    {
        error = -100;
        state->last_nonzero_error = error;
        desired_speed = normal_speed;
    }
    else if(middle_state == 0x03U)
    {
        error = 0;
        desired_speed = normal_speed;
    }
    else
    {
        /*
         * 中间两路都丢线时，按照最后一次偏离方向寻找黑线，并将基础速度
         * 降到14。这样与第二问已验证的椭圆循迹行为保持一致。
         */
        if(state->last_nonzero_error > 0)
        {
            error = Q56_LOST_ERROR;
        }
        else if(state->last_nonzero_error < 0)
        {
            error = -Q56_LOST_ERROR;
        }
        else
        {
            error = 0;
        }

        desired_speed = lost_speed;
    }

    /*
     * 软启动速度上限：0ms时为5，之后每150ms增加1，最大为24。
     * 使用题目计时elapsed_ms计算，因此不会受主循环执行快慢影响。
     */
    ramp_speed = (uint32_t)start_speed +
                 (elapsed_ms / ramp_interval_ms);
    if(ramp_speed > (uint32_t)normal_speed)
    {
        ramp_speed = (uint32_t)normal_speed;
    }
    startup_speed_limit = (int16_t)ramp_speed;

    if(desired_speed > startup_speed_limit)
    {
        desired_speed = startup_speed_limit;
    }

    /* 保留第二问恢复轨迹时缓升、丢线时较快降速的处理。 */
    if(state->base_speed < desired_speed)
    {
        state->base_speed += Q56_SPEED_UP_STEP;
        if(state->base_speed > desired_speed)
        {
            state->base_speed = desired_speed;
        }
    }
    else if(state->base_speed > desired_speed)
    {
        state->base_speed -= Q56_SPEED_DOWN_STEP;
        if(state->base_speed < desired_speed)
        {
            state->base_speed = desired_speed;
        }
    }

    state->pid_integral += error;
    if(state->pid_integral > Q56_PID_INTEGRAL_LIMIT)
    {
        state->pid_integral = Q56_PID_INTEGRAL_LIMIT;
    }
    else if(state->pid_integral < -Q56_PID_INTEGRAL_LIMIT)
    {
        state->pid_integral = -Q56_PID_INTEGRAL_LIMIT;
    }

    derivative = (int16_t)(error - state->pid_last_error);
    state->pid_last_error = error;

    output = ((int32_t)Q56_PID_KP * error) +
             ((int32_t)Q56_PID_KI * state->pid_integral) +
             ((int32_t)Q56_PID_KD * derivative);
    output /= Q56_PID_SCALE;

    if(output > Q56_PID_OUTPUT_LIMIT)
    {
        output = Q56_PID_OUTPUT_LIMIT;
    }
    else if(output < -Q56_PID_OUTPUT_LIMIT)
    {
        output = -Q56_PID_OUTPUT_LIMIT;
    }

    /*
     * 软启动尚未结束时，方向修正量也按当前基础速度同比减小。
     * 避免低速起步阶段一侧车轮突然得到过大的目标速度。
     */
    if(startup_speed_limit < normal_speed)
    {
        output = (output * state->base_speed) / normal_speed;
    }

    /* 最终再限制一次，保证起步阶段左右轮目标速度都不会反转。 */
    if(output > state->base_speed)
    {
        output = state->base_speed;
    }
    else if(output < -state->base_speed)
    {
        output = -state->base_speed;
    }

    *left_speed = (int16_t)(state->base_speed - output);
    *right_speed = (int16_t)(state->base_speed + output);
}

void Xunji_Q5_Reset(void)
{
    /* 第五问只清空自己的PID状态。 */
    Xunji_Q56_ResetCore(&q5_track_state, Q5_START_SPEED);
}

void Xunji_Q5_Task(
    uint32_t elapsed_ms, int16_t* left_speed, int16_t* right_speed)
{
    Xunji_Q56_TaskCore(
        elapsed_ms,
        left_speed,
        right_speed,
        &q5_track_state,
        Q5_NORMAL_SPEED,
        Q5_LOST_SPEED,
        Q5_START_SPEED,
        Q5_START_RAMP_INTERVAL_MS);
}

void Xunji_Q6_Reset(void)
{
    /* 第六问只清空自己的PID状态。 */
    Xunji_Q56_ResetCore(&q6_track_state, Q6_START_SPEED);
}

void Xunji_Q6_Task(
    uint32_t elapsed_ms, int16_t* left_speed, int16_t* right_speed)
{
    Xunji_Q56_TaskCore(
        elapsed_ms,
        left_speed,
        right_speed,
        &q6_track_state,
        Q6_NORMAL_SPEED,
        Q6_LOST_SPEED,
        Q6_START_SPEED,
        Q6_START_RAMP_INTERVAL_MS);
}

void Xunji_Q4_Reset(void)
{
    /* 每次启动第四问时，都从速度0重新开始软启动。 */
    q4_base_speed = 0;
    q4_last_correction = 0;
    q4_stop_requested = 0U;
    q4_stop_ramp_started = 0U;
    q4_stop_start_ms = 0U;
    q4_stop_start_speed = 0;
    Xunji_Q4_Debug = (Xunji_Q4Debug){0};
}

void Xunji_Q4_RequestStop(void)
{
    /*
     * 第四问确认通过B点并继续直行一段时间后，由主循环调用一次。
     * 收到请求后，基础速度每20ms降低1，直到平缓降为0。
     */
    q4_stop_requested = 1U;
}

uint8_t Xunji_Q4_Task(
    uint32_t elapsed_ms, int16_t* left_speed, int16_t* right_speed)
{
    uint8_t bits;
    uint8_t middle_state;
    int16_t correction;
    uint32_t ramp_steps;

    if((left_speed == 0) || (right_speed == 0))
    {
        return 0U;
    }

    bits = Xunji_Read_Bits();
    middle_state = (uint8_t)((bits >> 3) & 0x03U);

    if(q4_stop_requested == 0U)
    {
        /*
         * 基础速度从5开始，再按每150ms增加1，最多增加到25。
         * 起始值5较小，不会让小车突然前冲，同时可以减少从0开始时
         * 因电机静摩擦造成的按键后等待时间。
         * 即使IMU读取、OLED或串口发送暂时阻塞主循环，软启动时间也不会变慢。
         */
        ramp_steps = Q4_START_SPEED +
                     (elapsed_ms / Q4_START_RAMP_INTERVAL_MS);
        if(ramp_steps > Q4_MAX_SPEED)
        {
            ramp_steps = Q4_MAX_SPEED;
        }
        q4_base_speed = (int16_t)ramp_steps;
    }
    else
    {
        /* 第一次处理停车请求时，记录软刹车开始时间和当时的速度。 */
        if(q4_stop_ramp_started == 0U)
        {
            q4_stop_ramp_started = 1U;
            q4_stop_start_ms = elapsed_ms;
            q4_stop_start_speed = q4_base_speed;
        }

        ramp_steps = (elapsed_ms - q4_stop_start_ms) /
                     Q4_BRAKE_RAMP_INTERVAL_MS;

        if(ramp_steps >= (uint32_t)q4_stop_start_speed)
        {
            q4_base_speed = 0;
        }
        else
        {
            q4_base_speed = (int16_t)(
                q4_stop_start_speed - (int16_t)ramp_steps);
        }
    }

    /*
     * 第四问直线方向只使用XJ4、XJ5：
     * 只有XJ4为1时向一个方向修正，只有XJ5为1时向相反方向修正；
     * 两路都为1表示位于1.8cm黑线中间，不修正；
     * 两路都丢线时保持上一次修正方向找线，外侧六路不参与控制。
     */
    if(middle_state == 0x01U)
    {
        correction = Q4_TRACK_CORRECTION;
        q4_last_correction = correction;
    }
    else if(middle_state == 0x02U)
    {
        correction = -Q4_TRACK_CORRECTION;
        q4_last_correction = correction;
    }
    else if(middle_state == 0x03U)
    {
        correction = 0;
    }
    else
    {
        correction = q4_last_correction;
    }

    /*
     * 低速时如果直接使用±3修正，左右目标速度比例会相差很大，
     * 两个轮子克服静摩擦的时刻不同，会造成起步突然偏转。
     * 因此让修正量随基础速度逐渐增加：速度25时才使用完整的±3。
     */
    correction = (int16_t)(
        ((int32_t)correction * q4_base_speed) / Q4_MAX_SPEED);

    /* 再做一次限幅，保证软启动过程中两个车轮都不会反转。 */
    if(correction > q4_base_speed)
    {
        correction = q4_base_speed;
    }
    else if(correction < -q4_base_speed)
    {
        correction = -q4_base_speed;
    }

    *left_speed = (int16_t)(q4_base_speed - correction);
    *right_speed = (int16_t)(q4_base_speed + correction);

    Xunji_Q4_Debug.sensor_bits = bits;
    Xunji_Q4_Debug.middle_state = middle_state;
    Xunji_Q4_Debug.base_speed = q4_base_speed;
    Xunji_Q4_Debug.correction = correction;
    Xunji_Q4_Debug.left_target = *left_speed;
    Xunji_Q4_Debug.right_target = *right_speed;

    /* 软刹车已经降到0时返回1；当前通过B点的流程不会触发此返回值。 */
    return (uint8_t)(q4_stop_requested && (q4_base_speed == 0));
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
