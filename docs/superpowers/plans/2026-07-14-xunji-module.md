# Xunji Module Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a reusable 8-channel line-tracking module with one-place macro configuration for polarity, logical left/right direction, and per-channel GPIO remapping, plus helper APIs for raw reads, weighted position, center error, and common race-state detection.

**Architecture:** Implement a dedicated `project/xunji.h` public API and `project/xunji.c` implementation that read the existing SysConfig-generated GPIO macros from `project/Debug/ti_msp_dl_config.h`. Keep all race-day adjustments in a single header macro section so remapping a sensor from `XJ1` to `XJ6`, flipping black-line polarity, or reversing logical left/right never requires touching the algorithm code.

**Tech Stack:** C, MSPM0G3507, TI DriverLib, SysConfig-generated GPIO macros, tiarmclang toolchain.

## Global Constraints

- Do not edit `project/empty.syscfg` or `project/Debug/ti_msp_dl_config.[ch]`; the eight GPIOs are already initialized there.
- All race-day adjustments must live in `project/xunji.h` macros.
- Keep the module stateless; this version does not remember the last valid position.
- `Xunji_GetPosition()` must return `3500` when no sensor sees the line.
- `Xunji_GetError()` must return `position - 3500`.
- Provide both threshold-based and fixed-pattern helpers for cross and T-junction detection.
- Do not add a new dependency or test framework.
- Do not create a git commit unless the user explicitly asks.

---

### Task 1: Create the public header and race-day macro configuration

**Files:**
- Create: `project/xunji.h`
- Test: `project/Debug/xunji_api_smoke.c` (temporary local verification file, do not commit)

**Interfaces:**
- Consumes: `project/Debug/ti_msp_dl_config.h` GPIO macros `XUNJI_XJ1_PORT` … `XUNJI_XJ8_PIN`
- Produces:
  - `uint8_t Xunji_ReadRaw(uint8_t index);`
  - `uint8_t Xunji_IsActive(uint8_t index);`
  - `uint8_t Xunji_ReadBits(void);`
  - `uint8_t Xunji_CountActive(uint8_t bits);`
  - `uint16_t Xunji_GetPosition(uint8_t bits);`
  - `int16_t Xunji_GetError(uint8_t bits);`
  - `bool Xunji_IsAllWhite(uint8_t bits);`
  - `bool Xunji_IsAllBlack(uint8_t bits);`
  - `bool Xunji_IsLost(uint8_t bits);`
  - `bool Xunji_IsCrossByThreshold(uint8_t bits);`
  - `bool Xunji_IsCrossByPattern(uint8_t bits);`
  - `bool Xunji_IsTJunctionByThreshold(uint8_t bits);`
  - `bool Xunji_IsTJunctionByPattern(uint8_t bits);`

- [ ] **Step 1: Write the failing API smoke file**

```c
#include <stdbool.h>
#include <stdint.h>
#include "xunji.h"

int main(void)
{
    uint8_t bits = Xunji_ReadBits();
    uint16_t position = Xunji_GetPosition(bits);
    int16_t error = Xunji_GetError(bits);
    bool cross = Xunji_IsCrossByThreshold(bits);

    return (int)(position + (uint16_t)error + (cross ? 1U : 0U));
}
```

Save the file as `project/Debug/xunji_api_smoke.c`.

- [ ] **Step 2: Compile the smoke file to verify the header is still missing**

Run:

```bash
"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -o "project/Debug/xunji_api_smoke.o" "project/Debug/xunji_api_smoke.c"
```

Expected: FAIL with a missing-header error for `xunji.h`.

- [ ] **Step 3: Write `project/xunji.h` with all configurable macros and public declarations**

