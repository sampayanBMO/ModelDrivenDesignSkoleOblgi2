#pragma once

#include "lab_booking/user.hpp"

#include <chrono>
#include <vector>

namespace lab_booking {

class Booking;
class Equipment;

/// Student-facing booking requests and non-owning booking views.
class Student : public User
{
public:
    using TimePoint = std::chrono::system_clock::time_point;

    Student(int id, int departmentId, std::string name);

    /// Adds a pending booking when its interval does not overlap a reservation.
    void requestBooking(Booking& booking, Equipment& equipment,
                        TimePoint startDate, TimePoint endDate);
    /// Cancels a booking and releases equipment when it is active.
    void cancelBooking(Booking& booking, Equipment& equipment);
    /// Returns the student's non-owning booking references.
    std::vector<Booking*> viewBookings() const;

private:
    std::vector<Booking*> requests_;
};

}  // namespace lab_booking
