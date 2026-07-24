#ifndef IMU963RA_ATTITUDE_H
#define IMU963RA_ATTITUDE_H

#include "zf_common_typedef.h"

// 重要参数人工调节说明
// 1. IMU963RA_ATTITUDE_GYRO_SETTLE_DELAY_MS
//    上电后开始零偏校准前的静置等待时间，单位 ms。
//    如果模块上电瞬间容易晃动，可适当调大。
//
// 2. IMU963RA_ATTITUDE_GYRO_CALIBRATE_COUNT
//    零偏校准采样次数，采样次数越多零偏平均越稳定，但启动等待时间也会变长。
//
// 3. IMU963RA_ATTITUDE_GYRO_Z_SCALE_TRIM
//    yaw 方向比例微调系数。
//    如果实际转角偏小，可略微调大；如果实际转角偏大，可略微调小。
//
// 4. IMU963RA_ATTITUDE_DEFAULT_SAMPLE_RATE_HZ 与 IMU963RA_ATTITUDE_UPDATE_PERIOD_US
//    这两个参数分别对应姿态解算频率与 PIT 中断周期，一般需要保持匹配。
#define IMU963RA_ATTITUDE_DEFAULT_SAMPLE_RATE_HZ      (208.0f)
#define IMU963RA_ATTITUDE_UPDATE_PERIOD_US            (4808U)
#define IMU963RA_ATTITUDE_UPDATE_PERIOD_MS            (5U)
#define IMU963RA_ATTITUDE_GYRO_SETTLE_DELAY_MS        (1000U)
#define IMU963RA_ATTITUDE_GYRO_CALIBRATE_WARMUP_COUNT (8U)
#define IMU963RA_ATTITUDE_GYRO_CALIBRATE_COUNT        (50U)
#define IMU963RA_ATTITUDE_GYRO_CALIBRATE_DELAY_MS     (IMU963RA_ATTITUDE_UPDATE_PERIOD_MS)
#define IMU963RA_ATTITUDE_GYRO_Z_SCALE_TRIM           (0.9861f)

#define IMU963RA_ATTITUDE_ACC_X(value)                (value)
#define IMU963RA_ATTITUDE_ACC_Y(value)                (value)
#define IMU963RA_ATTITUDE_ACC_Z(value)                (value)

#define IMU963RA_ATTITUDE_GYRO_X(value)               (value)
#define IMU963RA_ATTITUDE_GYRO_Y(value)               (value)
#define IMU963RA_ATTITUDE_GYRO_Z(value)               ((value) * IMU963RA_ATTITUDE_GYRO_Z_SCALE_TRIM)

typedef struct
{
    float roll;
    float pitch;
    float yaw;
} imu963ra_attitude_angle_struct;

extern float imu963ra_attitude_roll;
extern float imu963ra_attitude_pitch;
extern float imu963ra_attitude_yaw;

void  imu963ra_attitude_init(float sample_rate_hz);
void  imu963ra_attitude_update(void);
uint8 imu963ra_attitude_is_ready(void);

// 参数说明     angle           用于接收姿态角的结构体指针，不能为空指针
// 推荐用法     imu963ra_attitude_angle_struct angle;
//              imu963ra_attitude_get_euler(&angle);
//              然后直接读取 angle.roll angle.pitch angle.yaw 即可，单位为度
void  imu963ra_attitude_get_euler(imu963ra_attitude_angle_struct *angle);

#endif
