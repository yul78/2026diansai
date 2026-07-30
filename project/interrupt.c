#include "interrupt.h"
#include "at8236.h"
#include "myxunji.h"

#define AT8236_PID_DEBUG_TICKS 10U

volatile uint8_t imu963ra_update_flag = 0U;  
volatile uint8_t at8236PID_update_flag = 0U;
// volatile uint8_t imu963ra_display_flag = 0U;  // 陀螺仪已停用
volatile uint8_t xunji_update_flag = 0U;
volatile uint8_t q2_stop_line_flag = 0U;
// volatile uint32_t imu963ra_tick_ms = 0U;      // 陀螺仪已停用
volatile uint32_t system_tick_ms = 0U;

extern uint8_t AJ1_pressed;
extern uint8_t AJ2_pressed;
extern uint8_t AJ3_pressed;
extern uint8_t AJ4_pressed;
extern volatile uint8_t question2_running_flag;

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
        static uint8_t tick_15ms = 0U;        // 原陀螺仪更新节拍，现已停用
        //static uint8_t at8236_pid_debug_tick = 0U;

        // imu963ra_tick_ms++;                   // 陀螺仪已停用
        system_tick_ms++;
        tick_10ms++;
        tick_15ms++;                          // 陀螺仪已停用
        if (tick_10ms >= 10U) {
            tick_10ms = 0U;
            // 每 10ms 执行一次的任务
            Encoder_GetSpeeds(&l_speed_now, &r_speed_now);
            AT8236_PID_Update(l_speed_now, r_speed_now);

            /*
             * 串口发送会阻塞主循环几十毫秒，所以停车线必须在固定10ms中断中检测。
             * 一旦XJ4、XJ5、XJ6同时扫到黑线，立即锁存标志并停止电机。
             */
            if((question2_running_flag != 0U) &&
               (q2_stop_line_flag == 0U) &&
               Xunji_Q2_StopLineDetected())
            {
                q2_stop_line_flag = 1U;
                AT8236_PID_Stop();
            }

            /* 通知主循环：现在可以计算一次新的循迹目标速度。 */
            xunji_update_flag = 1U;
            
            // at8236_pid_debug_tick++;
            // if (at8236_pid_debug_tick >= AT8236_PID_DEBUG_TICKS) {
            //     at8236_pid_debug_tick = 0U;
            //     at8236PID_update_flag = 1U;
            // }
        }
        if (tick_15ms >= 15U) {
            tick_15ms = 0U;
            imu963ra_update_flag = 1U;
        }
    }
}
