/*********************************************************************************************************************
* MSPM0G3507 Opensource Library 姿态角演示例程
*
* 本例程基于 IMU963RA 姿态解算模块编写，主函数仅保留最核心的调用流程，
* 目的是让用户一眼看懂如何获取 roll、pitch、yaw 三个姿态角。
*
* *************************** 例程使用流程 ***************************
* 1. 上电后先完成系统时钟与调试串口初始化。
* 2. 调用 imu963ra_attitude_demo_init() 完成以下工作：
*      IMU963RA 初始化
*      姿态解算初始化
*      TFT180 初始化
*      陀螺仪零偏校准前提示显示
* 3. 使用 pit_us_init() 配置 PIT 定时中断，周期调用
*      imu963ra_attitude_demo_pit_handler()
*    该中断函数内部会持续更新姿态解算。
* 4. 主循环中持续调用
*      imu963ra_attitude_demo_task();
*    该函数负责屏幕刷新与串口打印。
* 5. 用户在自己的工程里，直接读取下面三个成员即可获取姿态角：
*      imu963ra_attitude_demo_angle.roll
*      imu963ra_attitude_demo_angle.pitch
*      imu963ra_attitude_demo_angle.yaw
*
* *************************** 例程现象说明 ***************************
* 1. 上电后屏幕会先显示 imu 初始化提示。
* 2. 随后会显示 gyro calibrating 提示，此时请保持模块静止。
* 3. 零偏校准完成后，会显示 ahrs stabilizing 提示。
* 4. 稍等片刻后，TFT180 与串口开始输出三个姿态角。
* 5. 转动模块时，可以观察到 roll、pitch、yaw 实时变化。
*
* *************************** 重要参数人工修改说明 ***************************
* 需要人工调节的参数不在本文件中，而在 imu963ra_attitude.h 中：
*
* 1. IMU963RA_ATTITUDE_GYRO_SETTLE_DELAY_MS
*    上电后开始零偏校准前的静置等待时间。
*    如果模块上电瞬间容易晃动，可适当调大。
*    一般情况下保持默认即可。
*
* 2. IMU963RA_ATTITUDE_GYRO_CALIBRATE_COUNT
*    零偏校准采样次数。
*    采样次数越多，零偏平均越稳定，但启动等待时间越长。
*    一般情况下保持默认即可。
*
* 3. IMU963RA_ATTITUDE_GYRO_Z_SCALE_TRIM
*    yaw 方向比例微调系数。
*    这是最常用的人工调节参数。
*    例如实际旋转 180°，如果yaw显示偏小，可略微调小；如果显示偏大，可略微调大。
*********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "imu963ra_attitude_demo.h"

int main (void)
{
    // 系统时钟初始化
    clock_init(SYSTEM_CLOCK_80M);

    // 调试串口初始化，用于串口打印姿态角
    debug_init();

    // 完成 IMU 初始化、姿态解算初始化以及屏幕初始化
    // 如果返回 1，说明初始化失败
    if (imu963ra_attitude_demo_init())
    {
        while (1)
        {
        }
    }

    // 使用 PIT 定时中断驱动姿态更新
    // 中断函数里会周期调用 imu963ra_attitude_demo_pit_handler()
    pit_us_init(PIT_TIM_G12, IMU963RA_ATTITUDE_UPDATE_PERIOD_US, imu963ra_attitude_demo_pit_handler, NULL);

    while (1)
    {
        // demo 任务：负责刷新 TFT180 和串口输出
        imu963ra_attitude_demo_task();

        // 用户自己的工程里，直接这样使用三个姿态角即可
        // 这三个成员的单位都是“度”
        // imu963ra_attitude_demo_angle.roll
        // imu963ra_attitude_demo_angle.pitch
        // imu963ra_attitude_demo_angle.yaw
    }
}

