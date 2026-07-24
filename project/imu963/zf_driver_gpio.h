#ifndef ZF_DRIVER_GPIO_H
#define ZF_DRIVER_GPIO_H

#include "ti_msp_dl_config.h"
#include "zf_common_typedef.h"

typedef enum {
    IMU963RA_GPIO_SCL = 0,
    IMU963RA_GPIO_SDA = 1,
} gpio_pin_enum;

typedef enum { GPI = 0, GPO = 1 } gpio_dir_enum;
typedef enum { GPIO_LOW = 0, GPIO_HIGH = 1 } gpio_level_enum;
typedef enum { GPO_PUSH_PULL = 0, GPO_OPEN_DTAIN = 1 } gpio_output_mode_enum;
typedef enum { GPI_FLOATING_IN = 0 } gpio_input_mode_enum;

void gpio_init(gpio_pin_enum pin, gpio_dir_enum dir, gpio_level_enum level,
               gpio_output_mode_enum output_mode);
void gpio_set_dir(gpio_pin_enum pin, gpio_dir_enum dir, uint32 mode);
void gpio_high(uint32 pin);
void gpio_low(uint32 pin);
uint8 gpio_get_level(gpio_pin_enum pin);

#endif
