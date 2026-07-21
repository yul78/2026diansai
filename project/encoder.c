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

void Encoder_Init(void)
{
    EncoderA.count = 0;
    EncoderA.polarity = 1;
    EncoderB.count = 0;
    EncoderB.polarity = 1;
    NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);
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
