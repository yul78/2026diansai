#include "zf_driver_gpio.h"

static GPIO_Regs *gpio_port(gpio_pin_enum pin) {
    return (pin == IMU963RA_GPIO_SCL) ? IMU_IIC_IMU_SCL_PORT : IMU_IIC_IMU_SDA_PORT;
}

static uint32 gpio_mask(gpio_pin_enum pin) {
    return (pin == IMU963RA_GPIO_SCL) ? IMU_IIC_IMU_SCL_PIN : IMU_IIC_IMU_SDA_PIN;
}

static uint32 gpio_iomux(gpio_pin_enum pin) {
    return (pin == IMU963RA_GPIO_SCL) ? IMU_IIC_IMU_SCL_IOMUX : IMU_IIC_IMU_SDA_IOMUX;
}

void gpio_init(gpio_pin_enum pin, gpio_dir_enum dir, gpio_level_enum level,
               gpio_output_mode_enum output_mode) {
    (void) dir;
    (void) output_mode;
    DL_GPIO_initDigitalInputFeatures(gpio_iomux(pin),
                                     DL_GPIO_INVERSION_DISABLE,
                                     DL_GPIO_RESISTOR_PULL_UP,
                                     DL_GPIO_HYSTERESIS_DISABLE,
                                     DL_GPIO_WAKEUP_DISABLE);
    if (level == GPIO_LOW) {
        gpio_low(pin);
    } else {
        gpio_high(pin);
    }
}

void gpio_set_dir(gpio_pin_enum pin, gpio_dir_enum dir, uint32 mode) {
    (void) mode;
    if (dir == GPO) {
        DL_GPIO_enableOutput(gpio_port(pin), gpio_mask(pin));
    } else {
        DL_GPIO_disableOutput(gpio_port(pin), gpio_mask(pin));
    }
}

void gpio_high(uint32 pin) {
    /* Releasing the line implements the IIC open-drain high level. */
    DL_GPIO_disableOutput(gpio_port((gpio_pin_enum) pin), gpio_mask((gpio_pin_enum) pin));
}

void gpio_low(uint32 pin) {
    DL_GPIO_clearPins(gpio_port((gpio_pin_enum) pin), gpio_mask((gpio_pin_enum) pin));
    DL_GPIO_enableOutput(gpio_port((gpio_pin_enum) pin), gpio_mask((gpio_pin_enum) pin));
}

uint8 gpio_get_level(gpio_pin_enum pin) {
    return (DL_GPIO_readPins(gpio_port(pin), gpio_mask(pin)) != 0U) ? 1U : 0U;
}
