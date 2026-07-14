#include "app_jy901.h"

void APP_JY901_Init(void)
{
    BSP_JY901_Init();
}

void JY901_Task(void)
{
    uint8_t byte;

    while (BSP_JY901_ReadByte(&byte))
    {
        JY901_Feed(byte);
    }
}
