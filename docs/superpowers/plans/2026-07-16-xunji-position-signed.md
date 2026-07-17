# Xunji Signed Position Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将循迹位置值改为以 0 为中心的有符号语义，使用对称小整数权重，并用独立哨兵值表示丢线。

**Architecture:** 只改动 `project/xunji.h` 和 `project/xunji.c` 两个源码文件，不碰 SysConfig 生成文件。先用一个临时编译烟雾文件锁定头文件常量和函数签名，再修改实现文件恢复当前被注释掉的 `Xunji_CountActive()` 并完成有符号位置计算，最后用整工程编译和调用点检索确认兼容性。

**Tech Stack:** C, MSPM0G3507, TI DriverLib, SysConfig-generated GPIO macros, tiarmclang toolchain, make-based CCS debug build.

## Global Constraints

- 位置值改成有符号数，左偏为负，右偏为正。
- 中心值固定为 `0`。
- 丢线返回独立特殊值，不与中心位置混淆。
- 保留现有 `Xunji_GetError()` 接口，尽量减少上层调用改动。
- 顺手简化位置权重的量纲。
- 不修改循迹传感器采样逻辑。
- 不修改十字路口、T 字路口判定逻辑。
- 不在本次实现中调整 PID 参数，只为后续重调提供更清晰的误差语义。
- 位置权重采用 `{-7, -5, -3, -1, 1, 3, 5, 7}`。
- `XUNJI_POSITION_LOST` 定义为 `((int16_t)INT16_MIN)`。
- `Xunji_GetPosition()` 返回类型必须是 `int16_t`。
- 当前 `project/xunji.c` 中 `Xunji_CountActive()` 被整段注释掉；本次改动必须恢复该函数，最终结果必须重新可编译。
- 不要编辑 `project/empty.syscfg` 或 `project/Debug/ti_msp_dl_config.[ch]`。
- 不要创建 git commit，除非用户明确要求。

---

## File Map

- `project/xunji.h`
  - 责任：公开循迹模块的常量和函数声明。
  - 本次修改：引入 `limits.h`，把中心值和丢线值改成新语义，把 `Xunji_GetPosition()` 声明改成 `int16_t`。
- `project/xunji.c`
  - 责任：实现循迹位置计算、误差计算和路口判定。
  - 本次修改：恢复 `Xunji_CountActive()`，把位置权重和累加类型改成有符号实现，让 `Xunji_GetError()` 透传丢线哨兵值。
- `project/Debug/xunji_signature_smoke.c`
  - 责任：临时编译烟雾文件，只用于验证头文件常量和函数签名。
  - 本次修改：任务 1 创建，验证通过后删除，不提交。

### Task 1: 锁定头文件常量与函数签名

**Files:**
- Create: `project/Debug/xunji_signature_smoke.c`
- Modify: `project/xunji.h:4-10,44-45`
- Verify: `project/Debug/xunji_signature_smoke.o` (temporary local artifact, do not commit)

**Interfaces:**
- Consumes:
  - `project/Debug/ti_msp_dl_config.h`
  - existing `project/xunji.h`
- Produces:
  - `#define XUNJI_POSITION_CENTER 0`
  - `#define XUNJI_POSITION_LOST ((int16_t)INT16_MIN)`
  - `int16_t Xunji_GetPosition(uint8_t bits);`
  - unchanged `int16_t Xunji_GetError(uint8_t bits);`

- [ ] **Step 1: 写一个会先失败的头文件烟雾验证文件**

```c
#include <limits.h>
#include <stdint.h>
#include "xunji.h"

typedef int16_t (*xunji_position_fn_t)(uint8_t);
typedef int16_t (*xunji_error_fn_t)(uint8_t);

_Static_assert(XUNJI_SENSOR_COUNT == 8U, "sensor count changed unexpectedly");
_Static_assert(XUNJI_POSITION_CENTER == 0, "center must be zero");
_Static_assert(XUNJI_POSITION_LOST == ((int16_t)INT16_MIN), "lost sentinel mismatch");

static xunji_position_fn_t position_fn = Xunji_GetPosition;
static xunji_error_fn_t error_fn = Xunji_GetError;

int main(void)
{
    return (position_fn == 0 || error_fn == 0) ? 1 : 0;
}
```

