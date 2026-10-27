#pragma once

#include <chrono>
#include <string>
#include <vector>

namespace lab_booking {

class Booking;

/// Laboratory equipment and its active booking interval.
class Equipment
{
public:
    int equipmentId;
    std::string name;
    bool inUse = false;
    Booking* whichBooking = nullptr;
    std::chrono::system_clock::time_point startDate;
    std::chrono::system_clock::time_point endDate;
    std::vector<Booking*> booked_in_;

    Equipment(int equipmentId, std::string name);

    /// Reserves the equipment for an approved booking and marks it in use.
    void startBooking(Booking& booking,
                      std::chrono::system_clock::time_point startDate,
                      std::chrono::system_clock::time_point endDate);
    /// Releases the active booking; repeated release is harmless.
    void endBooking();

private:
    friend class Student;

    struct Reservation
    {
        Booking* booking;
        std::chrono::system_clock::time_point start;
        std::chrono::system_clock::time_point end;
    };

    std::vector<Reservation> reservations_;
};

}  // namespace lab_booking
