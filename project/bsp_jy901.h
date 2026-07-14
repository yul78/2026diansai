#ifndef __BSP_JY901_H
#define __BSP_JY901_H

#include "ti_msp_dl_config.h"

#define JY_Buffer_Size  256

typedef struct
{
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint8_t overflow;
    uint8_t buffer[JY_Buffer_Size];
} jy901_uart_t;

extern jy901_uart_t jy901_uart;

void BSP_JY901_Init(void);
void BSP_JY901_PushByte(uint8_t byte);
uint8_t BSP_JY901_ReadByte(uint8_t *byte);

#endif
