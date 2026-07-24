#include "imu963ra_attitude.h"

#include "FusionAhrs.h"
#include "FusionMath.h"
#include "FusionOffset.h"
#include "zf_driver_delay.h"
#include "zf_device_imu963ra.h"

float imu963ra_attitude_roll = 0.0f;
float imu963ra_attitude_pitch = 0.0f;
float imu963ra_attitude_yaw = 0.0f;

static FusionAhrs imu963ra_fusion_ahrs;
static FusionOffset imu963ra_fusion_offset;
static FusionVector imu963ra_attitude_gyro_bias = FUSION_VECTOR_ZERO;
static float imu963ra_attitude_delta_time = 1.0f / IMU963RA_ATTITUDE_DEFAULT_SAMPLE_RATE_HZ;
static uint8 imu963ra_attitude_ready = 0;

static FusionVector imu963ra_attitude_build_acc_from_current(void);
static FusionVector imu963ra_attitude_build_gyro_from_current(void);
static void imu963ra_attitude_refresh_inertial(FusionVector *accelerometer, FusionVector *gyroscope);
static float imu963ra_attitude_get_gyro_range(void);
static void imu963ra_attitude_calibrate_gyro(void);

void imu963ra_attitude_init(float sample_rate_hz) {
    FusionAhrsSettings settings;

    if (sample_rate_hz <= 0.0f) {
        sample_rate_hz = IMU963RA_ATTITUDE_DEFAULT_SAMPLE_RATE_HZ;
    }

    imu963ra_attitude_delta_time = 1.0f / sample_rate_hz;

    FusionAhrsInitialise(&imu963ra_fusion_ahrs);
    settings = imu963ra_fusion_ahrs.settings;
    settings.convention = FusionConventionNwu;
    settings.gyroscopeRange = imu963ra_attitude_get_gyro_range();
    settings.recoveryTriggerPeriod = 0U;
    FusionAhrsSetSettings(&imu963ra_fusion_ahrs, &settings);

    FusionOffsetInitialise(&imu963ra_fusion_offset, (unsigned int)(sample_rate_hz + 0.5f));
    imu963ra_attitude_gyro_bias = FUSION_VECTOR_ZERO;

    system_delay_ms(IMU963RA_ATTITUDE_GYRO_SETTLE_DELAY_MS);
    imu963ra_attitude_calibrate_gyro();

    imu963ra_attitude_roll = 0.0f;
    imu963ra_attitude_pitch = 0.0f;
    imu963ra_attitude_yaw = 0.0f;
    imu963ra_attitude_ready = 1;
}

void imu963ra_attitude_update(void) {
    FusionQuaternion quaternion;
    FusionEuler euler;
    FusionVector accelerometer;
    FusionVector gyroscope;

    if (!imu963ra_attitude_ready) {
        return;
    }

    imu963ra_attitude_refresh_inertial(&accelerometer, &gyroscope);
    gyroscope = FusionVectorSubtract(gyroscope, imu963ra_attitude_gyro_bias);
    gyroscope = FusionOffsetUpdate(&imu963ra_fusion_offset, gyroscope);

    FusionAhrsUpdateNoMagnetometer(&imu963ra_fusion_ahrs, gyroscope, accelerometer, imu963ra_attitude_delta_time);

    quaternion = FusionAhrsGetQuaternion(&imu963ra_fusion_ahrs);
    euler = FusionQuaternionToEuler(quaternion);

    imu963ra_attitude_roll = euler.angle.roll;
    imu963ra_attitude_pitch = euler.angle.pitch;
    imu963ra_attitude_yaw = euler.angle.yaw;
}

uint8 imu963ra_attitude_is_ready(void) {
    return (uint8)(imu963ra_attitude_ready && (false == imu963ra_fusion_ahrs.initialising));
}

void imu963ra_attitude_get_euler(imu963ra_attitude_angle_struct *angle) {
    if (0 == angle) {
        return;
    }

    __disable_irq();
    angle->roll = imu963ra_attitude_roll;
    angle->pitch = imu963ra_attitude_pitch;
    angle->yaw = imu963ra_attitude_yaw;
    __enable_irq();
}