```c
#ifndef XUNJI_H
#define XUNJI_H

#include <stdbool.h>
#include <stdint.h>
#include "ti_msp_dl_config.h"

#define XUNJI_SENSOR_COUNT 8U
#define XUNJI_POSITION_CENTER 3500U
#define XUNJI_POSITION_LOST   XUNJI_POSITION_CENTER

#define XUNJI_BLACK_ACTIVE_LOW  0U
#define XUNJI_BLACK_ACTIVE_HIGH 1U
#define XUNJI_BLACK_POLARITY    XUNJI_BLACK_ACTIVE_LOW

#define XUNJI_ORDER_LEFT_TO_RIGHT  0U
#define XUNJI_ORDER_RIGHT_TO_LEFT  1U
#define XUNJI_SENSOR_ORDER         XUNJI_ORDER_LEFT_TO_RIGHT

#define XUNJI_CROSS_ACTIVE_MIN   6U
#define XUNJI_T_ACTIVE_MIN       5U
#define XUNJI_T_CENTER_MIN       2U
#define XUNJI_PATTERN_CROSS      0xFFU
#define XUNJI_PATTERN_T          0x3CU

#define XUNJI_READ_PIN(port, pin) ((DL_GPIO_readPins((port), (pin)) != 0U) ? 1U : 0U)

#define XUNJI_HW_READ_0() XUNJI_READ_PIN(XUNJI_XJ1_PORT, XUNJI_XJ1_PIN)
#define XUNJI_HW_READ_1() XUNJI_READ_PIN(XUNJI_XJ2_PORT, XUNJI_XJ2_PIN)
#define XUNJI_HW_READ_2() XUNJI_READ_PIN(XUNJI_XJ3_PORT, XUNJI_XJ3_PIN)
#define XUNJI_HW_READ_3() XUNJI_READ_PIN(XUNJI_XJ4_PORT, XUNJI_XJ4_PIN)
#define XUNJI_HW_READ_4() XUNJI_READ_PIN(XUNJI_XJ5_PORT, XUNJI_XJ5_PIN)
#define XUNJI_HW_READ_5() XUNJI_READ_PIN(XUNJI_XJ6_PORT, XUNJI_XJ6_PIN)
#define XUNJI_HW_READ_6() XUNJI_READ_PIN(XUNJI_XJ7_PORT, XUNJI_XJ7_PIN)
#define XUNJI_HW_READ_7() XUNJI_READ_PIN(XUNJI_XJ8_PORT, XUNJI_XJ8_PIN)

uint8_t Xunji_ReadRaw(uint8_t index);
uint8_t Xunji_IsActive(uint8_t index);
uint8_t Xunji_ReadBits(void);
uint8_t Xunji_CountActive(uint8_t bits);
uint16_t Xunji_GetPosition(uint8_t bits);
int16_t Xunji_GetError(uint8_t bits);
bool Xunji_IsAllWhite(uint8_t bits);
bool Xunji_IsAllBlack(uint8_t bits);
bool Xunji_IsLost(uint8_t bits);
bool Xunji_IsCrossByThreshold(uint8_t bits);
bool Xunji_IsCrossByPattern(uint8_t bits);
bool Xunji_IsTJunctionByThreshold(uint8_t bits);
bool Xunji_IsTJunctionByPattern(uint8_t bits);

#endif
```

- [ ] **Step 4: Re-run the API smoke compile to verify the header passes**

Run:

```bash
"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -o "project/Debug/xunji_api_smoke.o" "project/Debug/xunji_api_smoke.c"
```

Expected: PASS and `project/Debug/xunji_api_smoke.o` is created.

- [ ] **Step 5: Leave the repo in a clean verification state**

Run:

```bash
rm -f "project/Debug/xunji_api_smoke.o"
```

Expected: the temporary object file is removed while `project/Debug/xunji_api_smoke.c` remains available for Task 2 verification.

### Task 2: Implement the line-tracking logic in `xunji.c`

**Files:**
- Create: `project/xunji.c`
- Test: `project/Debug/xunji_api_smoke.c` (from Task 1)

**Interfaces:**
- Consumes:
  - `uint8_t Xunji_ReadRaw(uint8_t index);`
  - `uint8_t Xunji_IsActive(uint8_t index);`
  - `uint8_t Xunji_ReadBits(void);`
  - `uint8_t Xunji_CountActive(uint8_t bits);`
