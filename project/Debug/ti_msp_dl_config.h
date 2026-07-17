/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)


#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)
#define CPUCLK_FREQ                                                     80000000
/* Defines for SYSPLL_ERR_01 Workaround */
/* Represent 1.000 as 1000 */
#define FLOAT_TO_INT_SCALE                                               (1000U)
#define FCC_EXPECTED_RATIO                                                  2000
#define FCC_UPPER_BOUND                       (FCC_EXPECTED_RATIO * (1 + 0.003))
#define FCC_LOWER_BOUND                       (FCC_EXPECTED_RATIO * (1 - 0.003))

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);


/* Defines for PWM_TB6612 */
#define PWM_TB6612_INST                                                    TIMG6
#define PWM_TB6612_INST_IRQHandler                              TIMG6_IRQHandler
#define PWM_TB6612_INST_INT_IRQN                                (TIMG6_INT_IRQn)
#define PWM_TB6612_INST_CLK_FREQ                                        80000000
/* GPIO defines for channel 0 */
#define GPIO_PWM_TB6612_C0_PORT                                            GPIOB
#define GPIO_PWM_TB6612_C0_PIN                                     DL_GPIO_PIN_6
#define GPIO_PWM_TB6612_C0_IOMUX                                 (IOMUX_PINCM23)
#define GPIO_PWM_TB6612_C0_IOMUX_FUNC                IOMUX_PINCM23_PF_TIMG6_CCP0
#define GPIO_PWM_TB6612_C0_IDX                               DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_PWM_TB6612_C1_PORT                                            GPIOB
#define GPIO_PWM_TB6612_C1_PIN                                     DL_GPIO_PIN_7
#define GPIO_PWM_TB6612_C1_IOMUX                                 (IOMUX_PINCM24)
#define GPIO_PWM_TB6612_C1_IOMUX_FUNC                IOMUX_PINCM24_PF_TIMG6_CCP1
#define GPIO_PWM_TB6612_C1_IDX                               DL_TIMER_CC_1_INDEX



/* Defines for UART_JY901 */
#define UART_JY901_INST                                                    UART1
#define UART_JY901_INST_FREQUENCY                                       40000000
#define UART_JY901_INST_IRQHandler                              UART1_IRQHandler
#define UART_JY901_INST_INT_IRQN                                  UART1_INT_IRQn
#define GPIO_UART_JY901_RX_PORT                                            GPIOA
#define GPIO_UART_JY901_TX_PORT                                            GPIOA
#define GPIO_UART_JY901_RX_PIN                                     DL_GPIO_PIN_9
#define GPIO_UART_JY901_TX_PIN                                     DL_GPIO_PIN_8
#define GPIO_UART_JY901_IOMUX_RX                                 (IOMUX_PINCM20)
#define GPIO_UART_JY901_IOMUX_TX                                 (IOMUX_PINCM19)
#define GPIO_UART_JY901_IOMUX_RX_FUNC                  IOMUX_PINCM20_PF_UART1_RX
#define GPIO_UART_JY901_IOMUX_TX_FUNC                  IOMUX_PINCM19_PF_UART1_TX
#define UART_JY901_BAUD_RATE                                              (9600)
#define UART_JY901_IBRD_40_MHZ_9600_BAUD                                   (260)
#define UART_JY901_FBRD_40_MHZ_9600_BAUD                                    (27)
/* Defines for UART_BT */
#define UART_BT_INST                                                       UART2
#define UART_BT_INST_FREQUENCY                                          40000000
#define UART_BT_INST_IRQHandler                                 UART2_IRQHandler
#define UART_BT_INST_INT_IRQN                                     UART2_INT_IRQn
#define GPIO_UART_BT_RX_PORT                                               GPIOB
#define GPIO_UART_BT_TX_PORT                                               GPIOB
#define GPIO_UART_BT_RX_PIN                                       DL_GPIO_PIN_16
#define GPIO_UART_BT_TX_PIN                                       DL_GPIO_PIN_17
#define GPIO_UART_BT_IOMUX_RX                                    (IOMUX_PINCM33)
#define GPIO_UART_BT_IOMUX_TX                                    (IOMUX_PINCM43)
#define GPIO_UART_BT_IOMUX_RX_FUNC                     IOMUX_PINCM33_PF_UART2_RX
#define GPIO_UART_BT_IOMUX_TX_FUNC                     IOMUX_PINCM43_PF_UART2_TX
#define UART_BT_BAUD_RATE                                                 (9600)
#define UART_BT_IBRD_40_MHZ_9600_BAUD                                      (260)
#define UART_BT_FBRD_40_MHZ_9600_BAUD                                       (27)





