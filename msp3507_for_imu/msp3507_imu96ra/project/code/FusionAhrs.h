/**
 * @file FusionAhrs.h
 * @author Seb Madgwick
 * @brief AHRS algorithm to combine gyroscope, accelerometer, and magnetometer.
 */

#ifndef FUSION_AHRS_H
#define FUSION_AHRS_H

#include "FusionConvention.h"
#include "FusionMath.h"

#include <stdbool.h>

typedef struct {
    FusionConvention convention;
    float gain;
    float gyroscopeRange;
    float accelerationRejection;
    float magneticRejection;
    unsigned int recoveryTriggerPeriod;
} FusionAhrsSettings;

typedef struct {
    FusionAhrsSettings settings;
    FusionQuaternion quaternion;
    FusionVector accelerometer;
    bool initialising;
    float rampedGain;
    float rampedGainStep;
    bool angularRateRecovery;
    FusionVector halfAccelerometerFeedback;
    FusionVector halfMagnetometerFeedback;
    bool accelerometerIgnored;
    int accelerationRecoveryTrigger;
    int accelerationRecoveryTimeout;
    bool magnetometerIgnored;
    int magneticRecoveryTrigger;
    int magneticRecoveryTimeout;
} FusionAhrs;

void FusionAhrsInitialise(FusionAhrs *const ahrs);
void FusionAhrsReset(FusionAhrs *const ahrs);
void FusionAhrsSetSettings(FusionAhrs *const ahrs, const FusionAhrsSettings *const settings);
void FusionAhrsUpdate(FusionAhrs *const ahrs, FusionVector gyroscope, FusionVector accelerometer, FusionVector magnetometer, float deltaTime);
void FusionAhrsUpdateNoMagnetometer(FusionAhrs *const ahrs, FusionVector gyroscope, FusionVector accelerometer, float deltaTime);
FusionQuaternion FusionAhrsGetQuaternion(const FusionAhrs *const ahrs);
void FusionAhrsSetHeading(FusionAhrs *const ahrs, float heading);

#endif
