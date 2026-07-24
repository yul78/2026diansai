#include "imu963ra_attitude_demo.h"

#include "zf_common_headfile.h"

#define IMU963RA_DEMO_DISPLAY_DIVIDER      (10U)
#define IMU963RA_DEMO_PRINT_DIVIDER        (5U)

imu963ra_attitude_angle_struct imu963ra_attitude_demo_angle = {0.0f, 0.0f, 0.0f};

static volatile uint8 imu963ra_attitude_demo_display_flag = 0;
static volatile uint8 imu963ra_attitude_demo_runtime_ready = 0;
static uint8 imu963ra_attitude_demo_layout_ready = 0;
static uint8 imu963ra_attitude_demo_print_divider = 0;
static uint8 imu963ra_attitude_demo_display_divider = 0;

static void imu963ra_attitude_demo_show_status(const char *title, const char *line1, const char *line2);
static void imu963ra_attitude_demo_show_error(const char *message);
static void imu963ra_attitude_demo_draw_layout(void);
static void imu963ra_attitude_demo_show_angle(const imu963ra_attitude_angle_struct *angle);
static void imu963ra_attitude_demo_print_angle(const imu963ra_attitude_angle_struct *angle);

uint8 imu963ra_attitude_demo_init(void)
{
    uint8 imu_init_state = 0;

    imu963ra_attitude_demo_layout_ready = 0;
    imu963ra_attitude_demo_print_divider = 0;
    imu963ra_attitude_demo_display_divider = 0;
    imu963ra_attitude_demo_display_flag = 0;
    imu963ra_attitude_demo_runtime_ready = 0;
    imu963ra_attitude_demo_angle.roll = 0.0f;
    imu963ra_attitude_demo_angle.pitch = 0.0f;
    imu963ra_attitude_demo_angle.yaw = 0.0f;

    tft180_init();
    tft180_set_dir(TFT180_CROSSWISE);
    tft180_set_font(TFT180_6X8_FONT);
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_clear();

    imu963ra_attitude_demo_show_status("IMU963RA ATTITUDE", "imu963ra init...", "please wait");
    printf("\r\n[imu963ra demo] imu963ra init start.");

    imu_init_state = imu963ra_init();
    if (imu_init_state)
    {
        imu963ra_attitude_demo_show_error("IMU963RA INIT ERR");
        printf("\r\n[imu963ra demo] imu963ra init error.");
        return 1;
    }

    printf("\r\n[imu963ra demo] imu963ra init success.");
    printf("\r\n[imu963ra demo] gyro calibrating, keep device still...");
    imu963ra_attitude_demo_show_status("IMU963RA ATTITUDE", "gyro calibrating", "keep device still");

    imu963ra_attitude_init(IMU963RA_ATTITUDE_DEFAULT_SAMPLE_RATE_HZ);

    printf("\r\n[imu963ra demo] gyro calibrate done.");
    printf("\r\n[imu963ra demo] ahrs stabilizing...");
    imu963ra_attitude_demo_show_status("IMU963RA ATTITUDE", "ahrs stabilizing", "wait for display");

    return 0;
}

void imu963ra_attitude_demo_task(void)
{
    if (imu963ra_attitude_demo_runtime_ready && !imu963ra_attitude_demo_layout_ready)
    {
        imu963ra_attitude_demo_layout_ready = 1;
        imu963ra_attitude_demo_draw_layout();
        printf("\r\n[imu963ra demo] attitude display ready.\r\n");
        return;
    }

    if (imu963ra_attitude_demo_runtime_ready && imu963ra_attitude_demo_display_flag)
    {
        imu963ra_attitude_demo_display_flag = 0;
        imu963ra_attitude_get_euler(&imu963ra_attitude_demo_angle);
        imu963ra_attitude_demo_show_angle(&imu963ra_attitude_demo_angle);

        imu963ra_attitude_demo_print_divider++;
        if (imu963ra_attitude_demo_print_divider >= IMU963RA_DEMO_PRINT_DIVIDER)
        {
            imu963ra_attitude_demo_print_divider = 0;
            imu963ra_attitude_demo_print_angle(&imu963ra_attitude_demo_angle);
        }
    }
}

void imu963ra_attitude_demo_pit_handler(uint32 event, void *ptr)
{
    (void)event;
    (void)ptr;

    imu963ra_attitude_update();

    if (!imu963ra_attitude_demo_runtime_ready)
    {
        if (imu963ra_attitude_is_ready())
        {
            imu963ra_attitude_demo_runtime_ready = 1;
            imu963ra_attitude_demo_display_divider = 0;
        }
        return;
    }

    imu963ra_attitude_demo_display_divider++;
    if (imu963ra_attitude_demo_display_divider >= IMU963RA_DEMO_DISPLAY_DIVIDER)
    {
        imu963ra_attitude_demo_display_divider = 0;
        imu963ra_attitude_demo_display_flag = 1;
    }
}

uint8 imu963ra_attitude_demo_is_ready(void)
{
    return imu963ra_attitude_demo_runtime_ready;
}

static void imu963ra_attitude_demo_show_status(const char *title, const char *line1, const char *line2)
{
    tft180_clear();
    tft180_show_string(0,  0, title);
    tft180_show_string(8, 36, line1);
    tft180_show_string(8, 52, line2);
}

static void imu963ra_attitude_demo_show_error(const char *message)
{
    tft180_clear();
    tft180_show_string(8, 8, "ERROR:");
    tft180_show_string(8, 36, message);
}

static void imu963ra_attitude_demo_draw_layout(void)
{
    tft180_clear();
    tft180_show_string(0,  0, "IMU963RA ATTITUDE");
    tft180_show_string(4, 28, "ROLL :");
    tft180_show_string(4, 48, "PITCH:");
    tft180_show_string(4, 68, "YAW  :");
}

static void imu963ra_attitude_demo_show_angle(const imu963ra_attitude_angle_struct *angle)
{
    if (0 == angle)
    {
        return;
    }

    tft180_show_float(56, 28, angle->roll,  8, 2);
    tft180_show_float(56, 48, angle->pitch, 8, 2);
    tft180_show_float(56, 68, angle->yaw,   8, 2);
}

static void imu963ra_attitude_demo_print_angle(const imu963ra_attitude_angle_struct *angle)
{
    if (0 == angle)
    {
        return;
    }

    printf("\r\nattitude angle: roll=%7.2f, pitch=%7.2f, yaw=%7.2f\r\n", angle->roll, angle->pitch, angle->yaw);
}