/* Port definition for Pin Group Buzzer */
#define Buzzer_PORT                                                      (GPIOB)

/* Defines for num1: GPIOB.8 with pinCMx 25 on package pin 60 */
#define Buzzer_num1_PIN                                          (DL_GPIO_PIN_8)
#define Buzzer_num1_IOMUX                                        (IOMUX_PINCM25)
/* Port definition for Pin Group AJ */
#define AJ_PORT                                                          (GPIOA)

/* Defines for AJ1: GPIOA.26 with pinCMx 59 on package pin 30 */
// pins affected by this interrupt request:["AJ1"]
#define AJ_INT_IRQN                                             (GPIOA_INT_IRQn)
#define AJ_INT_IIDX                             (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define AJ_AJ1_IIDX                                         (DL_GPIO_IIDX_DIO26)
#define AJ_AJ1_PIN                                              (DL_GPIO_PIN_26)
#define AJ_AJ1_IOMUX                                             (IOMUX_PINCM59)
/* Port definition for Pin Group OLED */
#define OLED_PORT                                                        (GPIOA)

/* Defines for OLED_SCL: GPIOA.29 with pinCMx 4 on package pin 36 */
#define OLED_OLED_SCL_PIN                                       (DL_GPIO_PIN_29)
#define OLED_OLED_SCL_IOMUX                                       (IOMUX_PINCM4)
/* Defines for OLED_SDA: GPIOA.30 with pinCMx 5 on package pin 37 */
#define OLED_OLED_SDA_PIN                                       (DL_GPIO_PIN_30)
#define OLED_OLED_SDA_IOMUX                                       (IOMUX_PINCM5)
/* Port definition for Pin Group MOTOR_DIR */
#define MOTOR_DIR_PORT                                                   (GPIOB)

/* Defines for MOTOR_A_IN1: GPIOB.0 with pinCMx 12 on package pin 47 */
#define MOTOR_DIR_MOTOR_A_IN1_PIN                                (DL_GPIO_PIN_0)
#define MOTOR_DIR_MOTOR_A_IN1_IOMUX                              (IOMUX_PINCM12)
/* Defines for MOTOR_A_IN2: GPIOB.1 with pinCMx 13 on package pin 48 */
#define MOTOR_DIR_MOTOR_A_IN2_PIN                                (DL_GPIO_PIN_1)
#define MOTOR_DIR_MOTOR_A_IN2_IOMUX                              (IOMUX_PINCM13)
/* Defines for MOTOR_B_IN1: GPIOB.2 with pinCMx 15 on package pin 50 */
#define MOTOR_DIR_MOTOR_B_IN1_PIN                                (DL_GPIO_PIN_2)
#define MOTOR_DIR_MOTOR_B_IN1_IOMUX                              (IOMUX_PINCM15)
/* Defines for MOTOR_B_IN2: GPIOB.3 with pinCMx 16 on package pin 51 */
#define MOTOR_DIR_MOTOR_B_IN2_PIN                                (DL_GPIO_PIN_3)
#define MOTOR_DIR_MOTOR_B_IN2_IOMUX                              (IOMUX_PINCM16)
/* Port definition for Pin Group ENCODER */
#define ENCODER_PORT                                                     (GPIOB)

