/**
 * @file FusionConvention.h
 * @author Seb Madgwick
 * @brief Earth axes convention.
 */

#ifndef FUSION_CONVENTION_H
#define FUSION_CONVENTION_H

typedef enum {
    FusionConventionNwu, /* North-West-Up */
    FusionConventionEnu, /* East-North-Up */
    FusionConventionNed, /* North-East-Down */
} FusionConvention;

#endif
