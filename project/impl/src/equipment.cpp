#include "lab_booking/equipment.hpp"

#include "lab_booking/booking.hpp"

#include <stdexcept>
#include <utility>

namespace lab_booking {

Equipment::Equipment(int equipmentId, std::string name)
    : equipmentId(equipmentId), name(std::move(name))
{
}

void Equipment::startBooking(Booking& booking,
                             std::chrono::system_clock::time_point bookingStart,
                             std::chrono::system_clock::time_point bookingEnd)
{
    if (bookingStart >= bookingEnd) {
        throw std::invalid_argument("booking end must be after booking start");
    }
    if (inUse && whichBooking != &booking) {
        throw std::logic_error("equipment is already in use");
    }
    if (booking.status != "approved") {
        throw std::logic_error("only an approved booking can start");
    }

    inUse = true;
    whichBooking = &booking;
    startDate = bookingStart;
    endDate = bookingEnd;
    bool alreadyRegistered = false;
    for (const Reservation& reservation : reservations_) {
        if (reservation.booking == &booking) {
            alreadyRegistered = true;
            break;
        }
    }
    if (!alreadyRegistered) {
        reservations_.push_back({&booking, bookingStart, bookingEnd});
        booked_in_.push_back(&booking);
    }
    booking.startBooking();
}

void Equipment::endBooking()
{
    if (!inUse || whichBooking == nullptr) {
        return;
    }

    Booking* finishedBooking = whichBooking;
    finishedBooking->endBooking();
    inUse = false;
    whichBooking = nullptr;

    for (auto reservation = reservations_.begin(); reservation != reservations_.end();
         ++reservation) {
        if (reservation->booking == finishedBooking) {
            reservations_.erase(reservation);
            break;
        }
    }
}

}  // namespace lab_booking