/* Defines for MOTOR_A_A: GPIOB.4 with pinCMx 17 on package pin 52 */
// pins affected by this interrupt request:["MOTOR_A_A","MOTOR_B_A"]
#define ENCODER_INT_IRQN                                        (GPIOB_INT_IRQn)
#define ENCODER_INT_IIDX                        (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define ENCODER_MOTOR_A_A_IIDX                               (DL_GPIO_IIDX_DIO4)
#define ENCODER_MOTOR_A_A_PIN                                    (DL_GPIO_PIN_4)
#define ENCODER_MOTOR_A_A_IOMUX                                  (IOMUX_PINCM17)
/* Defines for MOTOR_A_B: GPIOB.12 with pinCMx 29 on package pin 64 */
#define ENCODER_MOTOR_A_B_PIN                                   (DL_GPIO_PIN_12)
#define ENCODER_MOTOR_A_B_IOMUX                                  (IOMUX_PINCM29)
/* Defines for MOTOR_B_A: GPIOB.11 with pinCMx 28 on package pin 63 */
#define ENCODER_MOTOR_B_A_IIDX                              (DL_GPIO_IIDX_DIO11)
#define ENCODER_MOTOR_B_A_PIN                                   (DL_GPIO_PIN_11)
#define ENCODER_MOTOR_B_A_IOMUX                                  (IOMUX_PINCM28)
/* Defines for MOTOR_B_B: GPIOB.5 with pinCMx 18 on package pin 53 */
#define ENCODER_MOTOR_B_B_PIN                                    (DL_GPIO_PIN_5)
#define ENCODER_MOTOR_B_B_IOMUX                                  (IOMUX_PINCM18)
/* Defines for XJ1: GPIOA.27 with pinCMx 60 on package pin 31 */
#define XUNJI_XJ1_PORT                                                   (GPIOA)
#define XUNJI_XJ1_PIN                                           (DL_GPIO_PIN_27)
#define XUNJI_XJ1_IOMUX                                          (IOMUX_PINCM60)
/* Defines for XJ2: GPIOB.27 with pinCMx 58 on package pin 29 */
#define XUNJI_XJ2_PORT                                                   (GPIOB)
#define XUNJI_XJ2_PIN                                           (DL_GPIO_PIN_27)
#define XUNJI_XJ2_IOMUX                                          (IOMUX_PINCM58)
/* Defines for XJ3: GPIOB.25 with pinCMx 56 on package pin 27 */
#define XUNJI_XJ3_PORT                                                   (GPIOB)
#define XUNJI_XJ3_PIN                                           (DL_GPIO_PIN_25)
#define XUNJI_XJ3_IOMUX                                          (IOMUX_PINCM56)
/* Defines for XJ4: GPIOA.24 with pinCMx 54 on package pin 25 */
#define XUNJI_XJ4_PORT                                                   (GPIOA)
#define XUNJI_XJ4_PIN                                           (DL_GPIO_PIN_24)
#define XUNJI_XJ4_IOMUX                                          (IOMUX_PINCM54)
/* Defines for XJ5: GPIOB.24 with pinCMx 52 on package pin 23 */
#define XUNJI_XJ5_PORT                                                   (GPIOB)
#define XUNJI_XJ5_PIN                                           (DL_GPIO_PIN_24)
#define XUNJI_XJ5_IOMUX                                          (IOMUX_PINCM52)
/* Defines for XJ6: GPIOB.22 with pinCMx 50 on package pin 21 */
#define XUNJI_XJ6_PORT                                                   (GPIOB)
#define XUNJI_XJ6_PIN                                           (DL_GPIO_PIN_22)
#define XUNJI_XJ6_IOMUX                                          (IOMUX_PINCM50)
/* Defines for XJ7: GPIOB.20 with pinCMx 48 on package pin 19 */
#define XUNJI_XJ7_PORT                                                   (GPIOB)
#define XUNJI_XJ7_PIN                                           (DL_GPIO_PIN_20)
#define XUNJI_XJ7_IOMUX                                          (IOMUX_PINCM48)
/* Defines for XJ8: GPIOB.18 with pinCMx 44 on package pin 15 */
#define XUNJI_XJ8_PORT                                                   (GPIOB)
#define XUNJI_XJ8_PIN                                           (DL_GPIO_PIN_18)
#define XUNJI_XJ8_IOMUX                                          (IOMUX_PINCM44)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);

bool SYSCFG_DL_SYSCTL_SYSPLL_init(void);
void SYSCFG_DL_PWM_TB6612_init(void);
void SYSCFG_DL_UART_JY901_init(void);
void SYSCFG_DL_UART_BT_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
