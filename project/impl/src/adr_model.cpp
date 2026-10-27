#include "lab_booking/admin.hpp"
#include "lab_booking/booking.hpp"
#include "lab_booking/equipment.hpp"
#include "lab_booking/student.hpp"
#include "board_behavior.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

using Clock = std::chrono::system_clock;

Clock::time_point time_at(int hours)
{
    return Clock::time_point(Clock::duration(hours * 3600LL));
}

bool rejects_overlap()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Equipment equipment(2, "Microscope");
    lab_booking::Booking first(3);
    lab_booking::Booking second(4);
    student.requestBooking(first, equipment, time_at(0), time_at(2));
    try {
        student.requestBooking(second, equipment, time_at(1), time_at(3));
    } catch (const std::logic_error&) {
        return true;
    }
    return false;
}

}  // namespace

int main()
{
    using namespace lab_booking;

    const Booking initial(1);
    Booking approved(2);
    approved.handle(Booking::Event::approveBooking);
    Booking rejected(3);
    rejected.handle(Booking::Event::rejectBooking);
    Booking finished(5);
    finished.handle(Booking::Event::approveBooking);
    finished.startBooking();
    finished.endBooking();

    Student student(10, 20, "Student");
    Admin admin(11, 20, "Admin");
    Equipment equipment(12, "Microscope");
    Booking request(6);
    student.requestBooking(request, equipment, time_at(0), time_at(1));
    const bool student_has_request = student.viewBookings().size() == 1;
    const bool equipment_has_request = equipment.booked_in_.size() == 1;
    const std::string request_status = request.status;
    admin.approveBooking(request, student);
    const std::string admin_approved_status = request.status;
    equipment.startBooking(request, time_at(0), time_at(1));
    const bool active_in_use = equipment.inUse && equipment.whichBooking == &request;
    equipment.endBooking();
    const bool finished_released = !equipment.inUse && equipment.whichBooking == nullptr;

    Student cancelling_student(13, 20, "Student");
    Admin cancelling_admin(14, 20, "Admin");
    Equipment cancelling_equipment(15, "Camera");
    Booking cancelling_request(7);
    cancelling_student.requestBooking(cancelling_request, cancelling_equipment,
                                     time_at(0), time_at(1));
    cancelling_admin.approveBooking(cancelling_request, cancelling_student);
    cancelling_equipment.startBooking(cancelling_request, time_at(0), time_at(1));
    cancelling_student.cancelBooking(cancelling_request, cancelling_equipment);

    Student pending_cancel_student(16, 20, "Student");
    Equipment pending_cancel_equipment(17, "Camera");
    Booking pending_cancel(8);
    pending_cancel_student.requestBooking(pending_cancel, pending_cancel_equipment,
                                          time_at(0), time_at(1));
    pending_cancel_student.cancelBooking(pending_cancel, pending_cancel_equipment);

    Student rejecting_student(18, 20, "Student");
    Admin rejecting_admin(19, 20, "Admin");
    Booking admin_rejected(9);
    rejecting_admin.rejectBooking(admin_rejected, rejecting_student);

    std::cout
        << "{\"booking\":{"
        << "\"initial\":\"" << initial.status << "\","
        << "\"approved\":\"" << approved.status << "\","
        << "\"rejected\":\"" << rejected.status << "\","
        << "\"admin_approved\":\"" << admin_approved_status << "\","
        << "\"admin_rejected\":\"" << admin_rejected.status << "\","
        << "\"pending_cancel\":\"" << pending_cancel.status << "\","
        << "\"finished\":\"" << finished.status << "\","
        << "\"request_status\":\"" << request_status << "\","
        << "\"active_in_use\":" << (active_in_use ? "true" : "false") << ","
        << "\"finished_released\":" << (finished_released ? "true" : "false") << ","
        << "\"cancel_active_status\":\"" << cancelling_request.status << "\","
        << "\"cancel_active_released\":"
        << (!cancelling_equipment.inUse && cancelling_equipment.whichBooking == nullptr ? "true" : "false")
        << "},"
        << "\"request\":{"
        << "\"student_has_booking\":" << (student_has_request ? "true" : "false") << ","
        << "\"equipment_has_booking\":" << (equipment_has_request ? "true" : "false")
        << "},"
        << "\"equipment\":{\"rejects_overlap\":" << (rejects_overlap() ? "true" : "false") << "},"
        << "\"board\":{\"display\":{"
        << "\"pending\":\"" << booking_display_code("pending") << "\","
        << "\"approved\":\"" << booking_display_code("approved") << "\","
        << "\"in_use\":\"" << booking_display_code("in_use") << "\","
        << "\"finished\":\"" << booking_display_code("finished") << "\","
        << "\"cancelled\":\"" << booking_display_code("cancelled") << "\","
        << "\"rejected\":\"" << booking_display_code("rejected") << "\"},"
        << "\"button_a_from_first\":"
        << next_booking_index(0, 3, -1) << ","
        << "\"button_b_from_last\":"
        << next_booking_index(2, 3, 1)
        << "}}\n";
}
