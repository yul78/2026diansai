#include "ti_msp_dl_config.h"
#include "encoder.h"
#include "DL_OLED.h"
#include "bluetooth.h"
#include "interrupt.h"
#include "key.h"
#include "delay.h"
#include "vofa_debug.h"
#include "imu963/zf_device_imu963ra.h"
#include <stdio.h>
#include "at8236.h"
// #include "angle_ctrl.h"      // 陀螺仪航向控制已停用，文件保留备用
#include "myxunji.h"
#include "f103_uart.h"

int32_t l_speed_now, r_speed_now;
volatile uint8_t question2_running_flag = 0;
uint8_t question4_running_flag = 0;
uint8_t question5_running_flag = 0;
uint8_t question_number = 1;
uint8_t question_running_flag = 0;

/*
 * 所有题目共用这一套计时变量，单位均为毫秒。
 * 每次上电只执行一问，因此不需要为每一问分别保存一套时间。
 */
uint32_t run_start_ms = 0U;
uint32_t run_elapsed_ms = 0U;
uint32_t run_last_oled_ms = 0U;


uint8_t imu963ra_acc_ready = 0U;

void TimerTick_Init(void);

int main(void)
{
    SYSCFG_DL_init();
    AJ_Init();

    AT8236_Init();
    AT8236_PID_Init();
    
    Encoder_Init();
    OLED_Init();

    Bluetooth_Init();
    TimerTick_Init();

    if(imu963ra_init() == IMU963RA_INIT_OK)
    {
        imu963ra_acc_ready = 1U;
    }
    
    OLED_ShowString(1, 1, "Q num:");
    OLED_ShowString(2, 1, "T(ms):");
    OLED_ShowNum(2, 7, run_elapsed_ms, 6);

    while (1) {
        DL_UART_Main_transmitDataBlocking(UART_DEBUG_INST, 0x02);
        /* 本轮主循环只读取一次系统时间，单位为毫秒。 */
        uint32_t now_ms = system_tick_ms;

        if(imu963ra_update_flag)
        {
            imu963ra_update_flag = 0U;
            if(imu963ra_acc_ready)
            {
                F103_Trans_SendImu963Acceleration();
            }
        }

        OLED_ShowNum(1,8,question_number,1);

        /**************按键测试************ */

        /* 题目启动后锁定题号，防止运行过程中误按AJ1改变题目。 */
        if((question_running_flag == 0U) && AJ1_IsPressed())
        {
            question_number++;
            if(question_number > 6) question_number = 1;
        }
        if(AJ2_IsPressed())
        {
            /* 任意题目都在按下AJ2时使用同一套变量从0开始计时。 */
            if(question_running_flag == 0U)
            {
                question_running_flag = 1U;
                run_start_ms = now_ms;
                run_elapsed_ms = 0U;
                run_last_oled_ms = 0U;
                OLED_ShowNum(2, 7, run_elapsed_ms, 6);

                switch(question_number)
                {
                    case 2:
                        q2_stop_line_flag = 0U;
                        question2_running_flag = 1U;
                        Xunji_Q2_Reset();
                        /* 启动后立即计算一次，不必等待下一个10ms节拍。 */
                        xunji_update_flag = 1U;
                        break;
                    case 4:
                        question4_running_flag = 1U;
                        break;
                    case 5:
                        question5_running_flag = 1U;
                        break;
                    default:
                        /* 第1、3、6问实现后在这里加入各自的启动标志。 */
                        break;
                }
            }

        }
        if(AJ3_IsPressed())
        {

        }
        if(AJ4_IsPressed())
        {

        }

        /***************所有题目共用的计时与显示***************/
        if(question_running_flag)
        {
            run_elapsed_ms = now_ms - run_start_ms;

            /* 每50ms刷新一次OLED，避免频繁刷屏影响主循环。 */
            if((run_elapsed_ms - run_last_oled_ms) >= 50U)
            {
                run_last_oled_ms = run_elapsed_ms;
                OLED_ShowNum(2, 7, run_elapsed_ms, 6);
            }
        }
    
        /***************题目运行**************/
        //第二问
        if(question2_running_flag)
        {
            static int16_t left_speed_load = 0, right_speed_load = 0;

            /*
             * 10ms中断已经检测到停车线并停止电机时，在这里结束计时和运行。
             * 标志会一直保持为1，因此即使主循环正在发送串口也不会漏掉。
             */
            if(q2_stop_line_flag)
            {
                question2_running_flag = 0U;
                question_running_flag = 0U;
                AT8236_PID_Stop();
                OLED_ShowNum(2, 7, run_elapsed_ms, 6);
                
            }

            /*
             * 外层循迹PID每10ms只计算左右轮目标速度；
             * 内层轮速PID仍在定时器中断中用编码器实测速度完成速度闭环。
             */
            if(question2_running_flag && xunji_update_flag)
            {
                uint8_t stop_line_found;

                xunji_update_flag = 0U;
                stop_line_found = Xunji_Q2_Task(&left_speed_load, &right_speed_load);

                if(stop_line_found)
                {

                    question2_running_flag = 0U;
                    question_running_flag = 0U;
                    AT8236_PID_Stop();
                    AT8236_Stop();
                    OLED_ShowNum(2, 7, run_elapsed_ms, 6);

                    
                }
                else
                {
                    /* 这里只装载目标速度，不直接装载PWM。 */
                    if(q2_stop_line_flag == 0U)
                    {
                        AT8236_PID_SetTargets(left_speed_load, right_speed_load);
                    }
                }
            }

        } 
        //第四问
        if(question4_running_flag)
        {

        }
        //第五问
        if(question5_running_flag)
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
