# 循迹位置值改为正负对称并以 0 为中心的设计

## 背景

当前 `Xunji_GetPosition()` 使用 `0~7000` 的无符号位置权重，中心值通过 `XUNJI_POSITION_CENTER = 3500U` 表示。这个方案能工作，但语义不够直观：

- 左右偏差不是天然的负正方向
- 上层在做控制时通常还要再减一次中心值
- 丢线值当前与中心值重合，不便于直接区分

本次调整的目标是把位置值改成以 `0` 为中心的有符号量，同时给丢线状态分配独立的特殊值。

## 目标

- 位置值改成有符号数，左偏为负，右偏为正
- 中心值固定为 `0`
- 丢线返回独立特殊值，不与中心位置混淆
- 保留现有 `Xunji_GetError()` 接口，尽量减少上层调用改动
- 顺手简化位置权重的量纲

## 非目标

- 不修改循迹传感器采样逻辑
- 不修改十字路口、T 字路口判定逻辑
- 不在本次设计中调整 PID 参数，只为后续重调提供更清晰的误差语义

## 方案对比

### 方案一：对称小整数权重（采用）

权重使用 `{-7, -5, -3, -1, 1, 3, 5, 7}`。

优点：
- 语义最直观
- 位置正负方向清楚
- 中心点天然为 `0`
- 量纲简单，便于调试和打印

代价：
- 与现有 `0~7000` 量纲不兼容
- 上层 PID 参数大概率需要重新整定

### 方案二：对称但保留较高分辨率

例如 `{-350, -250, -150, -50, 50, 150, 250, 350}`。

优点：
- 仍然保持中心为 `0`
- 比小整数保留更多细分程度

缺点：
- 简化程度不如方案一明显
- 对当前任务价值不高

### 方案三：连续整数权重

例如 `{-4, -3, -2, -1, 1, 2, 3, 4}`。

优点：
- 很容易记忆

缺点：
- 中间与边缘的拉开程度较弱
- 实际控制灵敏度通常不如奇数阶梯方案稳定

## 最终设计

### 1. 位置语义

- `XUNJI_POSITION_CENTER` 定义为 `0`
- `XUNJI_POSITION_LOST` 定义为 `((int16_t)INT16_MIN)`
- `Xunji_GetPosition()` 返回类型改为 `int16_t`
- 位置权重采用 `{-7, -5, -3, -1, 1, 3, 5, 7}`

语义定义如下：
- 返回负数：黑线偏左
- 返回 `0` 附近：接近中心
- 返回正数：黑线偏右
- 返回 `XUNJI_POSITION_LOST`：丢线

### 2. 接口调整

#### `project/xunji.h`

- 增加 `#include <limits.h>`
- `#define XUNJI_POSITION_CENTER 0`
- `#define XUNJI_POSITION_LOST ((int16_t)INT16_MIN)`
- 将 `uint16_t Xunji_GetPosition(uint8_t bits);` 改为 `int16_t Xunji_GetPosition(uint8_t bits);`

#### `project/xunji.c`

- 权重数组改为：
  `static const int16_t weights[XUNJI_SENSOR_COUNT] = {-7, -5, -3, -1, 1, 3, 5, 7};`
- `weighted_sum` 改为有符号累加类型，例如 `int32_t`
- `Xunji_GetPosition()` 在 `bits == 0` 时返回 `XUNJI_POSITION_LOST`
- `Xunji_GetError()` 保持返回 `int16_t`

### 3. `Xunji_GetError()` 的行为

保留 `Xunji_GetError()` 接口，不删除。

行为定义：
- 若 `Xunji_GetPosition()` 返回 `XUNJI_POSITION_LOST`，则 `Xunji_GetError()` 直接返回 `XUNJI_POSITION_LOST`
- 否则返回当前位置相对中心的误差值
- 由于中心值为 `0`，正常情况下误差值等于位置值本身

这样可以兼顾：
- 上层继续沿用 `Xunji_GetError()` 的调用方式
- 实现层仍然保持“位置”和“误差”两个语义入口

### 4. 丢线处理

本设计不再让“丢线”和“中心”共用同一个值。

规则如下：
- `bits == 0` 时，`Xunji_GetPosition()` 返回 `XUNJI_POSITION_LOST`
- `Xunji_GetError()` 接收到丢线值时直接透传
- 上层控制逻辑应将 `XUNJI_POSITION_LOST` 视为独立状态，而不是普通偏差

### 5. 对调用层的影响

- 如果上层只使用 `Xunji_GetError()` 做 PID 输入，改动通常较小
- 如果上层直接使用 `Xunji_GetPosition()`，则必须同步检查：
  - 是否仍按无符号数处理
  - 是否对返回值做了区间判断
  - 是否缺少对 `XUNJI_POSITION_LOST` 的单独分支

本次实现应全局搜索 `Xunji_GetPosition` 和 `Xunji_GetError` 的调用点，并同步修正不兼容使用方式。

## 验证方案

### 编译验证

- 确认 `xunji.h` 与 `xunji.c` 的类型修改后可以正常编译
- 确认所有调用点不存在有符号/无符号不匹配导致的编译告警或错误

### 行为验证

至少验证以下输入场景：

- 左侧单探头触发时，位置值为负数
- 右侧单探头触发时，位置值为正数
- 中间探头触发时，位置值接近 `0`
- 多探头同时触发时，加权平均结果方向正确
- `bits == 0` 时，返回 `XUNJI_POSITION_LOST`

### 兼容性验证

- 检查上层是否存在拿 `Xunji_GetPosition()` 与无符号常量直接比较的逻辑
- 检查丢线分支是否会把 `XUNJI_POSITION_LOST` 误送入普通 PID 计算

## 实施顺序

1. 修改 `project/xunji.h` 中的常量定义和函数声明
2. 修改 `project/xunji.c` 中的权重数组、返回类型和丢线逻辑
3. 全局搜索调用点并修正不兼容代码
4. 编译验证并进行样例输入检查

## 风险与注意事项

- PID 参数需要后续重新整定
- 若上层遗漏对 `XUNJI_POSITION_LOST` 的判断，可能导致控制异常
- 由于当前量纲大幅缩小，日志、调试输出和阈值判断若依赖旧量纲，需要一起检查

## 结论

采用“对称小整数权重 + 中心为 0 + 丢线特殊值”的方案。

具体为：
- 权重：`{-7, -5, -3, -1, 1, 3, 5, 7}`
- 中心值：`0`
- 丢线值：`XUNJI_POSITION_LOST = ((int16_t)INT16_MIN)`
- `Xunji_GetPosition()` 返回类型：`int16_t`

该方案语义清晰、实现简单，并且能显著降低上层控制逻辑理解成本。