把这段内容保存到 `project/Debug/xunji_signature_smoke.c`。

- [ ] **Step 2: 编译烟雾文件，确认它在改头文件前先失败**

Run:

```bash
"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"e:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug/device.opt" -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -o "project/Debug/xunji_signature_smoke.o" "project/Debug/xunji_signature_smoke.c"
```

Expected: FAIL，至少包含下面一种错误：
- `static assertion failed`，因为当前 `XUNJI_POSITION_CENTER` 还是 `3500U`
- 或 `initializing 'xunji_position_fn_t' ... incompatible type`，因为当前 `Xunji_GetPosition()` 还是 `uint16_t`

- [ ] **Step 3: 修改 `project/xunji.h`，切换到有符号位置语义**

把头文件的开头常量和声明改成下面这样：

```c
#ifndef XUNJI_H
#define XUNJI_H

#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "ti_msp_dl_config.h"

#define XUNJI_SENSOR_COUNT     8U
#define XUNJI_POSITION_CENTER  0
#define XUNJI_POSITION_LOST    ((int16_t)INT16_MIN)

#define XUNJI_BLACK_ACTIVE_LOW   0U
#define XUNJI_BLACK_ACTIVE_HIGH  1U
#define XUNJI_BLACK_POLARITY     XUNJI_BLACK_ACTIVE_LOW

#define XUNJI_ORDER_LEFT_TO_RIGHT   0U
#define XUNJI_ORDER_RIGHT_TO_LEFT   1U
#define XUNJI_SENSOR_ORDER          XUNJI_ORDER_LEFT_TO_RIGHT

#define XUNJI_CROSS_ACTIVE_MIN  6U
#define XUNJI_T_ACTIVE_MIN      5U
#define XUNJI_T_CENTER_MIN      2U
#define XUNJI_LEFT_MASK         0x07U
#define XUNJI_CENTER_MASK       0x18U
#define XUNJI_RIGHT_MASK        0xE0U
#define XUNJI_PATTERN_CROSS     0xFFU
#define XUNJI_PATTERN_T         0x3CU

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
int16_t Xunji_GetPosition(uint8_t bits);
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

- [ ] **Step 4: 重新编译烟雾文件，确认头文件语义和签名已经对齐**

Run:

```bash
"E:/2026diansai/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"e:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug/device.opt" -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O0 -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project" -I"E:/My_MCU_Project/MSPM0G3507/2025car1.0/project/Debug" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source/third_party/CMSIS/Core/Include" -I"E:/2026diansai/mspm0_sdk_2_10_00_04/source" -gdwarf-3 -Wall -o "project/Debug/xunji_signature_smoke.o" "project/Debug/xunji_signature_smoke.c"
```

Expected: PASS，并生成 `project/Debug/xunji_signature_smoke.o`。

- [ ] **Step 5: 删除临时烟雾文件，避免把验证垃圾留在工作区**

Run:

```bash
rm -f "project/Debug/xunji_signature_smoke.c" "project/Debug/xunji_signature_smoke.o"
```

Expected: 两个临时文件被删除，工作区只保留真正需要提交的源码修改。

### Task 2: 修改实现文件并验证整工程兼容性

**Files:**
- Modify: `project/xunji.c:113-164`
- Verify: `project/Debug/xunji.o`, `project/Debug/project.out`

**Interfaces:**
- Consumes:
  - `project/xunji.h` 中新的常量与签名
  - `uint8_t Xunji_CountActive(uint8_t bits);`
- Produces:
  - `uint8_t Xunji_CountActive(uint8_t bits);`
  - `int16_t Xunji_GetPosition(uint8_t bits);`
  - `int16_t Xunji_GetError(uint8_t bits);`
  - 无代码调用点需要额外改动的确认结论，或发现调用点后立即修正

- [ ] **Step 1: 先跑整工程构建，确认实现文件在新头文件下确实失败**

Run:

```bash
make -C "project/Debug" all
```

Expected: FAIL，至少包含下面一种错误：
- `conflicting types for 'Xunji_GetPosition'`，因为 `project/xunji.c` 里还是 `uint16_t Xunji_GetPosition(uint8_t bits)`
- 或链接阶段出现 `undefined reference to 'Xunji_CountActive'`，因为当前这个函数被整段注释掉了

- [ ] **Step 2: 修改 `project/xunji.c`，恢复 `Xunji_CountActive()` 并完成有符号位置计算**

把当前被注释掉的 `Xunji_CountActive()` 恢复成真正的函数定义，并把 `Xunji_GetPosition()`、`Xunji_GetError()` 改成下面这组实现：

```c
uint8_t Xunji_CountActive(uint8_t bits)
{
    uint8_t count = 0U;

    for (uint8_t i = 0U; i < XUNJI_SENSOR_COUNT; ++i) {
        if ((bits & (uint8_t) (1U << i)) != 0U) {
            ++count;
        }
    }

    return count;
}

