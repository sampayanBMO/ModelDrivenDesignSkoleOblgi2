#pragma once

#include "lab_booking/user.hpp"

#include <string>
#include <vector>

namespace lab_booking {

class Booking;
class Student;

/// Administrator actions for reviewing booking requests.
class Admin : public User
{
public:
    Admin(int id, int departmentId, std::string name);

    /// Approves a pending booking and notifies its student.
    void approveBooking(Booking& booking, Student& student);
    /// Rejects a pending booking and notifies its student.
    void rejectBooking(Booking& booking, Student& student);
    /// Sends a decision notification to a student.
    void notifyStudent(Student& student, const std::string& decision);

private:
    std::vector<Booking*> reviews_;
};

}  // namespace lab_booking
