/**
 * @file FusionOffset.c
 * @author Seb Madgwick
 * @brief Gyroscope offset correction algorithm.
 */

#include "FusionOffset.h"

#include <math.h>

#define CUTOFF_FREQUENCY (0.02f)
#define TIMEOUT_SECONDS  (5U)
#define STATIONARY_DPS   (3.0f)

void FusionOffsetInitialise(FusionOffset *const offset, const unsigned int sampleRate) {
    offset->filterCoefficient = 2.0f * (float) M_PI * CUTOFF_FREQUENCY * (1.0f / (float) sampleRate);
    offset->timeout = TIMEOUT_SECONDS * sampleRate;
    offset->timer = 0U;
    offset->gyroscopeOffset = FUSION_VECTOR_ZERO;
}

FusionVector FusionOffsetUpdate(FusionOffset *const offset, FusionVector gyroscope) {
    gyroscope = FusionVectorSubtract(gyroscope, offset->gyroscopeOffset);

    if ((fabsf(gyroscope.axis.x) > STATIONARY_DPS) ||
        (fabsf(gyroscope.axis.y) > STATIONARY_DPS) ||
        (fabsf(gyroscope.axis.z) > STATIONARY_DPS)) {
        offset->timer = 0U;
        return gyroscope;
    }

    if (offset->timer < offset->timeout) {
        offset->timer++;
        return gyroscope;
    }

    offset->gyroscopeOffset = FusionVectorAdd(offset->gyroscopeOffset, FusionVectorMultiplyScalar(gyroscope, offset->filterCoefficient));
    return gyroscope;
}
