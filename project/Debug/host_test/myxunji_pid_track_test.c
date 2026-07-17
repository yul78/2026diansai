#include <assert.h>
#include <stdint.h>
#include "myxunji.h"

void delay_ms(uint32_t ms)
{
    (void)ms;
}

void delay_us(uint32_t us)
{
    (void)us;
}

static void assert_straight_track(uint8_t bits, int16_t base_speed)
{
    int16_t left_speed = 0;
    int16_t right_speed = 0;

    MyXunji_PidTrackStraight(bits, base_speed, &left_speed, &right_speed);

    assert(left_speed >= -4000);
    assert(left_speed <= 4000);
    assert(right_speed >= -4000);
    assert(right_speed <= 4000);
}

int main(void)
{
    int16_t left_speed = 123;
    int16_t right_speed = 456;

    MyXunji_PidTrackStraight(0x00U, 1800, &left_speed, &right_speed);
    assert(left_speed == 0);
    assert(right_speed == 0);

    MyXunji_PidTrackStraight(0x08U, 1800, &left_speed, &right_speed);
    assert(left_speed < right_speed);

    MyXunji_PidTrackStraight(0x10U, 1800, &left_speed, &right_speed);
    assert(left_speed > right_speed);

    assert_straight_track(0x01U, 3900);
    assert_straight_track(0x80U, 3900);

    return 0;
}
