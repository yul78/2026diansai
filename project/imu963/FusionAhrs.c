/**
 * @file FusionAhrs.c
 * @author Seb Madgwick
 * @brief AHRS algorithm to combine gyroscope, accelerometer, and magnetometer.
 */

#include "FusionAhrs.h"

#include <float.h>
#include <math.h>

#define INITIAL_GAIN                 (10.0f)
#define INITIALISATION_PERIOD        (3.0f)

static inline FusionVector FusionAhrsHalfGravity(const FusionAhrs *const ahrs);
static inline FusionVector FusionAhrsHalfMagnetic(const FusionAhrs *const ahrs);
static inline FusionVector FusionAhrsFeedback(FusionVector sensor, FusionVector reference);
static inline int FusionAhrsClamp(int value, int min, int max);

void FusionAhrsInitialise(FusionAhrs *const ahrs) {
    const FusionAhrsSettings settings = {
        .convention = FusionConventionNwu,
        .gain = 0.5f,
        .gyroscopeRange = 0.0f,
        .accelerationRejection = 90.0f,
        .magneticRejection = 90.0f,
        .recoveryTriggerPeriod = 0U,
    };
    FusionAhrsSetSettings(ahrs, &settings);
    FusionAhrsReset(ahrs);
}

void FusionAhrsReset(FusionAhrs *const ahrs) {
    ahrs->quaternion = FUSION_IDENTITY_QUATERNION;
    ahrs->accelerometer = FUSION_VECTOR_ZERO;
    ahrs->initialising = true;
    ahrs->rampedGain = INITIAL_GAIN;
    ahrs->angularRateRecovery = false;
    ahrs->halfAccelerometerFeedback = FUSION_VECTOR_ZERO;
    ahrs->halfMagnetometerFeedback = FUSION_VECTOR_ZERO;
    ahrs->accelerometerIgnored = false;
    ahrs->accelerationRecoveryTrigger = 0;
    ahrs->accelerationRecoveryTimeout = ahrs->settings.recoveryTriggerPeriod;
    ahrs->magnetometerIgnored = false;
    ahrs->magneticRecoveryTrigger = 0;
    ahrs->magneticRecoveryTimeout = ahrs->settings.recoveryTriggerPeriod;
}

void FusionAhrsSetSettings(FusionAhrs *const ahrs, const FusionAhrsSettings *const settings) {
    ahrs->settings.convention = settings->convention;
    ahrs->settings.gain = settings->gain;
    ahrs->settings.gyroscopeRange = (settings->gyroscopeRange == 0.0f) ? FLT_MAX : (0.98f * settings->gyroscopeRange);
    ahrs->settings.accelerationRejection = (settings->accelerationRejection == 0.0f)
                                           ? FLT_MAX
                                           : powf(0.5f * sinf(FusionDegreesToRadians(settings->accelerationRejection)), 2.0f);
    ahrs->settings.magneticRejection = (settings->magneticRejection == 0.0f)
                                       ? FLT_MAX
                                       : powf(0.5f * sinf(FusionDegreesToRadians(settings->magneticRejection)), 2.0f);
    ahrs->settings.recoveryTriggerPeriod = settings->recoveryTriggerPeriod;
    ahrs->accelerationRecoveryTimeout = ahrs->settings.recoveryTriggerPeriod;
    ahrs->magneticRecoveryTimeout = ahrs->settings.recoveryTriggerPeriod;

    if ((settings->gain == 0.0f) || (settings->recoveryTriggerPeriod == 0U)) {
        ahrs->settings.accelerationRejection = FLT_MAX;
        ahrs->settings.magneticRejection = FLT_MAX;
    }

    if (!ahrs->initialising) {
        ahrs->rampedGain = ahrs->settings.gain;
    }
    ahrs->rampedGainStep = (INITIAL_GAIN - ahrs->settings.gain) / INITIALISATION_PERIOD;
}