static FusionVector imu963ra_attitude_build_acc_from_current(void) {
    FusionVector result;

    result.axis.x = IMU963RA_ATTITUDE_ACC_X(imu963ra_acc_transition(imu963ra_acc_x));
    result.axis.y = IMU963RA_ATTITUDE_ACC_Y(imu963ra_acc_transition(imu963ra_acc_y));
    result.axis.z = IMU963RA_ATTITUDE_ACC_Z(imu963ra_acc_transition(imu963ra_acc_z));
    return result;
}

static FusionVector imu963ra_attitude_build_gyro_from_current(void) {
    FusionVector result;

    result.axis.x = IMU963RA_ATTITUDE_GYRO_X(imu963ra_gyro_transition(imu963ra_gyro_x));
    result.axis.y = IMU963RA_ATTITUDE_GYRO_Y(imu963ra_gyro_transition(imu963ra_gyro_y));
    result.axis.z = IMU963RA_ATTITUDE_GYRO_Z(imu963ra_gyro_transition(imu963ra_gyro_z));
    return result;
}

static void imu963ra_attitude_refresh_inertial(FusionVector *accelerometer, FusionVector *gyroscope) {
    imu963ra_get_acc_gyro();

    if (0 != accelerometer) {
        *accelerometer = imu963ra_attitude_build_acc_from_current();
    }

    if (0 != gyroscope) {
        *gyroscope = imu963ra_attitude_build_gyro_from_current();
    }
}

static float imu963ra_attitude_get_gyro_range(void) {
    const float factor = imu963ra_transition_factor[1];

    if (factor >= 200.0f) {
        return 125.0f;
    }
    if (factor >= 100.0f) {
        return 250.0f;
    }
    if (factor >= 50.0f) {
        return 500.0f;
    }
    if (factor >= 25.0f) {
        return 1000.0f;
    }
    if (factor >= 10.0f) {
        return 2000.0f;
    }
    return 4000.0f;
}

static void imu963ra_attitude_calibrate_gyro(void) {
    FusionVector sum = FUSION_VECTOR_ZERO;
    FusionVector minimum = FUSION_VECTOR_ZERO;
    FusionVector maximum = FUSION_VECTOR_ZERO;
    FusionVector sample = FUSION_VECTOR_ZERO;
    uint32 count = 0;
    float valid_count = (float)IMU963RA_ATTITUDE_GYRO_CALIBRATE_COUNT;

    for (count = 0; count < IMU963RA_ATTITUDE_GYRO_CALIBRATE_WARMUP_COUNT; count++) {
        imu963ra_attitude_refresh_inertial(0, &sample);
        system_delay_ms(IMU963RA_ATTITUDE_GYRO_CALIBRATE_DELAY_MS);
    }

    for (count = 0; count < IMU963RA_ATTITUDE_GYRO_CALIBRATE_COUNT; count++) {
        imu963ra_attitude_refresh_inertial(0, &sample);
        sum = FusionVectorAdd(sum, sample);

        if (0U == count) {
            minimum = sample;
            maximum = sample;
        } else {
            if (sample.axis.x < minimum.axis.x) minimum.axis.x = sample.axis.x;
            if (sample.axis.y < minimum.axis.y) minimum.axis.y = sample.axis.y;
            if (sample.axis.z < minimum.axis.z) minimum.axis.z = sample.axis.z;

            if (sample.axis.x > maximum.axis.x) maximum.axis.x = sample.axis.x;
            if (sample.axis.y > maximum.axis.y) maximum.axis.y = sample.axis.y;
            if (sample.axis.z > maximum.axis.z) maximum.axis.z = sample.axis.z;
        }

        system_delay_ms(IMU963RA_ATTITUDE_GYRO_CALIBRATE_DELAY_MS);
    }

    if (0U != IMU963RA_ATTITUDE_GYRO_CALIBRATE_COUNT) {
        if (IMU963RA_ATTITUDE_GYRO_CALIBRATE_COUNT > 2U) {
            sum = FusionVectorSubtract(sum, minimum);
            sum = FusionVectorSubtract(sum, maximum);
            valid_count -= 2.0f;
        }

        imu963ra_attitude_gyro_bias = FusionVectorMultiplyScalar(sum, 1.0f / valid_count);
    }
}