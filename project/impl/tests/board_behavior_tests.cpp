#include "board_behavior.hpp"

#include <cassert>

int main()
{
    using lab_booking::booking_display_code;
    using lab_booking::next_booking_index;

    assert(booking_display_code("pending") == 'P');
    assert(booking_display_code("approved") == 'A');
    assert(booking_display_code("in_use") == 'U');
    assert(booking_display_code("finished") == 'F');
    assert(booking_display_code("cancelled") == 'C');
    assert(booking_display_code("rejected") == 'R');
    assert(booking_display_code("unknown") == '?');

    assert(next_booking_index(0, 3, -1) == 2);
    assert(next_booking_index(2, 3, 1) == 0);
    assert(next_booking_index(1, 3, 0) == 1);
    assert(next_booking_index(0, 0, 1) == 0);
    return 0;
}
