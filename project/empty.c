#include "ti_msp_dl_config.h"
#include "encoder.h"
#include "DL_OLED.h"
#include "bluetooth.h"
#include "interrupt.h"
#include "key.h"
#include "delay.h"
#include "imu963/zf_device_imu963ra.h"
#include "imu963/imu963ra_attitude.h"
#include "vofa_debug.h"
#include <stdio.h>
#include "at8236.h"
#include "angle_ctrl.h"

#define ANGLE_CTRL_BASE_SPEED          0
#define ANGLE_CTRL_DEBUG_DIVIDER       10U

int32_t l_speed_now, r_speed_now;
static uint8_t imu963ra_ready;
static uint8_t imu963ra_init_state;
static uint8_t angle_ctrl_ready;
imu963ra_attitude_angle_struct imu963_angle;

#define IMU963RA_OLED_ROWS             (4U)
#define IMU963RA_OLED_COLUMNS          (16U)
#define IMU963RA_ANGLE_COLUMN          (6U)
#define IMU963RA_ANGLE_TEXT_LENGTH     (6U)

void TimerTick_Init(void);

int main(void)
{
    SYSCFG_DL_init();
    AJ_Init();

    imu963ra_init_state = imu963ra_init();
    imu963ra_ready = (imu963ra_init_state == IMU963RA_INIT_OK);
    if (imu963ra_ready) {
        imu963ra_attitude_init(100.0f);
    }

    while(1)
    {
        if(AJ1_IsPressed())
            break;
    }

    AT8236_Init();
    AT8236_PID_Init();
    AngleCtrl_Init();
    AngleCtrl_SetTarget(0.0f);
    angle_ctrl_ready = 1U;
    Encoder_Init();
    OLED_Init();

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

        // static int16_t left_speed_load = 600, right_speed_load = 600;
        // OLED_ShowSignedNum(1,1,left_speed,4);
        // OLED_ShowSignedNum(2,1,right_speed,4);

        //Xunji_Task(&left_speed_load, &right_speed_load);
        //TB6612_SetMotors(left_speed_load, right_speed_load);
        //AT8236_SetMotors(left_speed_load, right_speed_load);
        
        /***********************************/

        Bluetooth_Task();

        /***************IMU963部分********************/
        static uint32_t last_imu_update_ms = 0U;
        static uint8_t angle_ctrl_debug_count = 0U;
        if (imu963ra_ready && imu963ra_update_flag) {
            uint32_t now_ms;
            uint32_t elapsed_ms;
            __disable_irq();
            imu963ra_update_flag = 0U;
            now_ms = imu963ra_tick_ms;
            __enable_irq();

            elapsed_ms = now_ms - last_imu_update_ms;
            last_imu_update_ms = now_ms;
            imu963ra_attitude_update_with_delta_time((float)elapsed_ms * 0.001f);
            imu963ra_attitude_get_euler(&imu963_angle);

            AngleCtrl_UpdateMotors(ANGLE_CTRL_BASE_SPEED, imu963_angle.yaw);

            angle_ctrl_debug_count++;
            if (angle_ctrl_debug_count >= ANGLE_CTRL_DEBUG_DIVIDER) {
                int32_t targetA;
                int32_t targetB;
                int32_t speedA;
                int32_t speedB;
                int32_t outputA;
                int32_t outputB;
                char str[140];

                angle_ctrl_debug_count = 0U;
                __disable_irq();
                targetA = AT8236_PIDA.target_speed;
                targetB = AT8236_PIDB.target_speed;
                speedA = AT8236_PIDA.measured_speed;
                speedB = AT8236_PIDB.measured_speed;
                outputA = AT8236_PIDA.output;
                outputB = AT8236_PIDB.output;
                __enable_irq();

                sprintf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
                    (int)angle_ctrl_ready,
                    (int)AngleCtrl_PID.target_angle,
                    (int)AngleCtrl_PID.measured_angle,
                    (int)AngleCtrl_PID.error,
                    (int)AngleCtrl_PID.output,
                    (int)AngleCtrl_PID.integral,
                    (int)targetA,
                    (int)targetB,
                    (int)speedA,
                    (int)speedB,
                    (int)outputA,
                    (int)outputB);
                Uart_BlueSendString(str);
            }
        }

        // OLED_ShowFloat(1, 1, imu963_angle.pitch,2, 2);
        // OLED_ShowFloat(2, 1, imu963_angle.roll,2, 2);
        // OLED_ShowFloat(3, 1, imu963_angle.yaw,2, 2);

        /*********************************************/

        
        // OLED is refreshed with the PID debug frame above.
        //OLED_ShowSignedNum(2, 1, Encoder_GetCountA(), 4);
        //OLED_ShowSignedNum(3, 1, Encoder_GetCountB(), 4);

         /*********调试速度闭环PID用********/
        // if(at8236PID_update_flag)
        // {
        //     int32_t targetA;
        //     int32_t measuredA;
        //     int32_t outputA;
        //     int32_t errorA;
        //     int32_t integralA;
        //     int32_t targetB;
        //     int32_t measuredB;
        //     int32_t outputB;
        //     int32_t errorB;
        //     int32_t integralB;

        //     __disable_irq();
        //     at8236PID_update_flag = 0U;
        //     targetA = AT8236_PIDA.target_speed;
        //     measuredA = AT8236_PIDA.measured_speed;
        //     outputA = AT8236_PIDA.output;
        //     errorA = AT8236_PIDA.error;
        //     integralA = AT8236_PIDA.integral;
        //     targetB = AT8236_PIDB.target_speed;
        //     measuredB = AT8236_PIDB.measured_speed;
        //     outputB = AT8236_PIDB.output;
        //     errorB = AT8236_PIDB.error;
        //     integralB = AT8236_PIDB.integral;
        //     __enable_irq();

        //     char str[100];

        //     sprintf(str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n",
        //         (int)targetA,
        //         (int)measuredA,
        //         (int)outputA,
        //         (int)errorA,
        //         (int)integralA,
        //         (int)targetB,
        //         (int)measuredB,
        //         (int)outputB,
        //         (int)errorB,
        //         (int)integralB);
        //     Uart_BlueSendString(str);

        //     OLED_ShowSignedNum(2, 1, measuredA, 4);
        //     OLED_ShowSignedNum(3, 1, measuredA, 4);
        // }
        /************************************/
    }
    
   
}

void TimerTick_Init(void)
{
    DL_TimerG_clearInterruptStatus(TIMER_TICK_INST, DL_TIMER_IIDX_LOAD);
    NVIC_EnableIRQ(TIMER_TICK_INST_INT_IRQN);
    DL_TimerG_startCounter(TIMER_TICK_INST);
}
