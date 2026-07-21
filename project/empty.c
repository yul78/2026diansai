#include "ti_msp_dl_config.h"
#include "tb6612.h"
#include "encoder.h"
#include "DL_OLED.h"
#include "app_jy901.h"
#include "bluetooth.h"
#include "myxunji.h"
#include "interrupt.h"
#include "key.h"

int32_t l_speed_now, r_speed_now;

void TimerTick_Init(void);

int main(void)
{
    SYSCFG_DL_init();
    TB6612_Init();
    Encoder_Init();
    OLED_Init();
    APP_JY901_Init();
    Bluetooth_Init();
    TimerTick_Init();
    
    while (1) {


        /**************按键测试************ */

        if(AJ1_IsPressed())
        {

        }
        if(AJ2_IsPressed())
        {

        }
        if(AJ3_IsPressed())
        {

        }
        if(AJ4_IsPressed())
        {

        }
        

        /*************循迹测试**************/

        static int16_t left_speed_load = 0, right_speed_load = 0;
        // OLED_ShowSignedNum(1,1,left_speed,4);
        // OLED_ShowSignedNum(2,1,right_speed,4);

        //Xunji_Task(&left_speed_load, &right_speed_load);
        TB6612_SetMotors(left_speed_load, right_speed_load);
        
        /***********************************/

        Bluetooth_Task();
        JY901_Task();
        
        // OLED_ShowSignedNum(2, 1, l_speed_now, 4);
        // OLED_ShowSignedNum(3, 1, r_speed_now, 4);
        //OLED_ShowSignedNum(2, 1, Encoder_GetCountA(), 4);
        //OLED_ShowSignedNum(3, 1, Encoder_GetCountB(), 4);
        // OLED_ShowFloat(1, 3, jy901_data.roll,  3, 1);
        // OLED_ShowFloat(2, 3, jy901_data.pitch, 3, 1);
        // OLED_ShowFloat(3, 3, jy901_data.yaw,   3, 1);
    }
}

void TimerTick_Init(void)
{
    DL_TimerG_clearInterruptStatus(TIMER_TICK_INST, DL_TIMER_IIDX_LOAD);
    NVIC_EnableIRQ(TIMER_TICK_INST_INT_IRQN);
    DL_TimerG_startCounter(TIMER_TICK_INST);
}

