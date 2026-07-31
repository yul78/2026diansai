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
 * 唯一对外发送接口。
 * 读取IMU963的Y轴加速度，并发送固定6字节数据帧：
 * 帧头、题号、指定位置、Y低字节、Y高字节、异或校验。
 */
void F103_Trans_SendData(
    uint8_t question_number, uint8_t target_position_mm)
{
    int16_t acc_y_mg;
    uint8_t acc_y_low;
    uint8_t acc_y_high;
    uint8_t checksum;

    /* F103只需要处理第3、4、5、6题，其他题不发送。 */
    if((question_number < 3U) || (question_number > 6U))
    {
        return;
    }

    /* 只有第六问使用指定位置，其他题的数据帧中该字节固定为0。 */
    if(question_number != 6U)
    {
        target_position_mm = 0U;
    }

    /*
     * 一次读取X、Y、Z三个加速度寄存器，
     * 但后续只使用Y轴数据。
     */
    imu963ra_get_acc();

    acc_y_mg =
        F103_Trans_AccRawToMg(imu963ra_acc_y);

    /* int16_t按照小端格式拆分，先发送低字节，再发送高字节。 */
    acc_y_low = (uint8_t)((uint16_t)acc_y_mg & 0x00FFU);
    acc_y_high =
        (uint8_t)(((uint16_t)acc_y_mg >> 8) & 0x00FFU);

    checksum = F103_TRANS_FRAME_HEADER ^
               question_number ^
               target_position_mm ^
               acc_y_low ^
               acc_y_high;

    F103_Trans_SendByte(F103_TRANS_FRAME_HEADER);
    F103_Trans_SendByte(question_number);
    F103_Trans_SendByte(target_position_mm);
    F103_Trans_SendByte(acc_y_low);
    F103_Trans_SendByte(acc_y_high);
    F103_Trans_SendByte(checksum);
}
