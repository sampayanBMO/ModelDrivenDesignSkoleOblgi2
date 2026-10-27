#include "lab_booking/booking.hpp"
#include "lab_booking/equipment.hpp"
#include "lab_booking/student.hpp"

#include <cassert>
#include <chrono>
#include <stdexcept>

namespace {

using Clock = std::chrono::system_clock;

Clock::time_point time_at(int hours)
{
    return Clock::time_point(Clock::duration(hours * 3600LL));
}

void rejects_overlapping_reservations()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Equipment equipment(2, "Microscope");
    lab_booking::Booking first(3);
    lab_booking::Booking second(4);

    student.requestBooking(first, equipment, time_at(0), time_at(2));

    bool rejected = false;
    try {
        student.requestBooking(second, equipment, time_at(1), time_at(3));
    } catch (const std::logic_error&) {
        rejected = true;
    }
    assert(rejected);
}

void accepts_touching_reservations()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Equipment equipment(2, "Microscope");
    lab_booking::Booking first(3);
    lab_booking::Booking second(4);

    student.requestBooking(first, equipment, time_at(0), time_at(2));
    student.requestBooking(second, equipment, time_at(2), time_at(3));
    assert(student.viewBookings().size() == 2);
}

void starts_and_ends_booking()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Equipment equipment(2, "Microscope");
    lab_booking::Booking booking(3);

    student.requestBooking(booking, equipment, time_at(0), time_at(2));
    booking.handle(lab_booking::Booking::Event::approveBooking);
    equipment.startBooking(booking, time_at(0), time_at(2));
    assert(equipment.inUse);
    assert(equipment.whichBooking == &booking);
    assert(booking.status == "in_use");

    equipment.endBooking();
    assert(!equipment.inUse);
    assert(equipment.whichBooking == nullptr);
    assert(booking.status == "finished");
}

void rejects_invalid_interval()
{
    lab_booking::Student student(1, 10, "Student");
    lab_booking::Equipment equipment(2, "Microscope");
    lab_booking::Booking booking(3);

    bool rejected = false;
    try {
        student.requestBooking(booking, equipment, time_at(2), time_at(2));
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);
}

}  // namespace

int main()
{
    rejects_overlapping_reservations();
    accepts_touching_reservations();
    starts_and_ends_booking();
    rejects_invalid_interval();
    return 0;
}
