#include "ti_msp_dl_config.h"
#include "tb6612.h"
#include "encoder.h"
#include "DL_OLED.h"
#include "app_jy901.h"
#include "bluetooth.h"
#include "myxunji.h"

#define XUNJI_FORWARD_SPEED    1500
#define XUNJI_TURN_SLOW_SPEED  0
#define XUNJI_TURN_FAST_SPEED  2000


/********************使用滴答定时器需要做的配置如下******************** */
#define SYSTICK_1MS  80000  // 系统时钟为 80MHz 时，SysTick 每 1ms 中断一次的计数值
volatile uint64_t g_systick_ms = 0; // 全局毫秒计数器，使用 volatile 防止编译器优化
void SystemClock_Config(void) {
    // ... 你的其他时钟配置代码 ...

    // 配置 SysTick 周期为 1ms，并使能中断
    DL_SYSTICK_config(SYSTICK_1MS);
}
// SysTick 中断服务函数
void SysTick_Handler(void) {
    // 每 1ms 计数器加 1
    g_systick_ms++;
}
/************************************************************************ */

int main(void)
{
    SYSCFG_DL_init();
    TB6612_Init();
    Encoder_Init();
    OLED_Init();
    APP_JY901_Init();
    Bluetooth_Init();

    

    DL_GPIO_clearInterruptStatus(AJ_PORT, AJ_AJ1_PIN);
    DL_GPIO_enableInterrupt(AJ_PORT, AJ_AJ1_PIN);
    NVIC_EnableIRQ(AJ_INT_IRQN);

    /****************使用滴答定时器定时执行任务************** */
    SystemClock_Config();
    uint64_t last_task_5s = 0;
    const uint64_t INTERVAL_5S = 5000;   // 5秒 = 5000ms
    /******************************************************* */

    while (1) {

        uint64_t current_ms = g_systick_ms;
        if (current_ms - last_task_5s >= INTERVAL_5S) {
            last_task_5s = current_ms;
            int16_t test1 = 0;
            test1++;
            OLED_ShowNum(3,2,test1,4);
        }
        
        /**************按键测试************ */


        /*************循迹测试**************/
        static int16_t left_speed = 0, right_speed = 0;
        
        OLED_ShowSignedNum(1,1,left_speed,4);
        OLED_ShowSignedNum(2,1,right_speed,4);

        Xunji_Task(&left_speed, &right_speed);
        TB6612_SetMotors(left_speed, right_speed);
        
        //uint8_t bits = Xunji_Read_Hardware();
        // OLED_ShowNum(1, 1, Xunji_Read_Hardware(0), 1);
        // OLED_ShowNum(1, 2, Xunji_Read_Hardware(1), 1);
        // OLED_ShowNum(1, 3, Xunji_Read_Hardware(2), 1);
        // OLED_ShowNum(1, 4, Xunji_Read_Hardware(3), 1);
        // OLED_ShowNum(1, 5, Xunji_Read_Hardware(4), 1);
        // OLED_ShowNum(1, 6, Xunji_Read_Hardware(5), 1);
        // OLED_ShowNum(1, 7, Xunji_Read_Hardware(6), 1);
        // OLED_ShowNum(1, 8, Xunji_Read_Hardware(7), 1);
        

        Bluetooth_Task();
        JY901_Task();
        //TB6612_SetMotors(500, 500);

        // OLED_ShowSignedNum(2, 1, Encoder_GetCountA(), 7);
        // OLED_ShowSignedNum(3, 1, Encoder_GetCountB(), 7);
        // OLED_ShowFloat(1, 3, jy901_data.roll,  3, 1);
        // OLED_ShowFloat(2, 3, jy901_data.pitch, 3, 1);
        // OLED_ShowFloat(3, 3, jy901_data.yaw,   3, 1);
    }
}

// 

