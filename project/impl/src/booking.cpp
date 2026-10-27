#include "lab_booking/booking.hpp"

#include <utility>

namespace lab_booking {

Booking::Booking(int bookingId)
    : bookingId(bookingId), status("pending")
{
}

Booking::Booking(int bookingId, std::string status)
    : bookingId(bookingId), status(std::move(status))
{
}

void Booking::startBooking()
{
    handle(Event::startBooking);
}

void Booking::endBooking()
{
    handle(Event::endBooking);
}

void Booking::handle(Event event)
{
    for (const Transition& transition : transitions) {
        if (transition.from == state_ && transition.event == event) {
            state_ = transition.to;
            switch (state_) {
            case State::Pending:
                status = "pending";
                break;
            case State::Approved:
                status = "approved";
                break;
            case State::Rejected:
                status = "rejected";
                break;
            case State::InUse:
                status = "in_use";
                break;
            case State::Cancelled:
                status = "cancelled";
                break;
            case State::Finished:
                status = "finished";
                break;
            }
            return;
        }
    }
}

}  // namespace lab_booking
