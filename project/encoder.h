#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

typedef struct {
    int32_t count;
    int8_t  polarity;
} Encoder;

extern Encoder EncoderA;
extern Encoder EncoderB;

void Encoder_Init(void);
int32_t Encoder_GetCountA(void);
int32_t Encoder_GetCountB(void);

#endif
