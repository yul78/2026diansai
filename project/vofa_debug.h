#ifndef __VOFA_DEBUG_H
#define __VOFA_DEBUG_H

#include "ti_msp_dl_config.h"
// #include "imu963/imu963ra_attitude.h"  // 陀螺仪已停用
#include <stdio.h>

void Uart_DebugSendString(const char *str);
void Uart_DebugSendHexByte(uint8_t value);
// void Uart_DebugSendAngle(void);         // 陀螺仪角度调试已停用
void Uart_BlueSendString(const char *str);

#endif