void FusionAhrsUpdate(FusionAhrs *const ahrs, FusionVector gyroscope, FusionVector accelerometer, FusionVector magnetometer, float deltaTime) {
    ahrs->accelerometer = accelerometer;

    if ((fabsf(gyroscope.axis.x) > ahrs->settings.gyroscopeRange) ||
        (fabsf(gyroscope.axis.y) > ahrs->settings.gyroscopeRange) ||
        (fabsf(gyroscope.axis.z) > ahrs->settings.gyroscopeRange)) {
        const FusionQuaternion quaternion = ahrs->quaternion;
        FusionAhrsReset(ahrs);
        ahrs->quaternion = quaternion;
        ahrs->angularRateRecovery = true;
    }

    if (ahrs->initialising) {
        ahrs->rampedGain -= ahrs->rampedGainStep * deltaTime;
        if ((ahrs->rampedGain < ahrs->settings.gain) || (ahrs->settings.gain == 0.0f)) {
            ahrs->rampedGain = ahrs->settings.gain;
            ahrs->initialising = false;
            ahrs->angularRateRecovery = false;
        }
    }

    const FusionVector halfGravity = FusionAhrsHalfGravity(ahrs);

    FusionVector halfAccelerometerFeedback = FUSION_VECTOR_ZERO;
    ahrs->accelerometerIgnored = true;
    if (!FusionVectorIsZero(accelerometer)) {
        ahrs->halfAccelerometerFeedback = FusionAhrsFeedback(FusionVectorNormalise(accelerometer), halfGravity);

        if (ahrs->initialising ||
            (FusionVectorMagnitudeSquared(ahrs->halfAccelerometerFeedback) <= ahrs->settings.accelerationRejection)) {
            ahrs->accelerometerIgnored = false;
            ahrs->accelerationRecoveryTrigger -= 9;
        } else {
            ahrs->accelerationRecoveryTrigger += 1;
        }

        if (ahrs->accelerationRecoveryTrigger > ahrs->accelerationRecoveryTimeout) {
            ahrs->accelerationRecoveryTimeout = 0;
            ahrs->accelerometerIgnored = false;
        } else {
            ahrs->accelerationRecoveryTimeout = ahrs->settings.recoveryTriggerPeriod;
        }

        ahrs->accelerationRecoveryTrigger = FusionAhrsClamp(
            ahrs->accelerationRecoveryTrigger,
            0,
            (int)ahrs->settings.recoveryTriggerPeriod);

        if (!ahrs->accelerometerIgnored) {
            halfAccelerometerFeedback = ahrs->halfAccelerometerFeedback;
        }
    }

    FusionVector halfMagnetometerFeedback = FUSION_VECTOR_ZERO;
    ahrs->magnetometerIgnored = true;
    if (!FusionVectorIsZero(magnetometer)) {
        const FusionVector halfMagnetic = FusionAhrsHalfMagnetic(ahrs);
        ahrs->halfMagnetometerFeedback = FusionAhrsFeedback(
            FusionVectorNormalise(FusionVectorCrossProduct(halfGravity, magnetometer)),
            halfMagnetic);

        if (ahrs->initialising ||
            (FusionVectorMagnitudeSquared(ahrs->halfMagnetometerFeedback) <= ahrs->settings.magneticRejection)) {
            ahrs->magnetometerIgnored = false;
            ahrs->magneticRecoveryTrigger -= 9;
        } else {
            ahrs->magneticRecoveryTrigger += 1;
        }

        if (ahrs->magneticRecoveryTrigger > ahrs->magneticRecoveryTimeout) {
            ahrs->magneticRecoveryTimeout = 0;
            ahrs->magnetometerIgnored = false;
        } else {
            ahrs->magneticRecoveryTimeout = ahrs->settings.recoveryTriggerPeriod;
        }

        ahrs->magneticRecoveryTrigger = FusionAhrsClamp(
            ahrs->magneticRecoveryTrigger,
            0,
            (int)ahrs->settings.recoveryTriggerPeriod);

        if (!ahrs->magnetometerIgnored) {
            halfMagnetometerFeedback = ahrs->halfMagnetometerFeedback;
        }
    }

    const FusionVector halfGyroscope = FusionVectorMultiplyScalar(gyroscope, FusionDegreesToRadians(0.5f));
    const FusionVector adjustedHalfGyroscope = FusionVectorAdd(
        halfGyroscope,
        FusionVectorMultiplyScalar(
            FusionVectorAdd(halfAccelerometerFeedback, halfMagnetometerFeedback),
            ahrs->rampedGain));

    ahrs->quaternion = FusionQuaternionAdd(
        ahrs->quaternion,
        FusionQuaternionMultiplyVector(
            ahrs->quaternion,
            FusionVectorMultiplyScalar(adjustedHalfGyroscope, deltaTime)));

    ahrs->quaternion = FusionQuaternionNormalise(ahrs->quaternion);
}

void FusionAhrsUpdateNoMagnetometer(FusionAhrs *const ahrs, FusionVector gyroscope, FusionVector accelerometer, float deltaTime) {
    FusionAhrsUpdate(ahrs, gyroscope, accelerometer, FUSION_VECTOR_ZERO, deltaTime);

    if (ahrs->initialising) {
        FusionAhrsSetHeading(ahrs, 0.0f);
    }
}

FusionQuaternion FusionAhrsGetQuaternion(const FusionAhrs *const ahrs) {
    return ahrs->quaternion;
}

