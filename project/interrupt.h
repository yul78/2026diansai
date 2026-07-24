#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include "encoder.h"
#include "ti_msp_dl_config.h"
#include "vofa_debug.h"

extern int32_t l_speed_now, r_speed_now;
extern volatile uint8_t imu963ra_update_flag;
extern volatile uint8_t imu963ra_display_flag;
extern volatile uint32_t imu963ra_tick_ms;


#endif
