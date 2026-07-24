#include "ti_msp_dl_config.h"
#include "tb6612.h"
#include "encoder.h"
#include "DL_OLED.h"
#include "bluetooth.h"
#include "myxunji.h"
#include "interrupt.h"
#include "key.h"
#include "delay.h"
#include "imu963/zf_device_imu963ra.h"
#include "imu963/imu963ra_attitude.h"
#include "vofa_debug.h"
#include <stdio.h>

int32_t l_speed_now, r_speed_now;
static uint8_t imu963ra_ready;
static uint8_t imu963ra_init_state;
imu963ra_attitude_angle_struct imu963_angle;

#define IMU963RA_OLED_ROWS             (4U)
#define IMU963RA_OLED_COLUMNS          (16U)
#define IMU963RA_ANGLE_COLUMN          (6U)
#define IMU963RA_ANGLE_TEXT_LENGTH     (6U)



void TimerTick_Init(void);


int main(void)
{
    SYSCFG_DL_init();
    TB6612_Init();
    Encoder_Init();
    OLED_Init();

    imu963ra_init_state = imu963ra_init();
    imu963ra_ready = (imu963ra_init_state == IMU963RA_INIT_OK);
    OLED_Clear();
    if (imu963ra_ready) {
        imu963ra_attitude_init(100.0f);
    }
    
    Bluetooth_Init();
    TimerTick_Init();
    
    while (1) {
        char str[50];
        sprintf(str, "pitch:%.2f roll:%.2f yaw:%.2f\r\n", imu963_angle.pitch, imu963_angle.roll, imu963_angle.yaw);
        Uart_DebugSendString(str);
        //Uart_DebugSendAngle();

        static uint32_t last_imu_update_ms = 0U;
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

        if (imu963ra_ready && imu963ra_update_flag) {
            imu963ra_attitude_get_euler(&imu963_angle);


            uint32_t now_ms;
            uint32_t elapsed_ms;

            __disable_irq();
            imu963ra_update_flag = 0U;
            now_ms = imu963ra_tick_ms;
            __enable_irq();

            elapsed_ms = now_ms - last_imu_update_ms;
            last_imu_update_ms = now_ms;
            imu963ra_attitude_update_with_delta_time((float)elapsed_ms * 0.001f);
        }

        
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
