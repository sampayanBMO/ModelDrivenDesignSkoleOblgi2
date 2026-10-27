#include "lab_booking/admin.hpp"

#include "lab_booking/booking.hpp"
#include "lab_booking/student.hpp"

#include <stdexcept>
#include <utility>

namespace lab_booking {

Admin::Admin(int id, int departmentId, std::string name)
    : User(id, departmentId, std::move(name))
{
}

void Admin::approveBooking(Booking& booking, Student& student)
{
    if (booking.status != "pending") {
        throw std::logic_error("only a pending booking can be approved");
    }
    reviews_.push_back(&booking);
    booking.handle(Booking::Event::approveBooking);
    notifyStudent(student, "approved");
}

void Admin::rejectBooking(Booking& booking, Student& student)
{
    if (booking.status != "pending") {
        throw std::logic_error("only a pending booking can be rejected");
    }
    reviews_.push_back(&booking);
    booking.handle(Booking::Event::rejectBooking);
    notifyStudent(student, "rejected");
}

void Admin::notifyStudent(Student&, const std::string&)
{
}

}  // namespace lab_booking
