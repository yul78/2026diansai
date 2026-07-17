#ifndef TI_MSP_DL_CONFIG_H
#define TI_MSP_DL_CONFIG_H

#include <stdint.h>

typedef int GPIO_Regs;

static GPIO_Regs gpioa_stub = 0;
static GPIO_Regs gpiob_stub = 0;

#define GPIOA (&gpioa_stub)
#define GPIOB (&gpiob_stub)

#define DL_GPIO_PIN_0  (1u << 0)
#define DL_GPIO_PIN_1  (1u << 1)
#define DL_GPIO_PIN_2  (1u << 2)
#define DL_GPIO_PIN_3  (1u << 3)
#define DL_GPIO_PIN_4  (1u << 4)
#define DL_GPIO_PIN_5  (1u << 5)
#define DL_GPIO_PIN_6  (1u << 6)
#define DL_GPIO_PIN_7  (1u << 7)
#define DL_GPIO_PIN_8  (1u << 8)
#define DL_GPIO_PIN_9  (1u << 9)
#define DL_GPIO_PIN_11 (1u << 11)
#define DL_GPIO_PIN_12 (1u << 12)
#define DL_GPIO_PIN_16 (1u << 16)
#define DL_GPIO_PIN_17 (1u << 17)
#define DL_GPIO_PIN_22 (1u << 22)
#define DL_GPIO_PIN_24 (1u << 24)
#define DL_GPIO_PIN_25 (1u << 25)
#define DL_GPIO_PIN_27 (1u << 27)
#define DL_GPIO_PIN_29 (1u << 29)
#define DL_GPIO_PIN_30 (1u << 30)

#define XUNJI_XJ1_PORT GPIOA
#define XUNJI_XJ1_PIN  DL_GPIO_PIN_27
#define XUNJI_XJ2_PORT GPIOB
#define XUNJI_XJ2_PIN  DL_GPIO_PIN_27
#define XUNJI_XJ3_PORT GPIOB
#define XUNJI_XJ3_PIN  DL_GPIO_PIN_25
#define XUNJI_XJ4_PORT GPIOA
#define XUNJI_XJ4_PIN  DL_GPIO_PIN_24
#define XUNJI_XJ5_PORT GPIOB
#define XUNJI_XJ5_PIN  DL_GPIO_PIN_24
#define XUNJI_XJ6_PORT GPIOB
#define XUNJI_XJ6_PIN  DL_GPIO_PIN_22
#define XUNJI_XJ7_PORT GPIOB
#define XUNJI_XJ7_PIN  DL_GPIO_PIN_16
#define XUNJI_XJ8_PORT GPIOB
#define XUNJI_XJ8_PIN  DL_GPIO_PIN_17

static inline uint32_t DL_GPIO_readPins(GPIO_Regs *port, uint32_t pin)
{
    (void)port;
    (void)pin;
    return 0U;
}

#endif
