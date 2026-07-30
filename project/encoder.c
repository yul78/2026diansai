#include "encoder.h"
#include "ti_msp_dl_config.h"

extern uint64_t g_systick_ms;

/*
    图形化配置说明
    GPIO组命名为ENCODER，电机A编码器连接的两个GPIO命名为 MOTOR_A_A 和 MOTOR_A_B
                       ，电机B编码器连接的两个GPIO命名为 MOTOR_B_A 和 MOTOR_B_B
*/

Encoder EncoderA;
Encoder EncoderB;

static const int8_t quad_table[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

static uint8_t encoderA_last;
static uint8_t encoderB_last;

#define ENCODER_ALL_PINS  (ENCODER_MOTOR_A_A_PIN | ENCODER_MOTOR_A_B_PIN | \
                           ENCODER_MOTOR_B_A_PIN | ENCODER_MOTOR_B_B_PIN)

void Encoder_Init(void)
{
    EncoderA.count = 0;
    EncoderA.polarity = 1;
    EncoderB.count = 0;
    EncoderB.polarity = -1;

    encoderA_last = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_A_A_PIN) ? 2 : 0)
                  | (DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_A_B_PIN) ? 1 : 0);
    encoderB_last = (DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_B_A_PIN) ? 2 : 0)
                  | (DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_B_B_PIN) ? 1 : 0);

    DL_GPIO_enableInterrupt(ENCODER_PORT, ENCODER_MOTOR_A_B_PIN);
    NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);
}

void Encoder_IRQHandler(void)
{
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT, ENCODER_ALL_PINS);
    if (status == 0) return;

    if (status & (ENCODER_MOTOR_A_A_PIN | ENCODER_MOTOR_A_B_PIN)) {
        uint8_t a = DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_A_A_PIN) ? 2 : 0;
        uint8_t b = DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_A_B_PIN) ? 1 : 0;
        uint8_t curr = a | b;
        uint8_t idx = (encoderA_last << 2) | curr;
        EncoderA.count += (int32_t)quad_table[idx] * EncoderA.polarity;
        encoderA_last = curr;
    }

    if (status & (ENCODER_MOTOR_B_A_PIN | ENCODER_MOTOR_B_B_PIN)) {
        uint8_t a = DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_B_A_PIN) ? 2 : 0;
        uint8_t b = DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_B_B_PIN) ? 1 : 0;
        uint8_t curr = a | b;
        uint8_t idx = (encoderB_last << 2) | curr;
        EncoderB.count += (int32_t)quad_table[idx] * EncoderB.polarity;
        encoderB_last = curr;
    }

    DL_GPIO_clearInterruptStatus(ENCODER_PORT, status);
}

int32_t Encoder_GetCountA(void)
{
    return EncoderA.count;
}

int32_t Encoder_GetCountB(void)
{
    return EncoderB.count;
}

void Encoder_GetSpeeds(int32_t *speedA, int32_t *speedB)
{
    *speedA = EncoderA.count;
    *speedB = EncoderB.count;

    EncoderA.count = 0;
    EncoderB.count = 0;
}
