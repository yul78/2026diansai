#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include "encoder.h"
#include "ti_msp_dl_config.h"
#include "vofa_debug.h"

void Encoder_IRQHandler(void);

extern int32_t l_speed_now, r_speed_now;
extern volatile uint8_t imu963ra_update_flag; 
// extern volatile uint8_t imu963ra_display_flag;  // 陀螺仪已停用
// extern volatile uint32_t imu963ra_tick_ms;      // 陀螺仪已停用
extern volatile uint8_t at8236PID_update_flag;
/*
 * 每10ms置1一次，主循环看到该标志后执行一次第二问循迹PID。
 * 这里只置标志，不在中断里读取循迹模块或发送串口，避免中断执行时间过长。
 */
extern volatile uint8_t xunji_update_flag;
/* XJ4、XJ5、XJ6检测到停车线后由10ms中断锁存为1。 */
extern volatile uint8_t q2_stop_line_flag;
/* 第五、六问检测到A点横向启停线后，由10ms中断锁存。 */
extern volatile uint8_t q56_pass_line_flag;
/* 检测到A线时的system_tick_ms，用于计算准确的通过A点时间。 */
extern volatile uint32_t q56_pass_line_tick_ms;
/* system_tick_ms由1ms定时器中断累加，供题目运行计时使用。 */
extern volatile uint32_t system_tick_ms;

#endif
