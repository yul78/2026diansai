#ifndef TB6612_H
#define TB6612_H

#include <stdint.h>

void TB6612_Init(void);
void TB6612_SetMotorA(int16_t speed);
void TB6612_SetMotorB(int16_t speed);
void TB6612_SetMotors(int16_t motorA, int16_t motorB);
void TB6612_Stop(void);

#endif
