#include "encoder.h"
#include "ti_msp_dl_config.h"

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
    NVIC_EnableIRQ(ENCODER_INT_IRQN);
}



int32_t Encoder_GetCountA(void)
{
    return EncoderA.count;
}

int32_t Encoder_GetCountB(void)
{
    return EncoderB.count;
}

void GROUP1_IRQHandler(void)
{
    
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(ENCODER_PORT,
        ENCODER_MOTOR_A_A_PIN | ENCODER_MOTOR_B_A_PIN);

    if (status & ENCODER_MOTOR_A_A_PIN) {
        if (DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_A_B_PIN)) {
            EncoderA.count -= EncoderA.polarity;
        } else {
            EncoderA.count += EncoderA.polarity;
        }
    }

    if (status & ENCODER_MOTOR_B_A_PIN) {
        if (DL_GPIO_readPins(ENCODER_PORT, ENCODER_MOTOR_B_B_PIN)) {
            EncoderB.count -= EncoderB.polarity;
        } else {
            EncoderB.count += EncoderB.polarity;
        }
    }

    DL_GPIO_clearInterruptStatus(ENCODER_PORT, status);
}