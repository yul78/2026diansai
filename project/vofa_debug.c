#include "vofa_debug.h"

/**
 * @brief 通过串口发送一个字符串
 * @param str 要发送的字符串指针
 */
void Uart_DebugSendString(const char *str) {
    // 循环遍历字符串，直到遇到结束符 '\0'
    while (*str != '\0') {
        // 阻塞式发送当前字符
        // 该函数会等待直到数据被发送到移位寄存器
        DL_UART_Main_transmitDataBlocking(UART_DEBUG_INST, (uint8_t)*str);
        str++; // 指向下一个字符
    }
}

void Uart_DebugSendHexByte(uint8_t value) {
    static const char hex[] = "0123456789ABCDEF";
    char text[3];

    text[0] = hex[(value >> 4) & 0x0FU];
    text[1] = hex[value & 0x0FU];
    text[2] = '\0';
    Uart_DebugSendString(text);
}