- Produces:
  - `uint16_t Xunji_GetPosition(uint8_t bits);`
  - `int16_t Xunji_GetError(uint8_t bits);`
  - `bool Xunji_IsAllWhite(uint8_t bits);`
  - `bool Xunji_IsAllBlack(uint8_t bits);`
  - `bool Xunji_IsLost(uint8_t bits);`
  - `bool Xunji_IsCrossByThreshold(uint8_t bits);`
  - `bool Xunji_IsCrossByPattern(uint8_t bits);`
  - `bool Xunji_IsTJunctionByThreshold(uint8_t bits);`
  - `bool Xunji_IsTJunctionByPattern(uint8_t bits);`

- [ ] **Step 1: Compile the implementation target before the source exists**

Run:

```bash
"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"project/Debug/device.opt" -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -o "project/Debug/xunji.o" "project/xunji.c"
```

Expected: FAIL with a missing-source error for `project/xunji.c`.

- [ ] **Step 2: Write `project/xunji.c` with the minimal full implementation**

```c
#include "xunji.h"

#if (XUNJI_BLACK_POLARITY != XUNJI_BLACK_ACTIVE_LOW) && (XUNJI_BLACK_POLARITY != XUNJI_BLACK_ACTIVE_HIGH)
#error "Invalid XUNJI_BLACK_POLARITY"
#endif

#if (XUNJI_SENSOR_ORDER != XUNJI_ORDER_LEFT_TO_RIGHT) && (XUNJI_SENSOR_ORDER != XUNJI_ORDER_RIGHT_TO_LEFT)
#error "Invalid XUNJI_SENSOR_ORDER"
#endif

static uint8_t xunji_read_hardware(uint8_t index)
{
    switch (index) {
        case 0U: return XUNJI_HW_READ_0();
        case 1U: return XUNJI_HW_READ_1();
        case 2U: return XUNJI_HW_READ_2();
        case 3U: return XUNJI_HW_READ_3();
        case 4U: return XUNJI_HW_READ_4();
        case 5U: return XUNJI_HW_READ_5();
        case 6U: return XUNJI_HW_READ_6();
        case 7U: return XUNJI_HW_READ_7();
        default: return 0U;
    }
}

static uint8_t xunji_map_index(uint8_t index)
{
    return (XUNJI_SENSOR_ORDER == XUNJI_ORDER_LEFT_TO_RIGHT) ? index : (uint8_t)(7U - index);
}

static uint8_t xunji_raw_to_active(uint8_t raw)
{
    if (XUNJI_BLACK_POLARITY == XUNJI_BLACK_ACTIVE_LOW) {
        return (raw == 0U) ? 1U : 0U;
    }

    return (raw != 0U) ? 1U : 0U;
}

static uint8_t xunji_count_range(uint8_t bits, uint8_t start, uint8_t end)
{
    uint8_t count = 0U;

    for (uint8_t i = start; i <= end; ++i) {
        if ((bits & (uint8_t)(1U << i)) != 0U) {
            ++count;
        }
    }

    return count;
}

uint8_t Xunji_ReadRaw(uint8_t index)
{
    if (index >= XUNJI_SENSOR_COUNT) {
        return 0U;
    }

    return xunji_raw_to_active(xunji_read_hardware(xunji_map_index(index)));
}

uint8_t Xunji_IsActive(uint8_t index)
{
    return Xunji_ReadRaw(index);
}

uint8_t Xunji_ReadBits(void)
{
    uint8_t bits = 0U;

    for (uint8_t i = 0U; i < XUNJI_SENSOR_COUNT; ++i) {
        if (Xunji_ReadRaw(i) != 0U) {
            bits |= (uint8_t)(1U << i);
        }
    }

    return bits;
}

uint8_t Xunji_CountActive(uint8_t bits)
{
    uint8_t count = 0U;

    for (uint8_t i = 0U; i < XUNJI_SENSOR_COUNT; ++i) {
        if ((bits & (uint8_t)(1U << i)) != 0U) {
            ++count;
        }
    }

    return count;
}

uint16_t Xunji_GetPosition(uint8_t bits)
{
    static const uint16_t weights[XUNJI_SENSOR_COUNT] = {0U, 1000U, 2000U, 3000U, 4000U, 5000U, 6000U, 7000U};
    uint32_t weighted_sum = 0U;
    uint8_t active_count = 0U;

    for (uint8_t i = 0U; i < XUNJI_SENSOR_COUNT; ++i) {
        if ((bits & (uint8_t)(1U << i)) != 0U) {
            weighted_sum += weights[i];
            ++active_count;
        }
    }

    if (active_count == 0U) {
        return XUNJI_POSITION_LOST;
    }

    return (uint16_t)(weighted_sum / active_count);
}

int16_t Xunji_GetError(uint8_t bits)
{
    return (int16_t)Xunji_GetPosition(bits) - (int16_t)XUNJI_POSITION_CENTER;
}

bool Xunji_IsAllWhite(uint8_t bits)
{
    return bits == 0U;
}

bool Xunji_IsAllBlack(uint8_t bits)
{
    return bits == 0xFFU;
}

bool Xunji_IsLost(uint8_t bits)
{
    return bits == 0U;
}

bool Xunji_IsCrossByThreshold(uint8_t bits)
{
    uint8_t active = Xunji_CountActive(bits);
    uint8_t left = xunji_count_range(bits, 0U, 2U);
    uint8_t center = xunji_count_range(bits, 2U, 5U);
    uint8_t right = xunji_count_range(bits, 5U, 7U);

    return (active >= XUNJI_CROSS_ACTIVE_MIN) && (left >= 2U) && (center >= 2U) && (right >= 2U);
}

bool Xunji_IsCrossByPattern(uint8_t bits)
{
    return bits == XUNJI_PATTERN_CROSS;
}

bool Xunji_IsTJunctionByThreshold(uint8_t bits)
{
    uint8_t active = Xunji_CountActive(bits);
    uint8_t left = xunji_count_range(bits, 0U, 2U);
    uint8_t center = xunji_count_range(bits, 2U, 5U);
    uint8_t right = xunji_count_range(bits, 5U, 7U);

    if (Xunji_IsAllBlack(bits) || Xunji_IsCrossByThreshold(bits)) {
        return false;
    }

    return (active >= XUNJI_T_ACTIVE_MIN) && (center >= XUNJI_T_CENTER_MIN) && ((left >= 2U) || (right >= 2U));
}

bool Xunji_IsTJunctionByPattern(uint8_t bits)
{
    return bits == XUNJI_PATTERN_T;
}
```

