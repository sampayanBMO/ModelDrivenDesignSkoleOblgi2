#pragma once

#include <cstddef>
#include <string>

namespace lab_booking {

inline char booking_display_code(const std::string& status)
{
    if (status == "pending") return 'P';
    if (status == "approved") return 'A';
    if (status == "in_use") return 'U';
    if (status == "finished") return 'F';
    if (status == "cancelled") return 'C';
    if (status == "rejected") return 'R';
    return '?';
}

inline std::size_t next_booking_index(std::size_t current, std::size_t count, int direction)
{
    if (count == 0 || direction == 0) return count == 0 ? 0 : current % count;
    current %= count;
    return direction < 0 ? (current + count - 1) % count : (current + 1) % count;
}

}  // namespace lab_booking
