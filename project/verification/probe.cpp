#include "lab_booking/admin.hpp"
#include "lab_booking/booking.hpp"
#include "lab_booking/equipment.hpp"
#include "lab_booking/student.hpp"

#include <chrono>
#include <iostream>
#include <string>

namespace {
using Clock = std::chrono::system_clock;
Clock::time_point at(int hour) { return Clock::time_point(Clock::duration(hour * 3600LL)); }
void check(bool value, const char* name) { std::cout << "{\"check\":\"" << name << "\",\"observed\":" << (value ? "true" : "false") << "}\n"; }
}

int main()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Admin admin(2, 10, "Admin");
    lab_booking::Equipment equipment(3, "Oscilloscope");
    lab_booking::Booking pending(4);
    check(pending.status == "pending", "new_booking_is_pending");
    student.requestBooking(pending, equipment, at(0), at(1));
    check(student.viewBookings().size() == 1, "student_can_view_requested_booking");
    admin.approveBooking(pending, student);
    check(pending.status == "approved", "admin_can_approve");
    equipment.startBooking(pending, at(0), at(1));
    check(equipment.inUse && pending.status == "in_use", "approved_booking_activates_equipment");
    equipment.endBooking();
    check(!equipment.inUse && equipment.whichBooking == nullptr && pending.status == "finished",
          "ending_booking_releases_equipment");

    lab_booking::Booking cancelled(5);
    student.requestBooking(cancelled, equipment, at(2), at(3));
    student.cancelBooking(cancelled, equipment);
    check(cancelled.status == "cancelled", "pending_booking_can_be_cancelled");

    lab_booking::Booking first(6);
    lab_booking::Booking overlap(7);
    student.requestBooking(first, equipment, at(4), at(6));
    bool overlapRejected = false;
    try {
        student.requestBooking(overlap, equipment, at(5), at(7));
    } catch (const std::logic_error&) {
        overlapRejected = true;
    }
    check(overlapRejected, "overlapping_booking_is_rejected");

    lab_booking::Booking touching(8);
    bool touchingAccepted = true;
    try {
        student.requestBooking(touching, equipment, at(6), at(7));
    } catch (const std::exception&) {
        touchingAccepted = false;
    }
    check(touchingAccepted, "touching_booking_boundary_is_accepted");
}
