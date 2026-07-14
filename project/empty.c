#include "ti_msp_dl_config.h"
#include "tb6612.h"
#include "encoder.h"
#include "DL_OLED.h"
#include "app_jy901.h"
#include "bluetooth.h"

int main(void)
{
    SYSCFG_DL_init();
    TB6612_Init();
    Encoder_Init();
    OLED_Init();
    APP_JY901_Init();
    Bluetooth_Init();

    while (1) {
        Bluetooth_Task();
        JY901_Task();
        OLED_ShowSignedNum(2, 1, Encoder_GetCountA(), 7);
        OLED_ShowSignedNum(3, 1, Encoder_GetCountB(), 7);
        // OLED_ShowFloat(1, 3, jy901_data.roll,  3, 1);
        // OLED_ShowFloat(2, 3, jy901_data.pitch, 3, 1);
        // OLED_ShowFloat(3, 3, jy901_data.yaw,   3, 1);
    }
}