int16_t Xunji_GetPosition(uint8_t bits)
{
    static const int16_t weights[XUNJI_SENSOR_COUNT] = {-7, -5, -3, -1, 1, 3, 5, 7};
    int32_t weighted_sum = 0;
    uint8_t active_count = 0U;

    for (uint8_t i = 0U; i < XUNJI_SENSOR_COUNT; ++i) {
        if ((bits & (uint8_t) (1U << i)) != 0U) {
            weighted_sum += weights[i];
            ++active_count;
        }
    }

    if (active_count == 0U) {
        return XUNJI_POSITION_LOST;
    }

    return (int16_t) (weighted_sum / (int32_t) active_count);
}

int16_t Xunji_GetError(uint8_t bits)
{
    int16_t position = Xunji_GetPosition(bits);

    if (position == XUNJI_POSITION_LOST) {
        return XUNJI_POSITION_LOST;
    }

    return position;
}
```

同时把这三个函数上方的注释里“位置值”“丢线位置”“误差值”的描述保持和新语义一致；不要改动 `Xunji_ReadBits()`、`Xunji_IsCrossByThreshold()`、`Xunji_IsTJunctionByThreshold()` 等与本次需求无关的逻辑。

- [ ] **Step 3: 重新跑整工程构建，确认源码重新可编译**

Run:

```bash
make -C "project/Debug" all
```

Expected: PASS，输出中至少能看到：
- `Arm Compiler - building file: "../xunji.c"`
- `Finished building: "../xunji.c"`
- `Finished building target: "project.out"`

- [ ] **Step 4: 检索真实代码调用点，确认没有遗漏的无符号使用方式**

Run:

```bash
python - <<'PY'
from pathlib import Path
import re
root = Path(r'e:/My_MCU_Project/MSPM0G3507/2025car1.0/project')
pattern = re.compile(r'Xunji_Get(Position|Error)\s*\(')
for path in root.rglob('*'):
    if path.suffix.lower() not in {'.c', '.h'}:
        continue
    text = path.read_text(encoding='utf-8', errors='ignore')
    for i, line in enumerate(text.splitlines(), 1):
        if pattern.search(line):
            print(f'{path.as_posix()}:{i}:{line.strip()}')
PY
```

Expected: 结果只包含 `project/xunji.h` 与 `project/xunji.c`；如果出现其他 `.c` 文件调用 `Xunji_GetPosition()` 或 `Xunji_GetError()`，立刻在同一个任务里把那些调用改成兼容有符号位置值和 `XUNJI_POSITION_LOST` 的写法，然后重新执行 `make -C "project/Debug" all` 直到通过。

- [ ] **Step 5: 不提交，保留变更供人工复查**

Run:

```bash
git diff -- project/xunji.h project/xunji.c
```

Expected: diff 只包含本次需求相关的头文件与实现文件变更；不要创建 commit，等待人工复查或后续执行步骤。
