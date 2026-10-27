#include "lab_booking/admin.hpp"
#include "lab_booking/booking.hpp"
#include "lab_booking/equipment.hpp"
#include "lab_booking/student.hpp"

#include <cassert>
#include <chrono>

namespace {

using Clock = std::chrono::system_clock;

Clock::time_point time_at(int hours)
{
    return Clock::time_point(Clock::duration(hours * 3600LL));
}

void student_can_request_and_view_booking()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Equipment equipment(2, "Microscope");
    lab_booking::Booking booking(3);

    student.requestBooking(booking, equipment, time_at(0), time_at(1));

    const std::vector<lab_booking::Booking*> bookings = student.viewBookings();
    assert(bookings.size() == 1);
    assert(bookings.front() == &booking);
    assert(booking.status == "pending");
}

void admin_can_approve_booking()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Admin admin(2, 10, "Admin");
    lab_booking::Equipment equipment(3, "Microscope");
    lab_booking::Booking booking(4);

    student.requestBooking(booking, equipment, time_at(0), time_at(1));
    admin.approveBooking(booking, student);

    assert(booking.status == "approved");
}

void admin_can_reject_booking()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Admin admin(2, 10, "Admin");
    lab_booking::Equipment equipment(3, "Microscope");
    lab_booking::Booking booking(4);

    student.requestBooking(booking, equipment, time_at(0), time_at(1));
    admin.rejectBooking(booking, student);

    assert(booking.status == "rejected");
}

void cancellation_releases_active_equipment()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Admin admin(2, 10, "Admin");
    lab_booking::Equipment equipment(3, "Microscope");
    lab_booking::Booking booking(4);

    student.requestBooking(booking, equipment, time_at(0), time_at(1));
    admin.approveBooking(booking, student);
    equipment.startBooking(booking, time_at(0), time_at(1));
    student.cancelBooking(booking, equipment);

    assert(booking.status == "cancelled");
    assert(!equipment.inUse);
    assert(equipment.whichBooking == nullptr);
}

}  // namespace

int main()
{
    student_can_request_and_view_booking();
    admin_can_approve_booking();
    admin_can_reject_booking();
    cancellation_releases_active_equipment();
    return 0;
}
