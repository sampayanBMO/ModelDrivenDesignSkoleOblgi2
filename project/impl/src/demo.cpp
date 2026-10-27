#include "lab_booking/admin.hpp"
#include "lab_booking/booking.hpp"
#include "lab_booking/equipment.hpp"
#include "lab_booking/student.hpp"

#include <chrono>

int main()
{
    using Clock = std::chrono::system_clock;

    lab_booking::Student student(1, 10, "Student");
    lab_booking::Admin admin(2, 10, "Admin");
    lab_booking::Equipment equipment(3, "Oscilloscope");
    lab_booking::Booking booking(4);

    const auto start = Clock::time_point(Clock::duration(0));
    const auto end = start + std::chrono::hours(1);
    student.requestBooking(booking, equipment, start, end);
    admin.approveBooking(booking, student);
    equipment.startBooking(booking, start, end);
    equipment.endBooking();
    return 0;
}
