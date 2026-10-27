#include "lab_booking/student.hpp"

#include "lab_booking/booking.hpp"
#include "lab_booking/equipment.hpp"

#include <stdexcept>
#include <utility>

namespace lab_booking {

Student::Student(int id, int departmentId, std::string name)
    : User(id, departmentId, std::move(name))
{
}

void Student::requestBooking(Booking& booking, Equipment& equipment,
                             TimePoint startDate, TimePoint endDate)
{
    if (startDate >= endDate) {
        throw std::invalid_argument("booking end must be after booking start");
    }
    if (booking.status != "pending") {
        throw std::logic_error("only a pending booking can be requested");
    }
    for (const Equipment::Reservation& reservation : equipment.reservations_) {
        if (reservation.booking == &booking) {
            throw std::logic_error("booking has already been requested");
        }
        if (startDate < reservation.end && reservation.start < endDate) {
            throw std::logic_error("equipment booking overlaps an existing reservation");
        }
    }

    requests_.push_back(&booking);
    equipment.booked_in_.push_back(&booking);
    equipment.reservations_.push_back({&booking, startDate, endDate});
}

void Student::cancelBooking(Booking& booking, Equipment& equipment)
{
    booking.handle(Booking::Event::cancelBooking);
    if (equipment.whichBooking == &booking) {
        equipment.endBooking();
    } else {
        for (auto reservation = equipment.reservations_.begin();
             reservation != equipment.reservations_.end(); ++reservation) {
            if (reservation->booking == &booking) {
                equipment.reservations_.erase(reservation);
                break;
            }
        }
    }
}

std::vector<Booking*> Student::viewBookings() const
{
    return requests_;
}

}  // namespace lab_booking
