#ifndef F103_TRANS_H
#define F103_TRANS_H

#include <stdint.h>

/*
 * MSPM0发送给F103的固定数据帧，共6字节：
 *
 * [0]  0xAA                 帧头
 * [1]  question_number      当前题号，只发送第3～6题
 * [2]  target_position_mm   第六问指定位置0～250mm，其他题固定为0
 * [3]  acc_y低字节          单位mg，int16_t，小端
 * [4]  acc_y高字节
 * [5]  checksum             前5字节的异或校验值
 *
 * 校验计算：
 * checksum = 0xAA ^ question_number ^ target_position_mm
 *          ^ acc_y_low ^ acc_y_high;
 */
#define F103_TRANS_FRAME_HEADER  0xAAU

/*
 * 唯一对外发送接口：读取IMU963的Y轴加速度，并将题号、指定位置和
 * 加速度一起发送给F103。非第六问会在函数内部自动把指定位置清零。
 */
void F103_Trans_SendData(
    uint8_t question_number, uint8_t target_position_mm);

#endif
