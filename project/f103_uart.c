#include "f103_uart.h"

#include "ti_msp_dl_config.h"
#include "imu963/zf_device_imu963ra.h"


/*
 * 通过DEBUG串口阻塞发送一个字节。
 *
 * 串口配置：
 * 波特率：115200
 * 数据位：8位
 * 校验位：无
 * 停止位：1位
 */
static void F103_Trans_SendByte(uint8_t data)
{
    DL_UART_Main_transmitDataBlocking(UART_DEBUG_INST, data);
}


/*
 * 将IMU963加速度原始数据转换成mg。
 *
 * imu963ra_acc_transition()返回单位为g的加速度，
 * 乘1000后得到单位为mg的加速度。
 */
static int16_t F103_Trans_AccRawToMg(int16 raw_value)
{
    float acceleration_mg;

    acceleration_mg =
        imu963ra_acc_transition(raw_value) * 1000.0f;

    /*
     * 限制数据范围，避免转换成int16_t时溢出。
     */
    if(acceleration_mg > 32767.0f)
    {
        acceleration_mg = 32767.0f;
    }
    else if(acceleration_mg < -32768.0f)
    {
        acceleration_mg = -32768.0f;
    }

    return (int16_t)acceleration_mg;
}


/*
 * 发送已经换算成mg的Y轴加速度。
 *
 * 数据帧共4字节：
 * 帧头、Y低字节、Y高字节、异或校验。
 */
void F103_Trans_SendAccelerationMg(int16_t acc_y_mg)
{
    uint8_t acc_y_low;
    uint8_t acc_y_high;
    uint8_t checksum;

    /*
     * 将int16_t按照小端格式拆分。
     * 串口先发送低字节，再发送高字节。
     */
    acc_y_low =
        (uint8_t)((uint16_t)acc_y_mg & 0x00FFU);

    acc_y_high =
        (uint8_t)(((uint16_t)acc_y_mg >> 8) & 0x00FFU);

    /*
     * 帧头和两个数据字节直接异或。
     */
    checksum =
        F103_TRANS_FRAME_HEADER ^ acc_y_low ^ acc_y_high;

    F103_Trans_SendByte(F103_TRANS_FRAME_HEADER);
    F103_Trans_SendByte(acc_y_low);
    F103_Trans_SendByte(acc_y_high);
    F103_Trans_SendByte(checksum);
}


/*
 * 读取IMU963的加速度寄存器，只换算并发送Y轴。
 *
 * 该函数不会读取陀螺仪，也不会进行姿态角解算。
 * 建议在主循环中每20ms调用一次，不要放在定时器中断中执行。
 */
void F103_Trans_SendImu963Acceleration(void)
{
    int16_t acc_y_mg;

    /*
     * 一次读取X、Y、Z三个加速度寄存器，
     * 但后续只使用Y轴数据。
     */
    imu963ra_get_acc();

    acc_y_mg =
        F103_Trans_AccRawToMg(imu963ra_acc_y);

    F103_Trans_SendAccelerationMg(acc_y_mg);
}