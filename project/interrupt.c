#include "interrupt.h"

volatile uint8_t imu963ra_update_flag = 0U;
volatile uint8_t imu963ra_display_flag = 0U;
volatile uint32_t imu963ra_tick_ms = 0U;

extern uint8_t AJ1_pressed;
extern uint8_t AJ2_pressed;
extern uint8_t AJ3_pressed;
extern uint8_t AJ4_pressed;

void GROUP1_IRQHandler(void)
{
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

    Encoder_IRQHandler();
}

void TIMER_TICK_INST_IRQHandler(void)
{
    // 检查是否触发的中断
    if (DL_TimerG_getPendingInterrupt(TIMER_TICK_INST) == DL_TIMER_IIDX_LOAD) {
        // 清除中断标志
        DL_TimerG_clearInterruptStatus(TIMER_TICK_INST, DL_TIMER_IIDX_LOAD);
        
        // 在这里执行你的 1ms 任务
        // 例如：软件计数器累加
        static uint8_t tick_10ms = 0U;
        static uint8_t tick_100ms = 0U;

        imu963ra_tick_ms++;
        tick_10ms++;
        tick_100ms++;
        if (tick_10ms >= 10U) {
            tick_10ms = 0U;
            // 每 10ms 执行一次的任务
            Encoder_GetSpeeds(&l_speed_now, &r_speed_now);
            imu963ra_update_flag = 1U;
        }
        if (tick_100ms >= 100U) {
            tick_100ms = 0U;
        }
    }
}