void FusionAhrsSetHeading(FusionAhrs *const ahrs, float heading) {
    const float yaw = atan2f(
        ahrs->quaternion.element.w * ahrs->quaternion.element.z +
        ahrs->quaternion.element.x * ahrs->quaternion.element.y,
        0.5f - ahrs->quaternion.element.y * ahrs->quaternion.element.y -
        ahrs->quaternion.element.z * ahrs->quaternion.element.z);
    const float halfYawMinusHeading = 0.5f * (yaw - FusionDegreesToRadians(heading));
    const FusionQuaternion rotation = {.element = {
        .w = cosf(halfYawMinusHeading),
        .x = 0.0f,
        .y = 0.0f,
        .z = -sinf(halfYawMinusHeading),
    }};
    ahrs->quaternion = FusionQuaternionMultiply(rotation, ahrs->quaternion);
}

static inline FusionVector FusionAhrsHalfGravity(const FusionAhrs *const ahrs) {
    switch (ahrs->settings.convention) {
        case FusionConventionNwu:
        case FusionConventionEnu: {
            const FusionVector halfGravity = {.axis = {
                .x = ahrs->quaternion.element.x * ahrs->quaternion.element.z - ahrs->quaternion.element.w * ahrs->quaternion.element.y,
                .y = ahrs->quaternion.element.y * ahrs->quaternion.element.z + ahrs->quaternion.element.w * ahrs->quaternion.element.x,
                .z = ahrs->quaternion.element.w * ahrs->quaternion.element.w - 0.5f + ahrs->quaternion.element.z * ahrs->quaternion.element.z,
            }};
            return halfGravity;
        }
        case FusionConventionNed: {
            const FusionVector halfGravity = {.axis = {
                .x = ahrs->quaternion.element.w * ahrs->quaternion.element.y - ahrs->quaternion.element.x * ahrs->quaternion.element.z,
                .y = -(ahrs->quaternion.element.y * ahrs->quaternion.element.z + ahrs->quaternion.element.w * ahrs->quaternion.element.x),
                .z = 0.5f - ahrs->quaternion.element.w * ahrs->quaternion.element.w - ahrs->quaternion.element.z * ahrs->quaternion.element.z,
            }};
            return halfGravity;
        }
    }
    return FUSION_VECTOR_ZERO;
}

static inline FusionVector FusionAhrsHalfMagnetic(const FusionAhrs *const ahrs) {
    switch (ahrs->settings.convention) {
        case FusionConventionNwu: {
            const FusionVector halfMagnetic = {.axis = {
                .x = ahrs->quaternion.element.x * ahrs->quaternion.element.y + ahrs->quaternion.element.w * ahrs->quaternion.element.z,
                .y = ahrs->quaternion.element.w * ahrs->quaternion.element.w - 0.5f + ahrs->quaternion.element.y * ahrs->quaternion.element.y,
                .z = ahrs->quaternion.element.y * ahrs->quaternion.element.z - ahrs->quaternion.element.w * ahrs->quaternion.element.x,
            }};
            return halfMagnetic;
        }
        case FusionConventionEnu: {
            const FusionVector halfMagnetic = {.axis = {
                .x = 0.5f - ahrs->quaternion.element.w * ahrs->quaternion.element.w - ahrs->quaternion.element.x * ahrs->quaternion.element.x,
                .y = ahrs->quaternion.element.w * ahrs->quaternion.element.z - ahrs->quaternion.element.x * ahrs->quaternion.element.y,
                .z = -(ahrs->quaternion.element.x * ahrs->quaternion.element.z + ahrs->quaternion.element.w * ahrs->quaternion.element.y),
            }};
            return halfMagnetic;
        }
        case FusionConventionNed: {
            const FusionVector halfMagnetic = {.axis = {
                .x = -(ahrs->quaternion.element.x * ahrs->quaternion.element.y + ahrs->quaternion.element.w * ahrs->quaternion.element.z),
                .y = 0.5f - ahrs->quaternion.element.w * ahrs->quaternion.element.w - ahrs->quaternion.element.y * ahrs->quaternion.element.y,
                .z = ahrs->quaternion.element.w * ahrs->quaternion.element.x - ahrs->quaternion.element.y * ahrs->quaternion.element.z,
            }};
            return halfMagnetic;
        }
    }
    return FUSION_VECTOR_ZERO;
}

static inline FusionVector FusionAhrsFeedback(FusionVector sensor, FusionVector reference) {
    if (FusionVectorDotProduct(sensor, reference) < 0.0f) {
        return FusionVectorNormalise(FusionVectorCrossProduct(sensor, reference));
    }
    return FusionVectorCrossProduct(sensor, reference);
}

static inline int FusionAhrsClamp(int value, int min, int max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}
