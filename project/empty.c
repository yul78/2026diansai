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

int32_t l_speed_now, r_speed_now;
static uint8_t imu963ra_ready;
static uint8_t imu963ra_init_state;

#define IMU963RA_OLED_ROWS             (4U)
#define IMU963RA_OLED_COLUMNS          (16U)
#define IMU963RA_ANGLE_COLUMN          (6U)
#define IMU963RA_ANGLE_TEXT_LENGTH     (6U)

static char imu963ra_oled_target[IMU963RA_OLED_ROWS][IMU963RA_OLED_COLUMNS] = {
    "ROLL :          ",
    "PITCH:          ",
    "YAW  :          ",
    "STABILIZING...  ",
};
static char imu963ra_oled_displayed[IMU963RA_OLED_ROWS][IMU963RA_OLED_COLUMNS] = {
    "ROLL :          ",
    "PITCH:          ",
    "YAW  :          ",
    "STABILIZING...  ",
};

void TimerTick_Init(void);
static void IMU963RA_DisplayAngles(void);
static void IMU963RA_DisplayTask(void);
static void IMU963RA_FormatAngle(float angle, char text[IMU963RA_ANGLE_TEXT_LENGTH]);
static void IMU963RA_SendDebug(void);

int main(void)
{
    SYSCFG_DL_init();
    TB6612_Init();
    Encoder_Init();
    OLED_Init();
    OLED_ShowString(1, 1, "IMU963 INIT...");
    imu963ra_init_state = imu963ra_init();
    imu963ra_ready = (imu963ra_init_state == IMU963RA_INIT_OK);
    OLED_Clear();
    if (imu963ra_ready) {
        OLED_ShowString(1, 1, "KEEP IMU STILL");
        OLED_ShowString(2, 1, "CALIBRATING...");
        imu963ra_attitude_init(100.0f);
        OLED_Clear();
        OLED_ShowString(1, 1, "ROLL :");
        OLED_ShowString(2, 1, "PITCH:");
        OLED_ShowString(3, 1, "YAW  :");
        OLED_ShowString(4, 1, "STABILIZING...  ");
    } else {
        OLED_ShowString(1, 1, "IMU963 ERROR");
        OLED_ShowString(2, 1, "CODE:");
        OLED_ShowNum(2, 6, imu963ra_init_state, 1);
        OLED_ShowString(3, 1, "ADDR:0x");
        OLED_ShowHexNum(3, 8, imu963ra_device_addr, 2);
        OLED_ShowString(4, 1, "WHO:0x");
        OLED_ShowHexNum(4, 7, imu963ra_who_am_i, 2);
    }
    IMU963RA_SendDebug();
    Bluetooth_Init();
    TimerTick_Init();
    
    while (1) {
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
        if (imu963ra_ready && imu963ra_display_flag) {
            imu963ra_display_flag = 0U;
            IMU963RA_DisplayAngles();
        }
        if (imu963ra_ready) {
            IMU963RA_DisplayTask();
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

static void IMU963RA_DisplayAngles(void)
{
    static const char ready_text[IMU963RA_OLED_COLUMNS] = "ATTITUDE READY  ";
    imu963ra_attitude_angle_struct angle;
    char angle_text[IMU963RA_ANGLE_TEXT_LENGTH];
    uint8_t row;
    uint8_t column;

    imu963ra_attitude_get_euler(&angle);

    IMU963RA_FormatAngle(angle.roll, angle_text);
    for (column = 0U; column < IMU963RA_ANGLE_TEXT_LENGTH; column++) {
        imu963ra_oled_target[0][IMU963RA_ANGLE_COLUMN + column] = angle_text[column];
    }
    IMU963RA_FormatAngle(angle.pitch, angle_text);
    for (column = 0U; column < IMU963RA_ANGLE_TEXT_LENGTH; column++) {
        imu963ra_oled_target[1][IMU963RA_ANGLE_COLUMN + column] = angle_text[column];
    }
    IMU963RA_FormatAngle(angle.yaw, angle_text);
    for (column = 0U; column < IMU963RA_ANGLE_TEXT_LENGTH; column++) {
        imu963ra_oled_target[2][IMU963RA_ANGLE_COLUMN + column] = angle_text[column];
    }

    if (imu963ra_attitude_is_ready()) {
        for (row = 0U; row < IMU963RA_OLED_COLUMNS; row++) {
            imu963ra_oled_target[3][row] = ready_text[row];
        }
    }
}

static void IMU963RA_DisplayTask(void)
{
    static uint8_t scan_index = 0U;
    uint8_t attempt;

    for (attempt = 0U; attempt < IMU963RA_OLED_ROWS * IMU963RA_OLED_COLUMNS; attempt++) {
        uint8_t row = scan_index / IMU963RA_OLED_COLUMNS;
        uint8_t column = scan_index % IMU963RA_OLED_COLUMNS;

        scan_index++;
        if (scan_index >= IMU963RA_OLED_ROWS * IMU963RA_OLED_COLUMNS) {
            scan_index = 0U;
        }
        if (imu963ra_oled_displayed[row][column] != imu963ra_oled_target[row][column]) {
            OLED_ShowChar(row + 1U, column + 1U, imu963ra_oled_target[row][column]);
            imu963ra_oled_displayed[row][column] = imu963ra_oled_target[row][column];
            return;
        }
    }
}

static void IMU963RA_FormatAngle(float angle, char text[IMU963RA_ANGLE_TEXT_LENGTH])
{
    uint32_t scaled;

    if (angle > 999.9f) {
        angle = 999.9f;
    } else if (angle < -999.9f) {
        angle = -999.9f;
    }

    text[0] = (angle < 0.0f) ? '-' : '+';
    if (angle < 0.0f) {
        angle = -angle;
    }
    scaled = (uint32_t)(angle * 10.0f + 0.5f);
    text[1] = (char)('0' + ((scaled / 1000U) % 10U));
    text[2] = (char)('0' + ((scaled / 100U) % 10U));
    text[3] = (char)('0' + ((scaled / 10U) % 10U));
    text[4] = '.';
    text[5] = (char)('0' + (scaled % 10U));
}

static void IMU963RA_SendDebug(void)
{
    Uart_DebugSendString("IMU state=");
    Uart_DebugSendHexByte(imu963ra_init_state);
    Uart_DebugSendString(" addr=");
    Uart_DebugSendHexByte(imu963ra_device_addr);
    Uart_DebugSendString(" who=");
    Uart_DebugSendHexByte(imu963ra_who_am_i);
    Uart_DebugSendString(" SCL(L/R)=");
    Uart_DebugSendHexByte(imu963ra_diag_scl_low);
    Uart_DebugSendString("/");
    Uart_DebugSendHexByte(imu963ra_diag_scl_release);
    Uart_DebugSendString(" SDA(L/R)=");
    Uart_DebugSendHexByte(imu963ra_diag_sda_low);
    Uart_DebugSendString("/");
    Uart_DebugSendHexByte(imu963ra_diag_sda_release);
    Uart_DebugSendString(" ACK(6B/6A)=");
    Uart_DebugSendHexByte(imu963ra_diag_ack_6b);
    Uart_DebugSendString("/");
    Uart_DebugSendHexByte(imu963ra_diag_ack_6a);
    Uart_DebugSendString(" WHO(6B/6A)=");
    Uart_DebugSendHexByte(imu963ra_diag_who_6b);
    Uart_DebugSendString("/");
    Uart_DebugSendHexByte(imu963ra_diag_who_6a);
    Uart_DebugSendString("\r\n");
}
