/**
 * @file FusionMath.h
 * @author Seb Madgwick
 * @brief Minimal math helpers required by the Fusion AHRS pipeline.
 */

#ifndef FUSION_MATH_H
#define FUSION_MATH_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

typedef union {
    float array[3];
    struct {
        float x;
        float y;
        float z;
    } axis;
} FusionVector;

typedef union {
    float array[4];
    struct {
        float w;
        float x;
        float y;
        float z;
    } element;
} FusionQuaternion;

typedef union {
    float array[3];
    struct {
        float roll;
        float pitch;
        float yaw;
    } angle;
} FusionEuler;

#define FUSION_VECTOR_ZERO ((FusionVector){ .array = {0.0f, 0.0f, 0.0f} })
#define FUSION_IDENTITY_QUATERNION ((FusionQuaternion){ .array = {1.0f, 0.0f, 0.0f, 0.0f} })

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

static inline float FusionDegreesToRadians(const float degrees) {
    return degrees * ((float) M_PI / 180.0f);
}

static inline float FusionRadiansToDegrees(const float radians) {
    return radians * (180.0f / (float) M_PI);
}

static inline float FusionAsin(const float value) {
    if (value <= -1.0f) {
        return (float) M_PI / -2.0f;
    }
    if (value >= 1.0f) {
        return (float) M_PI / 2.0f;
    }
    return asinf(value);
}

static inline float FusionFastInverseSqrt(const float value) {
    typedef union {
        float f;
        int32_t i;
    } union32_t;

    union32_t number = {.f = value};
    number.i = 0x5F1F1412 - (number.i >> 1);
    return number.f * (1.69000231f - 0.714158168f * value * number.f * number.f);
}

static inline bool FusionVectorIsZero(const FusionVector vector) {
    return (vector.axis.x == 0.0f) && (vector.axis.y == 0.0f) && (vector.axis.z == 0.0f);
}

static inline FusionVector FusionVectorAdd(const FusionVector vector_a, const FusionVector vector_b) {
    const FusionVector result = {.axis = {
        .x = vector_a.axis.x + vector_b.axis.x,
        .y = vector_a.axis.y + vector_b.axis.y,
        .z = vector_a.axis.z + vector_b.axis.z,
    }};
    return result;
}

static inline FusionVector FusionVectorSubtract(const FusionVector vector_a, const FusionVector vector_b) {
    const FusionVector result = {.axis = {
        .x = vector_a.axis.x - vector_b.axis.x,
        .y = vector_a.axis.y - vector_b.axis.y,
        .z = vector_a.axis.z - vector_b.axis.z,
    }};
    return result;
}

static inline FusionVector FusionVectorMultiplyScalar(const FusionVector vector, const float scalar) {
    const FusionVector result = {.axis = {
        .x = vector.axis.x * scalar,
        .y = vector.axis.y * scalar,
        .z = vector.axis.z * scalar,
    }};
    return result;
}

static inline FusionVector FusionVectorHadamardProduct(const FusionVector vector_a, const FusionVector vector_b) {
    const FusionVector result = {.axis = {
        .x = vector_a.axis.x * vector_b.axis.x,
        .y = vector_a.axis.y * vector_b.axis.y,
        .z = vector_a.axis.z * vector_b.axis.z,
    }};
    return result;
}

static inline float FusionVectorDotProduct(const FusionVector vector_a, const FusionVector vector_b) {
    return vector_a.axis.x * vector_b.axis.x + vector_a.axis.y * vector_b.axis.y + vector_a.axis.z * vector_b.axis.z;
}

static inline FusionVector FusionVectorCrossProduct(const FusionVector vector_a, const FusionVector vector_b) {
    const FusionVector result = {.axis = {
        .x = vector_a.axis.y * vector_b.axis.z - vector_a.axis.z * vector_b.axis.y,
        .y = vector_a.axis.z * vector_b.axis.x - vector_a.axis.x * vector_b.axis.z,
        .z = vector_a.axis.x * vector_b.axis.y - vector_a.axis.y * vector_b.axis.x,
    }};
    return result;
}

static inline float FusionVectorMagnitudeSquared(const FusionVector vector) {
    return FusionVectorDotProduct(vector, vector);
}

static inline float FusionVectorMagnitude(const FusionVector vector) {
    return sqrtf(FusionVectorMagnitudeSquared(vector));
}

