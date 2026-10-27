#include "lab_booking/admin.hpp"
#include "lab_booking/booking.hpp"
#include "lab_booking/equipment.hpp"
#include "lab_booking/student.hpp"

#include <chrono>

namespace {

void scenario(lab_booking::Student& student, lab_booking::Admin& admin,
              lab_booking::Booking& approvedBooking,
              lab_booking::Booking& rejectedBooking,
              lab_booking::Equipment& equipment)
{
    using Clock = std::chrono::system_clock;
    const auto start = Clock::time_point(Clock::duration(0));
    const auto end = start + std::chrono::hours(1);

    student.requestBooking(approvedBooking, equipment, start, end);
    admin.approveBooking(approvedBooking, student);
    student.requestBooking(rejectedBooking, equipment, end, end + std::chrono::hours(1));
    admin.rejectBooking(rejectedBooking, student);
    admin.notifyStudent(student, "decision");
    student.cancelBooking(approvedBooking, equipment);
    student.viewBookings();
    approvedBooking.startBooking();
    approvedBooking.endBooking();
}

}  // namespace

int main()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Admin admin(2, 10, "Admin");
    lab_booking::Booking approvedBooking(4);
    lab_booking::Booking rejectedBooking(5);
    lab_booking::Equipment equipment(3, "Oscilloscope");
    scenario(student, admin, approvedBooking, rejectedBooking, equipment);
    return 0;
}
