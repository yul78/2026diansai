#include "zf_common_headfile.h"
#include "imu963ra_attitude.h"

static void attitude_demo_show_angles(void);
static void attitude_demo_show_error(const char *message);

int main(void)
{
    uint8 imu_init_state = 0;

    clock_init(SYSTEM_CLOCK_80M);
    debug_init();

    oled_init();
    oled_set_font(OLED_8X16_FONT);

    imu_init_state = imu963ra_init();
    if (imu_init_state)
    {
        attitude_demo_show_error("IMU963RA INIT ERR");
        while (true)
        {
            system_delay_ms(100);
        }
    }

    imu963ra_attitude_init(IMU963RA_ATTITUDE_DEFAULT_SAMPLE_RATE_HZ);

    while (true)
    {
        imu963ra_attitude_update();
        attitude_demo_show_angles();
        system_delay_ms(10);
    }
}

static void attitude_demo_show_angles(void)
{
    oled_clear();
    oled_show_string(0, 0, "ROLL:");
    oled_show_string(0, 16, "PITCH:");
    oled_show_string(0, 32, "YAW:");

    oled_show_float(48, 0, imu963ra_attitude_roll, 7, 2);
    oled_show_float(48, 16, imu963ra_attitude_pitch, 7, 2);
    oled_show_float(48, 32, imu963ra_attitude_yaw, 7, 2);
}

static void attitude_demo_show_error(const char *message)
{
    oled_clear();
    oled_show_string(0, 0, "ERROR:");
    oled_show_string(0, 16, message);
}