static inline FusionVector FusionVectorNormalise(const FusionVector vector) {
    const float reciprocal = FusionFastInverseSqrt(FusionVectorMagnitudeSquared(vector));
    return FusionVectorMultiplyScalar(vector, reciprocal);
}

static inline FusionQuaternion FusionQuaternionAdd(const FusionQuaternion quaternion_a, const FusionQuaternion quaternion_b) {
    const FusionQuaternion result = {.element = {
        .w = quaternion_a.element.w + quaternion_b.element.w,
        .x = quaternion_a.element.x + quaternion_b.element.x,
        .y = quaternion_a.element.y + quaternion_b.element.y,
        .z = quaternion_a.element.z + quaternion_b.element.z,
    }};
    return result;
}

static inline FusionQuaternion FusionQuaternionMultiply(const FusionQuaternion quaternion_a, const FusionQuaternion quaternion_b) {
    const FusionQuaternion result = {.element = {
        .w = quaternion_a.element.w * quaternion_b.element.w - quaternion_a.element.x * quaternion_b.element.x - quaternion_a.element.y * quaternion_b.element.y - quaternion_a.element.z * quaternion_b.element.z,
        .x = quaternion_a.element.w * quaternion_b.element.x + quaternion_a.element.x * quaternion_b.element.w + quaternion_a.element.y * quaternion_b.element.z - quaternion_a.element.z * quaternion_b.element.y,
        .y = quaternion_a.element.w * quaternion_b.element.y - quaternion_a.element.x * quaternion_b.element.z + quaternion_a.element.y * quaternion_b.element.w + quaternion_a.element.z * quaternion_b.element.x,
        .z = quaternion_a.element.w * quaternion_b.element.z + quaternion_a.element.x * quaternion_b.element.y - quaternion_a.element.y * quaternion_b.element.x + quaternion_a.element.z * quaternion_b.element.w,
    }};
    return result;
}

static inline FusionQuaternion FusionQuaternionMultiplyVector(const FusionQuaternion quaternion, const FusionVector vector) {
    const FusionQuaternion result = {.element = {
        .w = -quaternion.element.x * vector.axis.x - quaternion.element.y * vector.axis.y - quaternion.element.z * vector.axis.z,
        .x = quaternion.element.w * vector.axis.x + quaternion.element.y * vector.axis.z - quaternion.element.z * vector.axis.y,
        .y = quaternion.element.w * vector.axis.y - quaternion.element.x * vector.axis.z + quaternion.element.z * vector.axis.x,
        .z = quaternion.element.w * vector.axis.z + quaternion.element.x * vector.axis.y - quaternion.element.y * vector.axis.x,
    }};
    return result;
}

static inline FusionQuaternion FusionQuaternionNormalise(const FusionQuaternion quaternion) {
    const float reciprocal = FusionFastInverseSqrt(
        quaternion.element.w * quaternion.element.w +
        quaternion.element.x * quaternion.element.x +
        quaternion.element.y * quaternion.element.y +
        quaternion.element.z * quaternion.element.z
    );
    const FusionQuaternion result = {.element = {
        .w = quaternion.element.w * reciprocal,
        .x = quaternion.element.x * reciprocal,
        .y = quaternion.element.y * reciprocal,
        .z = quaternion.element.z * reciprocal,
    }};
    return result;
}

static inline FusionEuler FusionQuaternionToEuler(const FusionQuaternion quaternion) {
    const float half_minus_qy_squared = 0.5f - quaternion.element.y * quaternion.element.y;
    const FusionEuler euler = {.angle = {
        .roll = FusionRadiansToDegrees(atan2f(quaternion.element.w * quaternion.element.x + quaternion.element.y * quaternion.element.z,
                                              half_minus_qy_squared - quaternion.element.x * quaternion.element.x)),
        .pitch = FusionRadiansToDegrees(FusionAsin(2.0f * (quaternion.element.w * quaternion.element.y - quaternion.element.z * quaternion.element.x))),
        .yaw = FusionRadiansToDegrees(atan2f(quaternion.element.w * quaternion.element.z + quaternion.element.x * quaternion.element.y,
                                             half_minus_qy_squared - quaternion.element.z * quaternion.element.z)),
    }};
    return euler;
}

#endif
