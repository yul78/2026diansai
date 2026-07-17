#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include "xunji.h"

int main(void)
{
    assert(Xunji_IsLeftCornerByThreshold(0x07U));
    assert(!Xunji_IsRightCornerByThreshold(0x07U));

    assert(Xunji_IsRightCornerByThreshold(0xE0U));
    assert(!Xunji_IsLeftCornerByThreshold(0xE0U));

    assert(Xunji_IsCrossByThreshold(0xE7U));
    assert(!Xunji_IsLeftCornerByThreshold(0xE7U));
    assert(!Xunji_IsRightCornerByThreshold(0xE7U));

    assert(!Xunji_IsLeftCornerByThreshold(0x18U));
    assert(!Xunji_IsRightCornerByThreshold(0x18U));

    return 0;
}
