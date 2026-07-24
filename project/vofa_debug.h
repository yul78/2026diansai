#ifndef __VOFA_DEBUG_H
#define __VOFA_DEBUG_H

#include "ti_msp_dl_config.h"

void Uart_DebugSendString(const char *str);
void Uart_DebugSendHexByte(uint8_t value);

#endif
