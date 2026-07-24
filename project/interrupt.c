#include "interrupt.h"

extern uint8_t AJ1_pressed;
extern uint8_t AJ2_pressed;
extern uint8_t AJ3_pressed;
extern uint8_t AJ4_pressed;

void GROUP1_IRQHandler(void)
{
    // 分别获取两个端口的中断状态
    uint32_t encoder_status = DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT,ENCODER_MOTOR_A_A_PIN | ENCODER_MOTOR_B_A_PIN);
    uint32_t aj_status_portA = DL_GPIO_getEnabledInterruptStatus(GPIOA, AJ_AJ1_PIN | AJ_AJ3_PIN);  // AJ1、AJ3 在 GPIOA
    uint32_t aj_status_portB = DL_GPIO_getEnabledInterruptStatus(GPIOB, AJ_AJ2_PIN | AJ_AJ4_PIN);  // AJ2、AJ4 在 GPIOB

    // 处理按键中断
    if (aj_status_portA & AJ_AJ1_PIN) {
        AJ1_pressed = 1;
        DL_GPIO_clearInterruptStatus(AJ_AJ1_PORT, AJ_AJ1_PIN);
    }
    if (aj_status_portB & AJ_AJ2_PIN) {
        AJ2_pressed = 1;
        DL_GPIO_clearInterruptStatus(AJ_AJ2_PORT, AJ_AJ2_PIN);
    }
    if (aj_status_portA & AJ_AJ3_PIN) {
        AJ3_pressed = 1;
        DL_GPIO_clearInterruptStatus(AJ_AJ3_PORT, AJ_AJ3_PIN);
    }
    if (aj_status_portB & AJ_AJ4_PIN) {
        AJ4_pressed = 1;
        DL_GPIO_clearInterruptStatus(AJ_AJ4_PORT, AJ_AJ4_PIN);
    }

    // 处理编码器中断
    if (encoder_status) 
    {
        if (encoder_status & ENCODER_MOTOR_A_A_PIN) {
            if (DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_A_B_PIN)) {
                EncoderA.count -= EncoderA.polarity;
            } else {
                EncoderA.count += EncoderA.polarity;
            }
        }

        if (encoder_status & ENCODER_MOTOR_B_A_PIN) {
            if (DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_B_B_PIN)) {
                EncoderB.count -= EncoderB.polarity;
            } else {
                EncoderB.count += EncoderB.polarity;
            }
        }
            DL_GPIO_clearInterruptStatus(ENCODER_PORT, encoder_status);
    }
}

void TIMER_TICK_INST_IRQHandler(void)
{
    // 检查是否触发的中断
    if (DL_TimerG_getPendingInterrupt(TIMER_TICK_INST) == DL_TIMER_IIDX_LOAD) {
        // 清除中断标志
        DL_TimerG_clearInterruptStatus(TIMER_TICK_INST, DL_TIMER_IIDX_LOAD);
        
        // 在这里执行你的 1ms 任务
        // 例如：软件计数器累加
        static uint32_t tick = 0;
        tick++;
        if (tick >= 10) {
            tick = 0;
            // 每 10ms 执行一次的任务
            Encoder_GetSpeeds(&l_speed_now, &r_speed_now);
        }
    }
}
