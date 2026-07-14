#include "bsp_jy901.h"

/*
    图形化配置说明
    .syscfg中串口命名为 UART_JY901
*/

jy901_uart_t jy901_uart;

void BSP_JY901_Init(void)
{
    jy901_uart.head = 0;
    jy901_uart.tail = 0;
    jy901_uart.overflow = 0;

    DL_UART_Main_disableDMAReceiveEvent(UART_JY901_INST, DL_UART_DMA_INTERRUPT_RX);
    DL_UART_Main_disableInterrupt(UART_JY901_INST,DL_UART_MAIN_INTERRUPT_DMA_DONE_RX | DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);
    DL_UART_Main_clearInterruptStatus(UART_JY901_INST,DL_UART_MAIN_INTERRUPT_DMA_DONE_RX | DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR);

    DL_UART_Main_enableFIFOs(UART_JY901_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_JY901_INST, DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_enableInterrupt(UART_JY901_INST, DL_UART_MAIN_INTERRUPT_RX);

    NVIC_EnableIRQ(UART_JY901_INST_INT_IRQN);
}

void BSP_JY901_PushByte(uint8_t byte)
{
    uint16_t next = (uint16_t)((jy901_uart.head + 1U) & (JY_Buffer_Size - 1U));

    if (next == jy901_uart.tail)
    {
        jy901_uart.tail = (uint16_t)((jy901_uart.tail + 1U) & (JY_Buffer_Size - 1U));
        jy901_uart.overflow = 1;
    }

    jy901_uart.buffer[jy901_uart.head] = byte;
    jy901_uart.head = next;
}

uint8_t BSP_JY901_ReadByte(uint8_t *byte)
{
    if (jy901_uart.tail == jy901_uart.head)
    {
        return 0;
    }

    *byte = jy901_uart.buffer[jy901_uart.tail];
    jy901_uart.tail = (uint16_t)((jy901_uart.tail + 1U) & (JY_Buffer_Size - 1U));
    return 1;
}

void UART_JY901_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_JY901_INST))
    {
        case DL_UART_MAIN_IIDX_RX:
            BSP_JY901_PushByte((uint8_t)DL_UART_Main_receiveData(UART_JY901_INST));
            break;

        default:
            break;
    }
}