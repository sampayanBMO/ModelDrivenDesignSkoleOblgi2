#include "lab_booking/booking.hpp"

#include <cassert>

namespace {

void starts_pending()
{
    const lab_booking::Booking booking(1);
    assert(booking.status == "pending");
}

void approves_pending_booking()
{
    lab_booking::Booking booking(1);
    booking.handle(lab_booking::Booking::Event::approveBooking);
    assert(booking.status == "approved");
}

void rejects_pending_booking()
{
    lab_booking::Booking booking(1);
    booking.handle(lab_booking::Booking::Event::rejectBooking);
    assert(booking.status == "rejected");
}

void cancels_pending_booking()
{
    lab_booking::Booking booking(1);
    booking.handle(lab_booking::Booking::Event::cancelBooking);
    assert(booking.status == "cancelled");
}

void finishes_in_use_booking()
{
    lab_booking::Booking booking(1);
    booking.handle(lab_booking::Booking::Event::approveBooking);
    booking.startBooking();
    booking.endBooking();
    assert(booking.status == "finished");
}

void ignores_invalid_transition()
{
    lab_booking::Booking booking(1);
    booking.endBooking();
    assert(booking.status == "pending");
}

}  // namespace

int main()
{
    starts_pending();
    approves_pending_booking();
    rejects_pending_booking();
    cancels_pending_booking();
    finishes_in_use_booking();
    ignores_invalid_transition();
    return 0;
}
