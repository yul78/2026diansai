/**
 * @file FusionOffset.h
 * @author Seb Madgwick
 * @brief Gyroscope offset correction algorithm.
 */

#ifndef FUSION_OFFSET_H
#define FUSION_OFFSET_H

#include "FusionMath.h"

typedef struct {
    float filterCoefficient;
    unsigned int timeout;
    unsigned int timer;
    FusionVector gyroscopeOffset;
} FusionOffset;

void FusionOffsetInitialise(FusionOffset *const offset, const unsigned int sampleRate);
FusionVector FusionOffsetUpdate(FusionOffset *const offset, FusionVector gyroscope);

#endif
