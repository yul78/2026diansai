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

/*
 * 第四问不使用编码器累计距离判断B点。
 * 启动后的前3秒不检查丢线，避免起步位置不准时误判。
 * 3秒后，XJ4、XJ5连续200ms都扫不到黑线，就认为已经通过B点。
 * 本题计时精度要求很低，因此直接记录确认时刻，不回减这200ms。
 * 通过B点后继续等速直行3秒，再开始原来的软停车过程。
 */
#define Q4_B_DETECT_ENABLE_MS       3000U
#define Q4_B_LOST_CONFIRM_MS         200U
#define Q4_AFTER_B_RUN_MS           3000U

int32_t l_speed_now, r_speed_now;
volatile uint8_t question2_running_flag = 0;
uint8_t question3_running_flag = 0;
volatile uint8_t question4_running_flag = 0;
volatile uint8_t question5_running_flag = 0;
volatile uint8_t question6_running_flag = 0;
uint8_t question_number = 1;
uint8_t question_running_flag = 0;
/* 第六问的小球指定位置，单位mm，允许范围0～250。 */
uint8_t ball_target_position_mm = 0U;

/*
 * 所有题目共用这一套计时变量，单位均为毫秒。
 * 每次上电只执行一问，因此不需要为每一问分别保存一套时间。
 */
