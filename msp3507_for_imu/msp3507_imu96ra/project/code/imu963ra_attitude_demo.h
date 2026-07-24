#ifndef IMU963RA_ATTITUDE_DEMO_H
#define IMU963RA_ATTITUDE_DEMO_H

#include "zf_common_typedef.h"
#include "imu963ra_attitude.h"

// 模块内维护的姿态角结构体
// 用户可以在自己的工程里直接读取这三个成员
// imu963ra_attitude_demo_angle.roll
// imu963ra_attitude_demo_angle.pitch
// imu963ra_attitude_demo_angle.yaw
extern imu963ra_attitude_angle_struct imu963ra_attitude_demo_angle;

uint8 imu963ra_attitude_demo_init(void);
void  imu963ra_attitude_demo_task(void);
void  imu963ra_attitude_demo_pit_handler(uint32 event, void *ptr);
uint8 imu963ra_attitude_demo_is_ready(void);

#endif