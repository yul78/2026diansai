#include "bluetooth.h"
#include "tb6612.h"

/*
    图形化配置说明
    .syscfg中串口命名为 UART_BT
*/

#define BT_DEFAULT_SPEED 2500

static volatile uint8_t bt_cmd;
static volatile uint8_t bt_cmd_ready;

void Bluetooth_Init(void)
{
    bt_cmd       = 0;
    bt_cmd_ready = 0;

    DL_UART_Main_enableFIFOs(UART_BT_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_BT_INST,
        DL_UART_RX_FIFO_LEVEL_ONE_ENTRY);
    DL_UART_Main_enableInterrupt(UART_BT_INST,
        DL_UART_MAIN_INTERRUPT_RX);

    NVIC_EnableIRQ(UART_BT_INST_INT_IRQN);
}

void Bluetooth_Task(void)
{
    uint8_t cmd;

    if (!bt_cmd_ready) {
        return;
    }

    cmd          = bt_cmd;
    bt_cmd_ready = 0;

    switch (cmd) {
    case 'F':
    case 'f':
        TB6612_SetMotors(BT_DEFAULT_SPEED, BT_DEFAULT_SPEED);
        break;
    case 'B':
    case 'b':
        TB6612_SetMotors(-BT_DEFAULT_SPEED, -BT_DEFAULT_SPEED);
        break;
    case 'L':
    case 'l':
        TB6612_SetMotors(-BT_DEFAULT_SPEED, BT_DEFAULT_SPEED);
        break;
    case 'R':
    case 'r':
        TB6612_SetMotors(BT_DEFAULT_SPEED, -BT_DEFAULT_SPEED);
        break;
    case 'S':
    case 's':
        TB6612_Stop();
        break;
    default:
        break;
    }
}

void UART_BT_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_BT_INST)) {
    case DL_UART_MAIN_IIDX_RX:
        bt_cmd       = (uint8_t)DL_UART_Main_receiveData(UART_BT_INST);
        bt_cmd_ready = 1;
        break;

    default:
        break;
    }
}