- [ ] **Step 3: Compile the new source file and the API smoke file**

Run:

```bash
"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"project/Debug/device.opt" -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -o "project/Debug/xunji.o" "project/xunji.c"
"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -o "project/Debug/xunji_api_smoke.o" "project/Debug/xunji_api_smoke.c"
```

Expected: PASS twice and both `project/Debug/xunji.o` and `project/Debug/xunji_api_smoke.o` are created.

- [ ] **Step 4: Remove temporary verification outputs**

Run:

```bash
rm -f "project/Debug/xunji.o" "project/Debug/xunji_api_smoke.o"
```

Expected: both temporary object files are removed.

- [ ] **Step 5: Final code review against the agreed design**

Check that the final code still satisfies all of these exact points:

```text
1. The only race-day edit surface is the macro block at the top of project/xunji.h.
2. Left/right reversal is controlled by one direction macro, not by changing the algorithm.
3. Black-line polarity is controlled by one polarity macro, not by changing the algorithm.
4. GPIO remapping is controlled by XUNJI_HW_READ_0() ... XUNJI_HW_READ_7().
5. The module exports all 13 agreed functions.
6. Xunji_GetPosition(0) returns 3500.
7. Xunji_GetError(bits) returns Xunji_GetPosition(bits) - 3500.
8. Cross and T-junction each have threshold and fixed-pattern helpers.
```

Expected: every line above is true before reporting the task complete.