uint32_t run_start_ms = 0U;
uint32_t run_elapsed_ms = 0U;
uint32_t run_last_oled_ms = 0U;
// uint32_t q4_last_debug_ms = 0U;
uint32_t q4_pass_b_time_ms = 0U;
uint8_t q4_pass_b_flag = 0U;
uint8_t q4_brake_started_flag = 0U;
uint32_t q4_middle_lost_start_ms = 0U;
uint8_t q4_middle_lost_flag = 0U;
uint32_t q56_pass_a_time_ms = 0U;

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
        /* 本轮主循环只读取一次系统时间，单位为毫秒。 */
        uint32_t now_ms = system_tick_ms;

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
                    case 3:
                        /* 按键只置位运行标志，数据发送放在“题目运行”区域。 */
                        question3_running_flag = 1U;
                        break;
                    case 4:
                        /* 清空速度闭环和第四问历史状态，从低速开始软启动。 */
                        AT8236_PID_Stop();
                        Xunji_Q4_Reset();
                        // q4_last_debug_ms = 0U;
                        q4_pass_b_time_ms = 0U;
                        q4_pass_b_flag = 0U;
                        q4_brake_started_flag = 0U;
                        q4_middle_lost_start_ms = 0U;
                        q4_middle_lost_flag = 0U;

                        question4_running_flag = 1U;
                        /* 启动后立即计算一次目标速度，不等待下一个10ms节拍。 */
                        xunji_update_flag = 1U;
                        break;
                    case 5:
                        /*
                         * 第五问使用独立的软启动循迹PID。按键这里只初始化状态
                         * 并置运行标志，循迹和串口发送仍在“题目运行”区域执行。
                         */
                        AT8236_PID_Stop();
                        Xunji_Q5_Reset();
                        q56_pass_a_time_ms = 0U;

                        __disable_irq();
                        q56_pass_line_flag = 0U;
                        q56_pass_line_tick_ms = 0U;
                        __enable_irq();

                        question5_running_flag = 1U;
                        xunji_update_flag = 1U;
                        break;
                    case 6:
                        /*
                         * 第六问的小车循迹与第五问相同，只是发送给从控的
                         * ball_target_position_mm不再固定为0。
                         */
                        AT8236_PID_Stop();
                        Xunji_Q6_Reset();
                        q56_pass_a_time_ms = 0U;

                        __disable_irq();
                        q56_pass_line_flag = 0U;
                        q56_pass_line_tick_ms = 0U;
                        __enable_irq();

                        question6_running_flag = 1U;
                        xunji_update_flag = 1U;
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

            /*
             * 每50ms刷新一次OLED，避免频繁刷屏影响主循环。
             * 第四问通过B点后锁定显示通过时间，软刹车期间不再覆盖。
             */
            if(((question4_running_flag == 0U) || (q4_pass_b_flag == 0U)) &&
               (((question5_running_flag == 0U) &&
                 (question6_running_flag == 0U)) ||
                (q56_pass_line_flag == 0U)) &&
               ((run_elapsed_ms - run_last_oled_ms) >= 50U))
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
        //第三问
        if(question3_running_flag)
        {
            /* IMU每15ms置位一次标志，主循环在第三问运行块中发送。 */
            if(imu963ra_update_flag && imu963ra_acc_ready)
            {
                imu963ra_update_flag = 0U;
                F103_Trans_SendData(3U, 0U);
            }
        }
        //第四问
        if(question4_running_flag)
        {
            static int16_t left_speed_load = 0;
            static int16_t right_speed_load = 0;

            /* IMU每15ms置位一次标志，主循环在第四问运行块中发送。 */
            if(imu963ra_update_flag && imu963ra_acc_ready)
            {
                imu963ra_update_flag = 0U;
                F103_Trans_SendData(4U, 0U);
            }

            /*
             * 每10ms计算一次直线循迹目标速度，然后交给原有轮速PID闭环。
             * B点判断也使用本次计算刚读取到的XJ4、XJ5状态。
             */
            if(xunji_update_flag)
            {
                uint8_t q4_stopped;

                xunji_update_flag = 0U;
                q4_stopped = Xunji_Q4_Task(
                    run_elapsed_ms, &left_speed_load, &right_speed_load);

                /*
                 * 启动3秒后才允许判断B点。
                 * middle_state为0表示中间XJ4、XJ5都没有扫到黑线。
                 * 必须连续保持200ms才确认通过，短暂丢线只会重新计时。
                 */
                if(q4_pass_b_flag == 0U)
                {
                    if(run_elapsed_ms < Q4_B_DETECT_ENABLE_MS)
                    {
                        q4_middle_lost_flag = 0U;
                    }
                    else if(Xunji_Q4_Debug.middle_state == 0U)
                    {
                        if(q4_middle_lost_flag == 0U)
                        {
                            q4_middle_lost_flag = 1U;
                            q4_middle_lost_start_ms = run_elapsed_ms;
                        }
                        else if((run_elapsed_ms - q4_middle_lost_start_ms) >=
                                Q4_B_LOST_CONFIRM_MS)
                        {
                            /*
                             * 计时精度要求很低，直接把确认时刻作为通过B点时间。
                             * 从此OLED锁定该时间，不再显示后续直行和刹车时间。
                             */
                            q4_pass_b_flag = 1U;
                            q4_pass_b_time_ms = run_elapsed_ms;
                            OLED_ShowNum(2, 7, q4_pass_b_time_ms, 6);
                        }
                    }
                    else
                    {
                        q4_middle_lost_flag = 0U;
                    }
                }

                if(q4_pass_b_flag != 0U)
                {
                    /*
                     * B点以后已经没有循迹线，左右轮改为相同目标速度直行，
                     * 不再沿用丢线前最后一次方向修正，避免长距离行驶时跑偏。
                     */
                    left_speed_load = Xunji_Q4_Debug.base_speed;
                    right_speed_load = Xunji_Q4_Debug.base_speed;

                    /* 通过B点后等速行驶3秒，再只请求一次软停车。 */
                    if((q4_brake_started_flag == 0U) &&
                       ((run_elapsed_ms - q4_pass_b_time_ms) >=
                        Q4_AFTER_B_RUN_MS))
                    {
                        q4_brake_started_flag = 1U;
                        Xunji_Q4_RequestStop();
                    }
                }

                if(q4_stopped)
                {
                    // char q4_stop_str[64];

                    question4_running_flag = 0U;
                    question_running_flag = 0U;
                    AT8236_PID_Stop();
                    /* 电机停止后仍显示通过B点的时间，而不是停止时间。 */
                    OLED_ShowNum(2, 7, q4_pass_b_time_ms, 6);

                    // sprintf(q4_stop_str, "Q4_STOP,%lu\r\n",
                    //     (unsigned long)run_elapsed_ms);
                    // Uart_DebugSendString(q4_stop_str);
                }
                else
                {
                    AT8236_PID_SetTargets(left_speed_load, right_speed_load);
                }
            }

            /*
             * UART_BT当前为9600波特率，调试信息每1000ms发送一次，
             * 避免阻塞主循环过久。第二个调试阶段确认后可整体删除此代码块。
             */
            // if((run_elapsed_ms - q4_last_debug_ms) >= 1000U)
            // {
            //     int32_t speed_l;
            //     int32_t speed_r;
            //     char q4_debug_str[120];
            //
            //     q4_last_debug_ms = run_elapsed_ms;
            //
            //     __disable_irq();
            //     speed_l = l_speed_now;
            //     speed_r = r_speed_now;
            //     __enable_irq();
            //
            //     sprintf(q4_debug_str,
            //         "Q4,%lu,%u,%u,%d,%d,%d,%d,%ld,%ld\r\n",
            //         (unsigned long)run_elapsed_ms,
            //         (unsigned int)Xunji_Q4_Debug.sensor_bits,
            //         (unsigned int)Xunji_Q4_Debug.middle_state,
            //         (int)Xunji_Q4_Debug.base_speed,
            //         (int)Xunji_Q4_Debug.correction,
            //         (int)Xunji_Q4_Debug.left_target,
            //         (int)Xunji_Q4_Debug.right_target,
            //         (long)speed_l,
            //         (long)speed_r);
            //     Uart_DebugSendString(q4_debug_str);
            // }

        }
        //第五问
        if(question5_running_flag)
        {
            static int16_t left_speed_load = 0;
            static int16_t right_speed_load = 0;

            /*
             * 10ms中断检测到A点横线时已经立即停止电机并锁存了检测时刻。
             * 这里用锁存时刻计算整圈时间，OLED显示的不是主循环处理时刻。
             */
            if(q56_pass_line_flag)
            {
                q56_pass_a_time_ms =
                    q56_pass_line_tick_ms - run_start_ms;
                run_elapsed_ms = q56_pass_a_time_ms;

                question5_running_flag = 0U;
                question_running_flag = 0U;
                AT8236_PID_Stop();
                OLED_ShowNum(2, 7, q56_pass_a_time_ms, 6);
            }
            else
            {
                /* 第五问要求小球保持在中心位置，所以位置字节发送0。 */
                if(imu963ra_update_flag && imu963ra_acc_ready)
                {
                    imu963ra_update_flag = 0U;
                    F103_Trans_SendData(5U, 0U);
                }

                /*
                 * 外层循迹PID每10ms计算目标速度，内层轮速PID在定时器中断
                 * 中使用编码器实测速度闭环控制电机。
                 */
                if(xunji_update_flag)
                {
                    xunji_update_flag = 0U;
                    Xunji_Q5_Task(
                        run_elapsed_ms,
                        &left_speed_load,
                        &right_speed_load);

                    /*
                     * 计算期间若中断刚好检测到A线，就不能重新装载目标速度，
                     * 否则会覆盖中断中已经执行的立即停车。
                     */
                    if(q56_pass_line_flag == 0U)
                    {
                        AT8236_PID_SetTargets(
                            left_speed_load, right_speed_load);
                    }
                }
            }
        }
        //第六问
        if(question6_running_flag)
        {
            static int16_t left_speed_load = 0;
            static int16_t right_speed_load = 0;

            if(q56_pass_line_flag)
            {
                q56_pass_a_time_ms =
                    q56_pass_line_tick_ms - run_start_ms;
                run_elapsed_ms = q56_pass_a_time_ms;

                question6_running_flag = 0U;
                question_running_flag = 0U;
                AT8236_PID_Stop();
                OLED_ShowNum(2, 7, q56_pass_a_time_ms, 6);
            }
            else
            {
                /* 第六问发送0～250mm指定位置和Y轴加速度。 */
                if(imu963ra_update_flag && imu963ra_acc_ready)
                {
                    imu963ra_update_flag = 0U;
                    F103_Trans_SendData(
                        6U, ball_target_position_mm);
                }

                if(xunji_update_flag)
                {
                    xunji_update_flag = 0U;
                    Xunji_Q6_Task(
                        run_elapsed_ms,
                        &left_speed_load,
                        &right_speed_load);

                    if(q56_pass_line_flag == 0U)
                    {
                        AT8236_PID_SetTargets(
                            left_speed_load, right_speed_load);
                    }
                }
            }
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
