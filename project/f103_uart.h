#ifndef F103_TRANS_H
#define F103_TRANS_H

#include <stdint.h>

/*
 * MSPM0发送给F103的Y轴加速度数据帧，共4字节：
 *
 * [0]  0xAA                 帧头
 * [1]  acc_y低字节          单位mg，int16_t，小端
 * [2]  acc_y高字节
 * [3]  checksum             前3字节的异或校验值
 *
 * 校验计算：
 * checksum = 0xAA ^ acc_y_low ^ acc_y_high;
 */
#define F103_TRANS_FRAME_HEADER  0xAAU

/*
 * 发送已经换算成mg的Y轴加速度。
 */
void F103_Trans_SendAccelerationMg(int16_t acc_y_mg);

/*
 * 读取一次IMU963加速度，只使用Y轴，
 * 不读取陀螺仪，也不进行姿态角解算，然后发送给F103。
 */
void F103_Trans_SendImu963Acceleration(void);

#endif